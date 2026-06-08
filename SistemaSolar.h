#pragma once
#ifndef SOLARSYSTEM_HPP
#define SOLARSYSTEM_HPP

#include <GLFW/glfw3.h>
#include <cmath>
#include <vector>
#include <cstdlib>
#include <deque>
#include <algorithm>

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

enum class BodyType {
    STAR,
    PLANET,
    ASTEROID,
    BLACK_HOLE
};

class CelestialBody {
public:
    BodyType type;
    Vector2 pos, vel, acc;
    float mass, r;
    float r_col, g_col, b_col;
    bool isDead = false; 

    std::deque<Vector2> path;
    size_t maxPathLength = 250; 

    CelestialBody(BodyType t, float x, float y, float vx, float vy, float m, float r_c, float g_c, float b_c) {
        type = t;
        pos = Vector2(x, y);
        vel = Vector2(vx, vy);
        acc = Vector2(0, 0);
        mass = m;
        
        // DENSITY FIX: Multipliers are now standardized. 
        // When visualScale = 1.0, the physical radius strictly matches the visual OpenGL render.
        float radiusMultiplier;
        if (type == BodyType::STAR) {
            radiusMultiplier = 1.0f; // Baseline density
        } else if (type == BodyType::PLANET) {
            radiusMultiplier = 1.0f; // Baseline density
        } else if (type == BodyType::ASTEROID) {
            radiusMultiplier = 0.5f; // Denser rocks
        } else {
            radiusMultiplier = 0.2f; // Black hole singularity
        }

        r = std::sqrt(mass) * radiusMultiplier; 

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

        if (type == BodyType::PLANET || type == BodyType::ASTEROID) {
            path.push_back(pos);
            if (path.size() > maxPathLength) {
                path.pop_front();
            }
        }
    }

