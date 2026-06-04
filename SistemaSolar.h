#pragma once
#ifndef SOLARSYSTEM_HPP
#define SOLARSYSTEM_HPP

#include <GLFW/glfw3.h>
#include <cmath>
#include <vector>
#include <cstdlib>
#include <deque>
#include <algorithm> // Required for std::remove_if

// 1. Custom Vector structure
struct Vector2 {
    float x, y;

    Vector2(float _x = 0, float _y = 0) : x(_x), y(_y) {}

    void add(const Vector2& v) { x += v.x; y += v.y; }
    void sub(const Vector2& v) { x -= v.x; y -= v.y; }
    void mult(float n) { x *= n; y *= n; }
    void div(float n) { x /= n; y /= n; }
    float magSq() const { return x * x + y * y; }
    float mag() const { return std::sqrt(magSq()); }

    void setMag(float n) {
        float m = mag();
        if (m != 0) {
            mult(1.0f / m);
            mult(n);
        }
    }

    static Vector2 sub(const Vector2& v1, const Vector2& v2) {
        return Vector2(v1.x - v2.x, v1.y - v2.y);
    }
};

// 2. Mover class (The planets)
class Mover {
public:
    Vector2 pos, vel, acc;
    float mass, r;
    float r_col, g_col, b_col; // Stores the color of the planet (Red, Green, Blue)
    bool isDead = false; // Flag for collision cleanup

    // Trail storage
    std::deque<Vector2> path;
    size_t maxPathLength = 250; 

    // Constructor receiving color parameters
    Mover(float x, float y, float vx, float vy, float m, float r_c, float g_c, float b_c) {
        pos = Vector2(x, y);
        vel = Vector2(vx, vy);
        acc = Vector2(0, 0);
        mass = m;
        
        // Dynamically calculate radius based on mass for visual accuracy
        r = std::sqrt(mass) * 3.0f; 

        // Save the chosen color
        r_col = r_c;
        g_col = g_c;
        b_col = b_c;
    }

    void applyForce(Vector2 force) {
        force.div(mass);
        acc.add(force);
    }

    void update() {
        vel.add(acc);
        pos.add(vel);
        acc.mult(0);

        // Record current position to the trail
        path.push_back(pos);
        // Remove the oldest position if the trail exceeds the maximum length
        if (path.size() > maxPathLength) path.pop_front();
    }

    void checkEdges(float screenWidth, float screenHeight) {
        float bounceDamping = -0.8f; // Retain 80% of speed, flip direction

        if (pos.x > screenWidth - r) {
            pos.x = screenWidth - r;
            vel.x *= bounceDamping;
        } else if (pos.x < r) {
            pos.x = r;
            vel.x *= bounceDamping;
        }

        if (pos.y > screenHeight - r) {
            pos.y = screenHeight - r;
            vel.y *= bounceDamping;
        } else if (pos.y < r) {
            pos.y = r;
            vel.y *= bounceDamping;
        }
    }
};

// 3. Attractor class (The central Sun)
class Attractor {
public:
    Vector2 pos;
    float mass, r;

    Attractor(float x, float y, float m) {
        pos = Vector2(x, y);
        mass = m;
        // VISUAL CHANGE: Multiplied by 1.0f so the Sun does not occupy the entire screen.
        r = std::sqrt(mass) * 1.0f;
    }

    void attract(Mover& mover, float currentGravity) {
        Vector2 force = Vector2::sub(pos, mover.pos);
        float distanceSq = force.magSq();

        if (distanceSq < 100.0f) distanceSq = 100.0f;
        if (distanceSq > 250000.0f) distanceSq = 250000.0f;

        // Calculate strength using the dynamic gravity value
        float strength = currentGravity * (mass * mover.mass) / distanceSq;

        force.setMag(strength);
        mover.applyForce(force);
    }
};

// 4. Main SolarSystem manager
class SolarSystem {
private:
    std::vector<Mover> movers;
    Attractor* attractor;

