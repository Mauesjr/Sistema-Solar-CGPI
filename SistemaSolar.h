#pragma once
#ifndef SOLARSYSTEM_HPP
#define SOLARSYSTEM_HPP

#include <GLFW/glfw3.h>
#include <cmath>

class SolarSystem {
private:
    float angle; // Controla a rotação dos planetas

    // Adaptei a sua função drawCircle para aceitar cores (R, G, B)
    void drawCircle(float cx, float cy, float r, int num_segments, float r_color, float g_color, float b_color) {
        glBegin(GL_TRIANGLE_FAN);
        glColor3f(r_color, g_color, b_color);
        glVertex2f(cx, cy); // Centro
        for (int i = 0; i <= num_segments; i++) {
            float theta = 2.0f * 3.1415926f * float(i) / float(num_segments);
            float x = r * cosf(theta);
            float y = r * sinf(theta);
            glVertex2f(x + cx, y + cy);
        }
        glEnd();
    }

public:
    // Construtor
    SolarSystem() {
        angle = 0.0f;
    }

    // Chamado todo frame para atualizar a lógica
    void onUpdate() {
        angle += 0.1f; // Velocidade da órbita
        if (angle > 360.0f) {
            angle -= 360.0f;
        }
    }

    // Chamado todo frame para desenhar na tela
    void onDisplay() {
        // --- 1. Desenha o Sol no centro (Amarelo) ---
        glPushMatrix(); // Salva o estado atual da matriz (centro da tela)
        drawCircle(0.0f, 0.0f, 0.2f, 50, 1.0f, 1.0f, 0.0f);
        glPopMatrix();  // Restaura a matriz para o centro

        // --- 2. Desenha a Terra (Azul) ---
        glPushMatrix();
        glRotatef(angle, 0.0f, 0.0f, 1.0f); // Rotaciona a matriz no eixo Z
        glTranslatef(0.6f, 0.0f, 0.0f);     // Move a Terra 0.6 unidades para longe do Sol
        drawCircle(0.0f, 0.0f, 0.08f, 30, 0.0f, 0.0f, 1.0f);

        // --- 3. Desenha a Lua (Cinza) ---
        // Como a matriz já está na Terra, a Lua vai girar ao redor dela
        glRotatef(angle * 3.0f, 0.0f, 0.0f, 1.0f); // A Lua gira 3x mais rápido
        glTranslatef(0.15f, 0.0f, 0.0f);           // Distância da Lua para a Terra
        drawCircle(0.0f, 0.0f, 0.03f, 20, 0.7f, 0.7f, 0.7f);

        glPopMatrix(); // Restaura tudo de volta para o centro da tela
    }

    // Função de teclado exigida pelo main
    void onKeyboard(unsigned char key, int x, int y) {
        // Exemplo: se apertar 'R', reseta o ângulo
        if (key == 'r' || key == 'R') {
            angle = 0.0f;
        }
    }
};

#endif // SOLARSYSTEM_HPP