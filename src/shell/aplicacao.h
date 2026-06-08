#pragma once
#ifndef APLICACAO_HPP
#define APLICACAO_HPP

#include <GLFW/glfw3.h>
#include "../core/sistemasolar.h"

/**
 * @brief Classe que gerencia a janela, a entrada do usuário e o loop principal.
 */
class Aplicacao {
private:
    GLFWwindow* m_window;        ///< Ponteiro para a janela do GLFW
    int m_screenWidth;           ///< Largura da janela
    int m_screenHeight;          ///< Altura da janela
    const char* m_titulo;        ///< Título da janela

    // Ponteiro para o nosso Núcleo Funcional
    SolarSystem* m_sistemaSolar;

    // ==========================================
    // Variáveis de Estado da Câmera
    // ==========================================
    float m_cameraX, m_cameraY, m_cameraZoom;
    float m_panStartCameraX, m_panStartCameraY;
    double m_panStartMouseX, m_panStartMouseY;

    // ==========================================
    // Variáveis de Estado da Interface (UI/ImGui)
    // ==========================================
    float m_planetVisualScale;
    bool m_isPlacementMode;
    bool m_autoOrbit;
    float m_spawnMass;
    float m_spawnColor[3];
    int m_selectedBodyType; ///< Guarda o tipo de corpo celeste selecionado no menu (0 a 3)

    // ==========================================
    // Variáveis de Estado do Mouse
    // ==========================================
    bool m_isDragging;
    float m_dragStartX, m_dragStartY;
    float m_dragCurrentX, m_dragCurrentY;
    bool m_wasLeftMouseButtonDown;
    bool m_wasRightMouseButtonDown;

    std::vector<Vector2> m_predictedPath;

    // ==========================================
    // Funções Internas de Controle
    // ==========================================
    
    /**
     * @brief Processa as entradas de teclado e mouse do usuário.
     */
    void processarEntrada();
    
    /**
     * @brief Calcula a trajetória baseada na física atual (Universo Paralelo).
     */
    void calcularPrevisaoTrajetoria();

    /**
     * @brief Constrói os painéis e botões do ImGui.
     */
    void desenharInterfaceUsuario();
    
    /**
     * @brief Converte as coordenadas do mouse na tela (pixels) para o espaço mundial do motor físico.
     * @param sx Coordenada X na tela.
     * @param sy Coordenada Y na tela.
     * @param wx Referência para armazenar a Coordenada X no mundo.
     * @param wy Referência para armazenar a Coordenada Y no mundo.
     */
    void converterTelaParaMundo(double sx, double sy, float& wx, float& wy);

    // Callbacks do GLFW (precisam ser estáticos na orientação a objetos)
    static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

public:
    /**
     * @brief Construtor da Aplicação.
     * @param largura Largura da janela em pixels.
     * @param altura Altura da janela em pixels.
     * @param titulo Título exibido na barra da janela.
     */
    Aplicacao(int largura, int altura, const char* titulo);
    
    /**
     * @brief Destrutor para limpar a memória do GLFW e ImGui.
     */
    ~Aplicacao();

    /**
     * @brief Inicializa o GLFW, OpenGL e ImGui.
     * @return true se inicializou com sucesso, false caso ocorra erro.
     */
    bool inicializar();

    /**
     * @brief Inicia o loop infinito da simulação e renderização.
     */
    void executarLoop();
};



#endif // APLICACAO_HPP