    void drawCircle(float cx, float cy, float r, int num_segments, float r_color, float g_color, float b_color) {
        glBegin(GL_TRIANGLE_FAN);
        glColor3f(r_color, g_color, b_color);
        glVertex2f(cx, cy);
        for (int i = 0; i <= num_segments; i++) {
            float theta = 2.0f * 3.1415926f * float(i) / float(num_segments);
            float x = r * std::cos(theta);
            float y = r * std::sin(theta);
            glVertex2f(x + cx, y + cy);
        }
        glEnd();
    }

    float m_screenWidth;
    float m_screenHeight;

public:
    // UI Exposed Variables
    float gravityMultiplier = 2.0f;
    float sunMass = 300.0f;
    bool enableNBody = false;
    bool containPlanets = false; 
    bool enableCollisions = false; // UI Toggle for merging

    SolarSystem(int screenWidth, int screenHeight) {
        m_screenWidth = (float)screenWidth;
        m_screenHeight = (float)screenHeight;
        float cx = (float)screenWidth / 2.0f;
        float cy = (float)screenHeight / 2.0f;

        // Dynamic central Sun
        attractor = new Attractor(cx, cy, 300);

        // Parameters: X, Y (cy + radius), VelX, VelY, Mass, R, G, B
        // Distances are expanded. Masses are reduced. Velocities are recalculated for M=1000, G=2.0
        movers.push_back(Mover(cx, cy + 60.0f,  5.77f, 0.0f, 0.5f, 0.5f, 0.5f, 0.5f)); // Mercury
        movers.push_back(Mover(cx, cy + 100.0f, 4.47f, 0.0f, 1.0f, 0.9f, 0.7f, 0.2f)); // Venus
        movers.push_back(Mover(cx, cy + 150.0f, 3.65f, 0.0f, 1.2f, 0.2f, 0.4f, 1.0f)); // Earth
        movers.push_back(Mover(cx, cy + 200.0f, 3.16f, 0.0f, 0.8f, 0.8f, 0.2f, 0.1f)); // Mars
        movers.push_back(Mover(cx, cy + 260.0f, 2.77f, 0.0f, 5.0f, 0.8f, 0.6f, 0.4f)); // Jupiter
        movers.push_back(Mover(cx, cy + 320.0f, 2.50f, 0.0f, 3.0f, 0.9f, 0.8f, 0.6f)); // Saturn
        movers.push_back(Mover(cx, cy + 380.0f, 2.29f, 0.0f, 2.0f, 0.4f, 0.8f, 0.9f)); // Uranus
        movers.push_back(Mover(cx, cy + 450.0f, 2.10f, 0.0f, 2.0f, 0.1f, 0.2f, 0.8f)); // Neptune
    }

    ~SolarSystem() {
        delete attractor;
    }

