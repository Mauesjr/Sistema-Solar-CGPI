#pragma once
#ifndef RENDERIZADOR_HPP
#define RENDERIZADOR_HPP

#include "../core/sistemasolar.h" // Importação corrigida para minúsculo

/**
 * @brief Classe responsável pela renderização gráfica do sistema solar.
 * * Esta classe representa a "Casca Imperativa" (Imperative Shell). Ela não guarda
 * estado físico e não possui regras matemáticas do universo. Apenas lê os dados do 
 * núcleo e executa as chamadas do OpenGL para desenhar os pixels na tela.
 */
class Renderizador {
private:
    /**
     * @brief Função auxiliar imperativa para desenhar um círculo usando OpenGL.
     * @param cx Coordenada X do centro.
     * @param cy Coordenada Y do centro.
     * @param r Raio do círculo.
     * @param num_segments Quantidade de triângulos para formar o círculo.
     * @param r_color Cor vermelha (0.0 a 1.0).
     * @param g_color Cor verde (0.0 a 1.0).
     * @param b_color Cor azul (0.0 a 1.0).
     */
    static void drawCircle(float cx, float cy, float r, int num_segments, float r_color, float g_color, float b_color);

public:
    /**
     * @brief Renderiza todo o estado atual do Sistema Solar.
     * Recebe o estado como 'const' para garantir que a renderização jamais 
     * altere a física acidentalmente (Princípio do Functional Core).
     * @param sistema Referência constante (somente leitura) do motor físico.
     * @param visualScale Multiplicador visual para alterar o tamanho dos astros na tela.
     */
    static void renderizarSistema(const SolarSystem& sistema, float visualScale);
};

#endif // RENDERIZADOR_HPP