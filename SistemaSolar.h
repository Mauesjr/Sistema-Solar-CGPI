#pragma once
#ifndef SOLARSYSTEM_HPP
#define SOLARSYSTEM_HPP

#include <GLFW/glfw3.h>
#include <cmath>
#include <vector>
#include <cstdlib>
#include <deque>

// Custom Vector structure
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

// Mover class representing planets
class Mover {
public:
    Vector2 pos, vel, acc;
    float mass, r;
    float r_col, g_col, b_col;

    // Trail storage
    std::deque<Vector2> path;
    size_t maxPathLength = 250; 

    Mover(float x, float y, float vx, float vy, float m, float r_c, float g_c, float b_c) {
        pos = Vector2(x, y);
        vel = Vector2(vx, vy);
        acc = Vector2(0, 0);
        mass = m;
        r = 8.0f;

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

// Attractor class representing the central sun
class Attractor {
public:
    Vector2 pos;
    float mass, r;

    Attractor(float x, float y, float m) {
        pos = Vector2(x, y);
        mass = m;
        r = std::sqrt(mass) * 1.0f;
    }

    void attract(Mover& mover, float currentGravity) {
        Vector2 force = Vector2::sub(pos, mover.pos);
        float distanceSq = force.magSq();

        if (distanceSq < 25.0f) distanceSq = 25.0f;
        if (distanceSq > 250000.0f) distanceSq = 250000.0f;

        float strength = currentGravity * (mass * mover.mass) / distanceSq;

        force.setMag(strength);
        mover.applyForce(force);
    }
};

// Main SolarSystem manager
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
    float gravityMultiplier = 2.0f;
    float sunMass = 300.0f;
    bool enableNBody = false;
    bool containPlanets = true; 

    SolarSystem(int screenWidth, int screenHeight) {
        m_screenWidth = (float)screenWidth;
        m_screenHeight = (float)screenHeight;
        float cx = (float)screenWidth / 2.0f;
        float cy = (float)screenHeight / 2.0f;

        attractor = new Attractor(cx, cy, 300);

        movers.push_back(Mover(cx, cy + 30.0f, 4.47f, 0.0f, 2.0f, 0.5f, 0.5f, 0.5f));   
        movers.push_back(Mover(cx, cy + 50.0f, 3.46f, 0.0f, 4.0f, 0.9f, 0.7f, 0.2f));   
        movers.push_back(Mover(cx, cy + 70.0f, 2.92f, 0.0f, 5.0f, 0.2f, 0.4f, 1.0f));   
        movers.push_back(Mover(cx, cy + 90.0f, 2.58f, 0.0f, 3.0f, 0.8f, 0.2f, 0.1f));   
        movers.push_back(Mover(cx, cy + 120.0f, 2.23f, 0.0f, 25.0f, 0.8f, 0.6f, 0.4f)); 
        movers.push_back(Mover(cx, cy + 150.0f, 2.00f, 0.0f, 15.0f, 0.9f, 0.8f, 0.6f)); 
        movers.push_back(Mover(cx, cy + 180.0f, 1.82f, 0.0f, 10.0f, 0.4f, 0.8f, 0.9f)); 
        movers.push_back(Mover(cx, cy + 210.0f, 1.69f, 0.0f, 9.0f, 0.1f, 0.2f, 0.8f));  
    }

    ~SolarSystem() {
        delete attractor;
    }

    void onUpdate() {
        attractor->mass = sunMass;
        attractor->r = std::sqrt(sunMass) * 1.0f;
        
        for (auto& mover : movers) {
            attractor->attract(mover, gravityMultiplier);
        }
        
        if (enableNBody) {
            for (size_t i = 0; i < movers.size(); i++) {
                for (size_t j = i + 1; j < movers.size(); j++) {
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
        
        for (auto& mover : movers) {
            mover.update();
            if (containPlanets) {
                mover.checkEdges(m_screenWidth, m_screenHeight);
            }
        }
    }

    void onDisplay() {
        for (const auto& mover : movers) {
            glBegin(GL_LINE_STRIP);
            glColor3f(mover.r_col, mover.g_col, mover.b_col);
            for (const auto& point : mover.path) {
                glVertex2f(point.x, point.y);
            }
            glEnd();
        }
        
        for (auto& mover : movers) {
            drawCircle(mover.pos.x, mover.pos.y, mover.r, 20, mover.r_col, mover.g_col, mover.b_col);
        }
                 
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