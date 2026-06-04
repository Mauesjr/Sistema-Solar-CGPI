#include "renderizador.h"
#include <GLFW/glfw3.h> 
#include <cmath>

void Renderizador::drawCircle(float cx, float cy, float r, int num_segments, float r_color, float g_color, float b_color) {
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

void Renderizador::renderizarSistema(const SolarSystem& sistema, float visualScale) {
    // 1. Desenha os rastros (Paths) das órbitas
    for (const auto& mover : sistema.movers) {
        glBegin(GL_LINE_STRIP);
        glColor3f(mover.r_col, mover.g_col, mover.b_col);
        for (const auto& point : mover.path) {
            glVertex2f(point.x, point.y);
        }
        glEnd();
    }
    
    // 2. Desenha os corpos celestes (Planetas)
    for (const auto& mover : sistema.movers) {
        float renderRadius = mover.r * visualScale;
        drawCircle(mover.pos.x, mover.pos.y, renderRadius, 20, mover.r_col, mover.g_col, mover.b_col);
    }
             
    // 3. Desenha o grande atrator central (Sol)
    if (sistema.attractor != nullptr) {
        // Aplica uma escala reduzida para o Sol não engolir os planetas internos visualmente
        float sunRenderRadius = sistema.attractor->r * (visualScale * 0.5f);
        drawCircle(sistema.attractor->pos.x, sistema.attractor->pos.y, sunRenderRadius, 40, 1.0f, 0.8f, 0.0f);
    }
}