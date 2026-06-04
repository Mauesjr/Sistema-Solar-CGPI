#include "sistemasolar.h"
#include <cmath>

/// Implementação do Vector2
Vector2::Vector2(float _x, float _y) : x(_x), y(_y) {}

void Vector2::add(const Vector2& v) { 
    x += v.x; 
    y += v.y; 
}

void Vector2::sub(const Vector2& v) { 
    x -= v.x; 
    y -= v.y; 
}

void Vector2::mult(float n) { 
    x *= n; 
    y *= n; 
}

void Vector2::div(float n) { 
    x /= n; 
    y /= n; 
}

float Vector2::magSq() const { 
    return x * x + y * y; 
}

float Vector2::mag() const { 
    return std::sqrt(magSq()); 
}

void Vector2::setMag(float n) {
    float m = mag();
    if (m != 0) {
        mult(1.0f / m);
        mult(n);
    }
}

Vector2 Vector2::sub(const Vector2& v1, const Vector2& v2) {
    return Vector2(v1.x - v2.x, v1.y - v2.y);
}


// ==========================================
// 2. Classe dos Planetas (Implementações)
// ==========================================

Mover::Mover(float x, float y, float vx, float vy, float m, float r_c, float g_c, float b_c) {
    pos = Vector2(x, y);
    vel = Vector2(vx, vy);
    acc = Vector2(0, 0);
    mass = m;
    
    // O raio continua sendo atrelado à massa matematicamente
    r = std::sqrt(mass) * 3.0f; 
    
    r_col = r_c;
    g_col = g_c;
    b_col = b_c;
    
    // Inicialização das variáveis que antes ficavam no .h
    isDead = false;
    maxPathLength = 250; 
}

void Mover::applyForce(Vector2 force) {
    force.div(mass);
    acc.add(force);
}

void Mover::update() {
    vel.add(acc);
    pos.add(vel);
    acc.mult(0); // Zera a aceleração para o próximo frame

    // Grava a posição atual no rastro
    path.push_back(pos);
    if (path.size() > maxPathLength) path.pop_front();
}

