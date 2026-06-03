#include <GLFW/glfw3.h>
#include "SistemaSolar.h"

// Define as constantes de tamanho da janela
#define WIDTH 700
#define HEIGHT 700

// Instancia o objeto global da classe que acabamos de criar
SolarSystem solarsystem;

// Callback de teclado para o GLFW
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    // Só responde quando a tecla é pressionada ou mantida pressionada
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        unsigned char c = 0;

        // Converte as teclas principais para char
        if (key >= GLFW_KEY_A && key <= GLFW_KEY_Z) {
            c = (unsigned char)key;
            if (!(mods & GLFW_MOD_SHIFT)) c += 32; // Minúscula se shift não estiver pressionado
        }
        else if (key >= GLFW_KEY_0 && key <= GLFW_KEY_9) {
            c = (unsigned char)key;
        }
        else if (key == GLFW_KEY_ESCAPE) {
            c = 27; // Código ASCII do ESC
        }
        else if (key == GLFW_KEY_SPACE) {
            c = ' ';
        }

        // Pega a posição do mouse no momento do clique
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);

        // Passa a tecla e a posição do mouse para a sua classe
        if (c != 0) {
            solarsystem.onKeyboard(c, (int)xpos, (int)ypos);
        }
    }
}

int main(void)
{
    GLFWwindow* window;

    /* Inicializa a biblioteca GLFW */
    if (!glfwInit())
        return -1;

    /* Cria a janela */
    window = glfwCreateWindow(WIDTH, HEIGHT, "SolarSystem at LabEx", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Torna o contexto da janela o contexto atual */
    glfwMakeContextCurrent(window);

    /* Registra a função de teclado no GLFW */
    glfwSetKeyCallback(window, keyCallback);

    /* Laço principal até que o usuário feche a janela */
    while (!glfwWindowShouldClose(window))
    {
        // 1. ATUALIZAÇÃO DA LÓGICA (Gira os planetas)
        solarsystem.onUpdate();

        // 2. PREPARAÇÃO DA RENDERIZAÇÃO
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Mantém a proporção da tela se o usuário redimensionar a janela
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        if (height == 0) height = 1;
        glViewport(0, 0, width, height);

        // 3. RENDERIZAÇÃO (Desenha o Sol, Terra e Lua)
        solarsystem.onDisplay();

        /* Troca os buffers e processa eventos */
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    /* Encerra a biblioteca */
    glfwTerminate();
    return 0;
}