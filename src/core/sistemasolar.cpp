#include "sistemasolar.h"
#include <cmath>
#include <algorithm>

// ==========================================
// 1. Implementação do Vector2
// ==========================================
Vector2::Vector2(float _x, float _y) : x(_x), y(_y) {}

void Vector2::add(const Vector2& v) { x += v.x; y += v.y; }
void Vector2::sub(const Vector2& v) { x -= v.x; y -= v.y; }
void Vector2::mult(float n) { x *= n; y *= n; }
void Vector2::div(float n) { x /= n; y /= n; }
float Vector2::magSq() const { return x * x + y * y; }
float Vector2::mag() const { return std::sqrt(magSq()); }
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
// 2. Implementação do CelestialBody
// ==========================================
CelestialBody::CelestialBody(BodyType t, float x, float y, float vx, float vy, float m, float r_c, float g_c, float b_c) {
    type = t;
    pos = Vector2(x, y);
    vel = Vector2(vx, vy);
    acc = Vector2(0, 0);
    mass = m;
    
    // Multiplicadores de densidade (Buracos negros são menores e mais densos)
    float radiusMultiplier;
    if (type == BodyType::STAR) {
        radiusMultiplier = 1.0f; 
    } else if (type == BodyType::PLANET) {
        radiusMultiplier = 1.0f; 
    } else if (type == BodyType::ASTEROID) {
        radiusMultiplier = 0.5f; 
    } else {
        radiusMultiplier = 0.2f; 
    }

    r = std::sqrt(mass) * radiusMultiplier; 

    r_col = r_c;
    g_col = g_c;
    b_col = b_c;
    
    isDead = false;
    maxPathLength = 250; 
}

void CelestialBody::applyForce(Vector2 force) {
    force.div(mass);
    acc.add(force);
}

void CelestialBody::update() {
    vel.add(acc);
    pos.add(vel);
    acc.mult(0); 

    // Apenas planetas e asteroides deixam rastro na tela
    if (type == BodyType::PLANET || type == BodyType::ASTEROID) {
        path.push_back(pos);
        if (path.size() > maxPathLength) {
            path.pop_front();
        }
    }
}

void CelestialBody::checkEdges(float screenWidth, float screenHeight) {
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

// ==========================================
// 3. Implementação do SolarSystem
// ==========================================
SolarSystem::SolarSystem(int screenWidth, int screenHeight) {
    m_screenWidth = (float)screenWidth;
    m_screenHeight = (float)screenHeight;
    float cx = (float)screenWidth / 2.0f;
    float cy = (float)screenHeight / 2.0f;

    gravityMultiplier = 2.0f;
    enableNBody = true;
    containPlanets = false; 
    enableCollisions = true; 

    
    bodies.push_back(CelestialBody(BodyType::STAR, cx, cy, 0.0f, 0.0f, 10000.0f, 1.0f, 0.8f, 0.0f));
    bodies.push_back(CelestialBody(BodyType::PLANET, cx, cy + 160.0f, 11.18f, 0.0f, 0.5f, 0.6f, 0.6f, 0.6f)); 
    bodies.push_back(CelestialBody(BodyType::PLANET, cx, cy + 220.0f, 9.53f, 0.0f, 4.0f, 0.9f, 0.7f, 0.2f));  
    bodies.push_back(CelestialBody(BodyType::PLANET, cx, cy + 150.0f, 11.54f, 0.0f, 5.0f, 0.2f, 0.4f, 1.0f)); 
    bodies.push_back(CelestialBody(BodyType::PLANET, cx, cy + 400.0f, 7.07f, 0.0f, 3.0f, 0.8f, 0.2f, 0.1f));
}

void SolarSystem::onUpdate() {
    for (size_t i = 0; i < bodies.size(); i++) {
        if (bodies[i].isDead) continue;

        for (size_t j = i + 1; j < bodies.size(); j++) {
            if (bodies[j].isDead) continue;

            Vector2 force = Vector2::sub(bodies[i].pos, bodies[j].pos);
            float distanceSq = force.magSq();
            
            if (distanceSq < 100.0f) distanceSq = 100.0f;

            bool swallowed = false;

            // Prioridade de Absorção: Horizonte de Eventos do Buraco Negro
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

            // Lógica Gravitacional Baseada em Tipos
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

    // Colisões Inelásticas Padrão
    if (enableCollisions) {
        for (size_t i = 0; i < bodies.size(); i++) {
            if (bodies[i].isDead) continue;
            for (size_t j = i + 1; j < bodies.size(); j++) {
                if (bodies[j].isDead) continue;
                
                // Buracos negros usam a lógica própria do horizonte de eventos acima
                if (bodies[i].type == BodyType::BLACK_HOLE || bodies[j].type == BodyType::BLACK_HOLE) continue;
                
                float distSq = Vector2::sub(bodies[i].pos, bodies[j].pos).magSq();
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
    
    // Limpa os mortos
    bodies.erase(std::remove_if(bodies.begin(), bodies.end(), [](const CelestialBody& m) { return m.isDead; }), bodies.end());
    
    for (auto& body : bodies) {
        body.update();
        if (containPlanets) body.checkEdges(m_screenWidth, m_screenHeight);
    }
}

void SolarSystem::addBody(BodyType type, float x, float y, float vx, float vy, float m, float r_c, float g_c, float b_c ) {
    bodies.push_back(CelestialBody(type, x, y, vx, vy, m, r_c, g_c, b_c));
}

Vector2 SolarSystem::getDominantGravityCenter() const {
    if (bodies.empty()) return Vector2(0, 0);
    const CelestialBody* heaviest = &bodies[0];
    for (const auto& b : bodies) {
        if (b.mass > heaviest->mass) heaviest = &b;
    }
    return heaviest->pos;
}

float SolarSystem::getDominantMass() const {
    if (bodies.empty()) return 0;
    float maxMass = bodies[0].mass;
    for (const auto& b : bodies) {
        if (b.mass > maxMass) maxMass = b.mass;
    }
    return maxMass;
}