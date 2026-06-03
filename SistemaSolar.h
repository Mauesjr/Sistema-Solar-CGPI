#pragma once
#ifndef SOLARSYSTEM_HPP
#define SOLARSYSTEM_HPP

#include <GLFW/glfw3.h>
#include <cmath>
#include <vector>
#include <cstdlib>

// 1. Nossa própria estrutura de Vetores
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

// 2. Classe Mover (Os planetas)
class Mover {
public:
    Vector2 pos, vel, acc;
    float mass, r;
    float r_col, g_col, b_col; // NOVO: Guardam a cor do planeta (Red, Green, Blue)

    // NOVO: Adicionamos os 3 últimos parâmetros para receber a cor
    Mover(float x, float y, float vx, float vy, float m, float r_c, float g_c, float b_c) {
        pos = Vector2(x, y);
        vel = Vector2(vx, vy);
        acc = Vector2(0, 0);
        mass = m;
        r = 8.0f; // Tamanho visual fixo para todos

        // Salva a cor escolhida
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
    }
};
// 3. Classe Attractor (O Sol no centro)
class Attractor {
public:
    Vector2 pos;
    float mass, r;

    Attractor(float x, float y, float m) {
        pos = Vector2(x, y);
        mass = m;
        // MUDANÇA VISUAL: Multiplicamos por 1.0f para o Sol não ocupar a tela inteira.
        r = std::sqrt(mass) * 1.0f;
    }

    void attract(Mover& mover) {
        Vector2 force = Vector2::sub(pos, mover.pos);
        float distanceSq = force.magSq();

        if (distanceSq < 25.0f) distanceSq = 25.0f;
        if (distanceSq > 250000.0f) distanceSq = 250000.0f;

        // MUDANÇA NA FÍSICA: Diminuímos o G de 5.0 para 1.0. 
        // Isso deixa tudo orbitando mais devagar e suavemente.
        float G = 2.0f; 
        float strength = G * (mass * mover.mass) / distanceSq;

        force.setMag(strength);
        mover.applyForce(force);
    }
};

// 4. O Gerenciador Principal
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

public:
    // NOVO: O construtor agora exige saber o tamanho da tela quando o jogo começar
    SolarSystem(int screenWidth, int screenHeight) {
        // Calcula o centro exato de qualquer monitor
        float cx = (float)screenWidth / 2.0f;
        float cy = (float)screenHeight / 2.0f;

        // Sol no centro dinâmico
        attractor = new Attractor(cx, cy, 300);

        // Planetas alinhados em relação ao centro Y (cy)
        movers.push_back(Mover(cx, cy + 40.0f, 5.00f, 0.0f, 2.0f, 0.5f, 0.5f, 0.5f));   // Mercúrio
        movers.push_back(Mover(cx, cy + 70.0f, 3.78f, 0.0f, 4.0f, 0.9f, 0.7f, 0.2f));   // Vênus
        movers.push_back(Mover(cx, cy + 100.0f, 3.16f, 0.0f, 5.0f, 0.2f, 0.4f, 1.0f));  // Terra
        movers.push_back(Mover(cx, cy + 130.0f, 2.77f, 0.0f, 3.0f, 0.8f, 0.2f, 0.1f));  // Marte
        movers.push_back(Mover(cx, cy + 170.0f, 2.42f, 0.0f, 25.0f, 0.8f, 0.6f, 0.4f)); // Júpiter
        movers.push_back(Mover(cx, cy + 210.0f, 2.18f, 0.0f, 15.0f, 0.9f, 0.8f, 0.6f)); // Saturno
        movers.push_back(Mover(cx, cy + 250.0f, 2.00f, 0.0f, 10.0f, 0.4f, 0.8f, 0.9f)); // Urano
        movers.push_back(Mover(cx, cy + 290.0f, 1.86f, 0.0f, 9.0f, 0.1f, 0.2f, 0.8f));  // Netuno
    }

    ~SolarSystem() {
        delete attractor;
    }

    void onUpdate() {
        for (auto& mover : movers) {
            attractor->attract(mover);
            mover.update();
        }
    }

    void onDisplay() {
        for (auto& mover : movers) {
            drawCircle(mover.pos.x, mover.pos.y, mover.r, 20, mover.r_col, mover.g_col, mover.b_col);
        }
        drawCircle(attractor->pos.x, attractor->pos.y, attractor->r, 40, 1.0f, 0.8f, 0.0f);
    }

    void onKeyboard(unsigned char key, int x, int y) {
        // Reservado
    }
};
#endif // SOLARSYSTEM_HPP