    void checkEdges(float screenWidth, float screenHeight) {
        float bounceDamping = -0.8f; 

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

class SolarSystem {
private:
    std::vector<CelestialBody> bodies;

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
    float gravityMultiplier = 2.0f;
    bool enableNBody = true;
    bool containPlanets = false; 
    bool enableCollisions = true; // Turn ON by default so mass merging works intuitively
    float visualScale = 1.0f;     // Default to 1.0f so physics and visuals match out-of-the-box

    SolarSystem(int screenWidth, int screenHeight) {
        m_screenWidth = (float)screenWidth;
        m_screenHeight = (float)screenHeight;
        float cx = (float)screenWidth / 2.0f;
        float cy = (float)screenHeight / 2.0f;

       // SYSTEM ANCHOR FIX: Mass 10000 ensures heavy planets do not eject the sun easily
        // The Sun
        bodies.push_back(CelestialBody(BodyType::STAR, cx, cy, 0.0f, 0.0f, 10000.0f, 1.0f, 0.8f, 0.0f));

        // Creating the Inner Solar System
        // Formula for stable orbit velocity: v = sqrt((G * Mass_Sun) / Distance)
        // Here, G = gravityMultiplier = 2.0f, Mass_Sun = 10000.0f. So G * M = 20000.0f

        // 1. Mercury (Closest, Fastest, Smallest)
        // Distance: 60.0f. v = sqrt(20000 / 60) = sqrt(333.33) ≈ 18.25
        bodies.push_back(CelestialBody(BodyType::PLANET, cx, cy + 60.0f, 18.25f, 0.0f, 0.5f, 0.6f, 0.6f, 0.6f)); // Grey

        // 2. Venus (Slower, Further, Similar to Earth size)
        // Distance: 100.0f. v = sqrt(20000 / 100) = sqrt(200) ≈ 14.14
        bodies.push_back(CelestialBody(BodyType::PLANET, cx, cy + 100.0f, 14.14f, 0.0f, 4.0f, 0.9f, 0.7f, 0.2f)); // Orange/Yellow

        // 3. Earth (The baseline)
        // Distance: 150.0f. v = sqrt(20000 / 150) = sqrt(133.33) ≈ 11.54
        bodies.push_back(CelestialBody(BodyType::PLANET, cx, cy + 150.0f, 11.54f, 0.0f, 5.0f, 0.2f, 0.4f, 1.0f)); // Blue

        // 4. Mars (Furthest of inner, slowest, smaller than Earth)
        // Distance: 200.0f. v = sqrt(20000 / 200) = sqrt(100) = 10.0
        bodies.push_back(CelestialBody(BodyType::PLANET, cx, cy + 200.0f, 10.0f, 0.0f, 3.0f, 0.8f, 0.2f, 0.1f)); // Red
    }

    void onUpdate() {
        for (size_t i = 0; i < bodies.size(); i++) {
            if (bodies[i].isDead) continue;

            for (size_t j = i + 1; j < bodies.size(); j++) {
                if (bodies[j].isDead) continue;

                Vector2 force = Vector2::sub(bodies[i].pos, bodies[j].pos);
                float distanceSq = force.magSq();
                
                if (distanceSq < 100.0f) distanceSq = 100.0f;

                bool swallowed = false;

                // BLACK HOLE PRIORITY ABSORPTION
                if (bodies[i].type == BodyType::BLACK_HOLE) {
                    float eventHorizonSq = std::pow(bodies[i].r * 4.0f, 2); 
                    if (distanceSq < eventHorizonSq) {
                        Vector2 newVel(
                            (bodies[i].mass * bodies[i].vel.x + bodies[j].mass * bodies[j].vel.x) / (bodies[i].mass + bodies[j].mass),
                            (bodies[i].mass * bodies[i].vel.y + bodies[j].mass * bodies[j].vel.y) / (bodies[i].mass + bodies[j].mass)
                        );
                        bodies[i].vel = newVel;
                        bodies[i].mass += bodies[j].mass; 
                        bodies[i].r = std::sqrt(bodies[i].mass) * 0.2f; 
                        bodies[j].isDead = true; 
                        swallowed = true;
                    }
                }
                
                if (!swallowed && bodies[j].type == BodyType::BLACK_HOLE) {
                    float eventHorizonSq = std::pow(bodies[j].r * 4.0f, 2);
                    if (distanceSq < eventHorizonSq) {
                        Vector2 newVel(
                            (bodies[j].mass * bodies[j].vel.x + bodies[i].mass * bodies[i].vel.x) / (bodies[j].mass + bodies[i].mass),
                            (bodies[j].mass * bodies[j].vel.y + bodies[i].mass * bodies[i].vel.y) / (bodies[j].mass + bodies[i].mass)
                        );
                        bodies[j].vel = newVel;
                        bodies[j].mass += bodies[i].mass;
                        bodies[j].r = std::sqrt(bodies[j].mass) * 0.2f;
                        bodies[i].isDead = true;
                        swallowed = true;
                    }
                }

                if (swallowed) continue;

                bool isGravitySourceI = bodies[i].type == BodyType::STAR || bodies[i].type == BodyType::BLACK_HOLE;
                bool isGravitySourceJ = bodies[j].type == BodyType::STAR || bodies[j].type == BodyType::BLACK_HOLE;

                if (enableNBody || isGravitySourceI || isGravitySourceJ) {
                    float strength = gravityMultiplier * (bodies[i].mass * bodies[j].mass) / distanceSq;
                    force.setMag(strength);

                    if (enableNBody || isGravitySourceI) bodies[j].applyForce(force);
                    
                    Vector2 reverseForce(-force.x, -force.y);
                    if (enableNBody || isGravitySourceJ) bodies[i].applyForce(reverseForce);
                }
            }   
        }

        // STANDARD INELASTIC COLLISIONS
        if (enableCollisions) {
            for (size_t i = 0; i < bodies.size(); i++) {
                if (bodies[i].isDead) continue;
                for (size_t j = i + 1; j < bodies.size(); j++) {
                    if (bodies[j].isDead) continue;
                    
                    if (bodies[i].type == BodyType::BLACK_HOLE || bodies[j].type == BodyType::BLACK_HOLE) continue;
                    
                    float distSq = Vector2::sub(bodies[i].pos, bodies[j].pos).magSq();
                    
                    // COLLISION FIX: Directly compare physical radii squared. No visual modifiers allowed here.
                    float radiusSum = bodies[i].r + bodies[j].r;
                    
                    if (distSq < radiusSum * radiusSum) {
                        float totalMass = bodies[i].mass + bodies[j].mass;
                        
                        Vector2 newVel(
                            (bodies[i].mass * bodies[i].vel.x + bodies[j].mass * bodies[j].vel.x) / totalMass,
                            (bodies[i].mass * bodies[i].vel.y + bodies[j].mass * bodies[j].vel.y) / totalMass
                        );
                        Vector2 newPos(
                            (bodies[i].mass * bodies[i].pos.x + bodies[j].mass * bodies[j].pos.x) / totalMass,
                            (bodies[i].mass * bodies[i].pos.y + bodies[j].mass * bodies[j].pos.y) / totalMass
                        );
                        
                        float r_col = (bodies[i].r_col * bodies[i].mass + bodies[j].r_col * bodies[j].mass) / totalMass;
                        float g_col = (bodies[i].g_col * bodies[i].mass + bodies[j].g_col * bodies[j].mass) / totalMass;
                        float b_col = (bodies[i].b_col * bodies[i].mass + bodies[j].b_col * bodies[j].mass) / totalMass;

                        bodies[i].mass = totalMass;
                        bodies[i].vel = newVel;
                        bodies[i].pos = newPos;
                        bodies[i].r_col = r_col;
                        bodies[i].g_col = g_col;
                        bodies[i].b_col = b_col;
                        
                        float radiusMult;
                        if (bodies[i].type == BodyType::STAR) radiusMult = 1.0f;
                        else if (bodies[i].type == BodyType::PLANET) radiusMult = 1.0f;
                        else radiusMult = 0.5f;

                        bodies[i].r = std::sqrt(totalMass) * radiusMult; 
                        bodies[j].isDead = true; 
                    }
                }
            }
        }
        
        bodies.erase(std::remove_if(bodies.begin(), bodies.end(), [](const CelestialBody& m) { return m.isDead; }), bodies.end());
        
        for (auto& body : bodies) {
            body.update();
            if (containPlanets) body.checkEdges(m_screenWidth, m_screenHeight);
        }
    }

    void onDisplay() {
        for (const auto& body : bodies) {
            if (body.type == BodyType::PLANET || body.type == BodyType::ASTEROID) {
                glBegin(GL_LINE_STRIP);
                glColor3f(body.r_col, body.g_col, body.b_col);
                for (const auto& point : body.path) glVertex2f(point.x, point.y);
                glEnd();
            }
        }
        
        // RENDER ALIGNMENT FIX: 
        // No hidden modifiers. renderRadius is purely physical radius * explicit visualScale UI modifier.
        for (auto& body : bodies) {
            float renderRadius = body.r * visualScale;
            
            if (body.type == BodyType::BLACK_HOLE) {
                // Event Horizon (Draws relative to the physical renderRadius)
                drawCircle(body.pos.x, body.pos.y, renderRadius * 4.0f, 30, 0.2f, 0.0f, 0.3f);
                drawCircle(body.pos.x, body.pos.y, renderRadius, 30, 0.0f, 0.0f, 0.0f);
            } else {
                drawCircle(body.pos.x, body.pos.y, renderRadius, 30, body.r_col, body.g_col, body.b_col);
            }
        }
    }

    void addBody(BodyType type, float x, float y, float vx, float vy, float m, float r_c, float g_c, float b_c ) {
        bodies.push_back(CelestialBody(type, x, y, vx, vy, m, r_c, g_c, b_c));
    }

    Vector2 getDominantGravityCenter() const {
        if (bodies.empty()) return Vector2(0, 0);
        const CelestialBody* heaviest = &bodies[0];
        for (const auto& b : bodies) {
            if (b.mass > heaviest->mass) heaviest = &b;
        }
        return heaviest->pos;
    }

    float getDominantMass() const {
        if (bodies.empty()) return 0;
        float maxMass = bodies[0].mass;
        for (const auto& b : bodies) {
            if (b.mass > maxMass) maxMass = b.mass;
        }
        return maxMass;
    }
};
#endif // SOLARSYSTEM_HPP