    void onUpdate() {
        // 1. Update the sun's mass dynamically based on the UI
        attractor->mass = sunMass;
        attractor->r = std::sqrt(sunMass) * 1.0f;
        
        // 2. The Sun attracts all planets
        for (auto& mover : movers) {
            attractor->attract(mover, gravityMultiplier);
        }
        
        // 3. N-Body Interaction (Planets attracting planets)
        if (enableNBody) {
            for (size_t i = 0; i < movers.size(); i++) {
                if (movers[i].isDead) continue;

                for (size_t j = i + 1; j < movers.size(); j++) {
                    if (movers[j].isDead) continue;

                    // Force vector pointing from j to i
                    Vector2 force = Vector2::sub(movers[i].pos, movers[j].pos);
                    float distanceSq = force.magSq();
                    
                    if (distanceSq < 100.0f) distanceSq = 100.0f;
                    if (distanceSq > 250000.0f) distanceSq = 250000.0f;

                    float strength = gravityMultiplier * (movers[i].mass * movers[j].mass) / distanceSq;
                    force.setMag(strength);

                    // Pull body j towards body i
                    movers[j].applyForce(force);

                    // Newton's Third Law: Apply equal and opposite force to body i
                    Vector2 reverseForce(-force.x, -force.y);
                    movers[i].applyForce(reverseForce);
                }
            }   
        }

        // 4. INELASTIC COLLISION LOGIC
        if (enableCollisions) {
            // Planet-Planet Collisions
            for (size_t i = 0; i < movers.size(); i++) {
                if (movers[i].isDead) continue;
                
                for (size_t j = i + 1; j < movers.size(); j++) {
                    if (movers[j].isDead) continue;
                    
                    float distSq = Vector2::sub(movers[i].pos, movers[j].pos).magSq();
                    float radiusSum = movers[i].r + movers[j].r;
                    
                    if (distSq < radiusSum * radiusSum) {
                        float totalMass = movers[i].mass + movers[j].mass;
                        
                        // Conservation of Momentum calculation
                        Vector2 newVel(
                            (movers[i].mass * movers[i].vel.x + movers[j].mass * movers[j].vel.x) / totalMass,
                            (movers[i].mass * movers[i].vel.y + movers[j].mass * movers[j].vel.y) / totalMass
                        );
                        
                        // Center of Mass calculation
                        Vector2 newPos(
                            (movers[i].mass * movers[i].pos.x + movers[j].mass * movers[j].pos.x) / totalMass,
                            (movers[i].mass * movers[i].pos.y + movers[j].mass * movers[j].pos.y) / totalMass
                        );
                        
                        // Color blending
                        float r_col = (movers[i].r_col * movers[i].mass + movers[j].r_col * movers[j].mass) / totalMass;
                        float g_col = (movers[i].g_col * movers[i].mass + movers[j].g_col * movers[j].mass) / totalMass;
                        float b_col = (movers[i].b_col * movers[i].mass + movers[j].b_col * movers[j].mass) / totalMass;
                        
                        // Update body I to represent the new merged mass
                        movers[i].mass = totalMass;
                        movers[i].vel = newVel;
                        movers[i].pos = newPos;
                        movers[i].r_col = r_col;
                        movers[i].g_col = g_col;
                        movers[i].b_col = b_col;
                        movers[i].r = std::sqrt(totalMass) * 3.0f; // Update scale
                        
                        movers[j].isDead = true; // Mark body J for deletion
                    }
                }
            }
            
            // Sun-Planet Collisions
            for (auto& mover : movers) {
                if (mover.isDead) continue;
                
                float distSq = Vector2::sub(attractor->pos, mover.pos).magSq();
                float radiusSum = attractor->r + mover.r;
                
                if (distSq < radiusSum * radiusSum) {
                    sunMass += mover.mass; // Transfer mass to the sun
                    mover.isDead = true;
                }
            }
            
            // Memory Cleanup: Erase all dead entities
            movers.erase(std::remove_if(movers.begin(), movers.end(), [](const Mover& m) { return m.isDead; }), movers.end());
        }
        
        // 5. Update kinematics
        for (auto& mover : movers) {
            mover.update();
            if (containPlanets) {
                mover.checkEdges(m_screenWidth, m_screenHeight);
            }
        }
    }

    void onDisplay() {
        // Draw the trails first 
        for (const auto& mover : movers) {
            glBegin(GL_LINE_STRIP);
            glColor3f(mover.r_col, mover.g_col, mover.b_col);
            for (const auto& point : mover.path) {
                glVertex2f(point.x, point.y);
            }
            glEnd();
        }
        
        // Draw the planets
        for (auto& mover : movers) {
            drawCircle(mover.pos.x, mover.pos.y, mover.r, 20, mover.r_col, mover.g_col, mover.b_col);
        }
                 
        // Draw the central Sun
        drawCircle(attractor->pos.x, attractor->pos.y, attractor->r, 40, 1.0f, 0.8f, 0.0f);
    }

    void onKeyboard(unsigned char key, int x, int y) {}

    void setSunPosition(float x, float y) {
        if (attractor != nullptr) {
            attractor->pos.x = x;
            attractor->pos.y = y;
        }
    }

    void addBody(float x, float y, float vx, float vy, float m, float r_c, float g_c, float b_c ) {
        movers.push_back(Mover(x, y, vx, vy, m, r_c, g_c, b_c));
    }

    float getSunPosX() const { return attractor->pos.x; }
    float getSunPosY() const { return attractor->pos.y; }
};
#endif // SOLARSYSTEM_HPP