void Mover::checkEdges(float screenWidth, float screenHeight) {
    float bounceDamping = -0.8f; // Mantém 80% da velocidade e inverte a direção

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

// ==========================================
// 3. Classe do Sol (Implementações)
// ==========================================

Attractor::Attractor(float x, float y, float m) {
    pos = Vector2(x, y);
    mass = m;
    r = std::sqrt(mass) * 1.0f;
}

void Attractor::attract(Mover& mover, float currentGravity) {
    Vector2 force = Vector2::sub(pos, mover.pos);
    float distanceSq = force.magSq();

    // Limites para evitar que a força tenda ao infinito (divisão por zero) ou seja nula
    if (distanceSq < 100.0f) distanceSq = 100.0f;
    if (distanceSq > 250000.0f) distanceSq = 250000.0f;

    float strength = currentGravity * (mass * mover.mass) / distanceSq;

    force.setMag(strength);
    mover.applyForce(force);
}

// ==========================================
// 4. Gerenciador Central (Implementações)
// ==========================================

SolarSystem::SolarSystem(int screenWidth, int screenHeight) {
    m_screenWidth = (float)screenWidth;
    m_screenHeight = (float)screenHeight;
    float cx = (float)screenWidth / 2.0f;
    float cy = (float)screenHeight / 2.0f;

    // Inicializa valores padrões baseados no projeto original
    gravityMultiplier = 2.0f;
    sunMass = 300.0f;
    enableNBody = false;
    containPlanets = false;
    enableCollisions = false;

    attractor = new Attractor(cx, cy, sunMass);

    // Adiciona os planetas iniciais do sistema
    movers.push_back(Mover(cx, cy + 60.0f,  5.77f, 0.0f, 0.5f, 0.5f, 0.5f, 0.5f)); 
    movers.push_back(Mover(cx, cy + 100.0f, 4.47f, 0.0f, 1.0f, 0.9f, 0.7f, 0.2f)); 
    movers.push_back(Mover(cx, cy + 150.0f, 3.65f, 0.0f, 1.2f, 0.2f, 0.4f, 1.0f)); 
    movers.push_back(Mover(cx, cy + 200.0f, 3.16f, 0.0f, 0.8f, 0.8f, 0.2f, 0.1f)); 
    movers.push_back(Mover(cx, cy + 260.0f, 2.77f, 0.0f, 5.0f, 0.8f, 0.6f, 0.4f)); 
    movers.push_back(Mover(cx, cy + 320.0f, 2.50f, 0.0f, 3.0f, 0.9f, 0.8f, 0.6f)); 
    movers.push_back(Mover(cx, cy + 380.0f, 2.29f, 0.0f, 2.0f, 0.4f, 0.8f, 0.9f)); 
    movers.push_back(Mover(cx, cy + 450.0f, 2.10f, 0.0f, 2.0f, 0.1f, 0.2f, 0.8f)); 
}

SolarSystem::~SolarSystem() {
    delete attractor;
}

void SolarSystem::onUpdate() {
    attractor->mass = sunMass;
    attractor->r = std::sqrt(sunMass) * 1.0f;
    
    // O Sol atrai todos os planetas
    for (auto& mover : movers) {
        attractor->attract(mover, gravityMultiplier);
    }
    
    // Interação N-Body (Planeta atrai Planeta)
    if (enableNBody) {
        for (size_t i = 0; i < movers.size(); i++) {
            if (movers[i].isDead) continue;

            for (size_t j = i + 1; j < movers.size(); j++) {
                if (movers[j].isDead) continue;

                Vector2 force = Vector2::sub(movers[i].pos, movers[j].pos);
                float distanceSq = force.magSq();
                
                if (distanceSq < 100.0f) distanceSq = 100.0f;
                if (distanceSq > 250000.0f) distanceSq = 250000.0f;

                float strength = gravityMultiplier * (movers[i].mass * movers[j].mass) / distanceSq;
                force.setMag(strength);

                movers[j].applyForce(force);

                // Terceira Lei de Newton (Ação e Reação)
                Vector2 reverseForce(-force.x, -force.y);
                movers[i].applyForce(reverseForce);
            }
        }   
    }

    // Colisões Inelásticas (Fusão de corpos celestes)
    if (enableCollisions) {
        for (size_t i = 0; i < movers.size(); i++) {
            if (movers[i].isDead) continue;
            
            for (size_t j = i + 1; j < movers.size(); j++) {
                if (movers[j].isDead) continue;
                
                float distSq = Vector2::sub(movers[i].pos, movers[j].pos).magSq();
                float radiusSum = movers[i].r + movers[j].r;
                
                if (distSq < radiusSum * radiusSum) {
                    float totalMass = movers[i].mass + movers[j].mass;
                    
                    Vector2 newVel(
                        (movers[i].mass * movers[i].vel.x + movers[j].mass * movers[j].vel.x) / totalMass,
                        (movers[i].mass * movers[i].vel.y + movers[j].mass * movers[j].vel.y) / totalMass
                    );
                    
                    Vector2 newPos(
                        (movers[i].mass * movers[i].pos.x + movers[j].mass * movers[j].pos.x) / totalMass,
                        (movers[i].mass * movers[i].pos.y + movers[j].mass * movers[j].pos.y) / totalMass
                    );
                    
                    float r_col = (movers[i].r_col * movers[i].mass + movers[j].r_col * movers[j].mass) / totalMass;
                    float g_col = (movers[i].g_col * movers[i].mass + movers[j].g_col * movers[j].mass) / totalMass;
                    float b_col = (movers[i].b_col * movers[i].mass + movers[j].b_col * movers[j].mass) / totalMass;
                    
                    movers[i].mass = totalMass;
                    movers[i].vel = newVel;
                    movers[i].pos = newPos;
                    movers[i].r_col = r_col;
                    movers[i].g_col = g_col;
                    movers[i].b_col = b_col;
                    movers[i].r = std::sqrt(totalMass) * 3.0f; 
                    
                    movers[j].isDead = true; 
                }
            }
        }
        
        // Colisão com o Sol
        for (auto& mover : movers) {
            if (mover.isDead) continue;
            
            float distSq = Vector2::sub(attractor->pos, mover.pos).magSq();
            float radiusSum = attractor->r + mover.r;
            
            if (distSq < radiusSum * radiusSum) {
                sunMass += mover.mass; 
                mover.isDead = true;
            }
        }
        
        // Remove os corpos "mortos" da memória
        movers.erase(std::remove_if(movers.begin(), movers.end(), [](const Mover& m) { return m.isDead; }), movers.end());
    }
    
    // Atualiza cinemática final de todos os objetos
    for (auto& mover : movers) {
        mover.update();
        if (containPlanets) {
            mover.checkEdges(m_screenWidth, m_screenHeight);
        }
    }
}

void SolarSystem::setSunPosition(float x, float y) {
    if (attractor != nullptr) {
        attractor->pos.x = x;
        attractor->pos.y = y;
    }
}

void SolarSystem::addBody(float x, float y, float vx, float vy, float m, float r_c, float g_c, float b_c ) {
    movers.push_back(Mover(x, y, vx, vy, m, r_c, g_c, b_c));
}

float SolarSystem::getSunPosX() const { return attractor->pos.x; }
float SolarSystem::getSunPosY() const { return attractor->pos.y; }
