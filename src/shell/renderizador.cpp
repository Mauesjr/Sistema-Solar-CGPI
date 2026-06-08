#include "renderizador.h"
#include <GLFW/glfw3.h> 
#include <cmath>
#include <vector>
#include <utility>
#include <cstdlib>


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
    desenharEstrelasDeFundo();
    
    // 1. Desenha os rastros (Paths) das órbitas
    for (const auto& body : sistema.bodies) {
        if (body.type == BodyType::PLANET || body.type == BodyType::ASTEROID) {
            glBegin(GL_LINE_STRIP);
            glColor3f(body.r_col, body.g_col, body.b_col);
            for (const auto& point : body.path) {
                glVertex2f(point.x, point.y);
            }
            glEnd();
        }
    }
    
    // 2. Desenha os corpos celestes baseados nos seus tipos
    for (const auto& body : sistema.bodies) {
        float renderRadius = body.r * visualScale;
        
        if (body.type == BodyType::BLACK_HOLE) {
            // Desenha o Horizonte de Eventos (maior e roxo escuro)
            drawCircle(body.pos.x, body.pos.y, renderRadius * 4.0f, 30, 0.2f, 0.0f, 0.3f);
            
            // Desenha a Singularidade central (menor e perfeitamente preta)
            drawCircle(body.pos.x, body.pos.y, renderRadius, 30, 0.0f, 0.0f, 0.0f);
        } else {
            // Estrelas, Planetas e Asteroides usam a renderização padrão
            drawCircle(body.pos.x, body.pos.y, renderRadius, 30, body.r_col, body.g_col, body.b_col);
        }
    }
}

void Renderizador::desenharEstrelasDeFundo() {
    // Variáveis estáticas persistem na memória entre as chamadas da função.
    static std::vector<std::pair<float, float>> estrelas;
    static bool inicializado = false;

    // Gera 1500 estrelas aleatórias APENAS UMA VEZ na primeira execução
    if (!inicializado) {
        for (int i = 0; i < 1500; i++) {
            // Espalha estrelas em um espaço gigante de -15000 a +15000
            float x = (static_cast<float>(std::rand()) / RAND_MAX) * 30000.0f - 15000.0f;
            float y = (static_cast<float>(std::rand()) / RAND_MAX) * 30000.0f - 15000.0f;
            estrelas.push_back({x, y});
        }
        inicializado = true;
    }

    // Informa ao OpenGL que vamos desenhar "Pontos" em vez de linhas ou polígonos
    glPointSize(1.2f); 
    glBegin(GL_POINTS);
    
    for (size_t i = 0; i < estrelas.size(); i++) {
        // Criamos uma pequena variação de cor para dar um efeito de profundidade térmica
        if (i % 4 == 0) {
            glColor4f(0.7f, 0.8f, 1.0f, 0.8f); // Estrelas jovens (Azuladas)
        } else if (i % 7 == 0) {
            glColor4f(1.0f, 0.8f, 0.7f, 0.7f); // Estrelas antigas (Amareladas/Avermelhadas)
        } else {
            glColor4f(1.0f, 1.0f, 1.0f, 0.5f); // Estrelas comuns (Brancas)
        }

        glVertex2f(estrelas[i].first, estrelas[i].second);
    }
    glEnd();
}