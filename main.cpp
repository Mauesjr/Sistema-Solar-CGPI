#include <GLFW/glfw3.h>
#include "SistemaSolar.h"

// NOVO: Usamos um PONTEIRO global agora, porque só podemos criar o sistema 
// DEPOIS que descobrirmos o tamanho da tela.
SolarSystem* solarsystem;

// Callback de teclado para o GLFW
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        unsigned char c = 0;
        if (key >= GLFW_KEY_A && key <= GLFW_KEY_Z) {
            c = (unsigned char)key;
            if (!(mods & GLFW_MOD_SHIFT)) c += 32;
        }
        else if (key >= GLFW_KEY_0 && key <= GLFW_KEY_9) c = (unsigned char)key;
        else if (key == GLFW_KEY_ESCAPE) c = 27;
        else if (key == GLFW_KEY_SPACE) c = ' ';

        // Se o usuário apertar ESC, fechamos o jogo (muito útil em Tela Cheia!)
        if (c == 27) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }

        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);

        if (c != 0 && solarsystem != nullptr) {
            solarsystem->onKeyboard(c, (int)xpos, (int)ypos);
        }
    }
}

int main(void)
{
    GLFWwindow* window;

    if (!glfwInit())
        return -1;

    // --- CÓDIGO NOVO: PEGANDO A TELA INTEIRA ---
    // 1. Encontra o seu monitor principal
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();

    // 2. Descobre qual é a resolução atual dele (ex: 1920x1080)
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    int screenWidth = mode->width;
    int screenHeight = mode->height;

    // 3. Cria a janela usando as dimensões reais e passando o 'monitor' para ativar Tela Cheia
    window = glfwCreateWindow(screenWidth, screenHeight, "SolarSystem at LabEx", monitor, NULL);
    // -------------------------------------------

    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    // LIGA O VSYNC! (Mantém em ~60 FPS)
    glfwSwapInterval(1);

    glfwSetKeyCallback(window, keyCallback);

    // AGORA SIM! Criamos o Sistema Solar passando a largura e altura da tela
    solarsystem = new SolarSystem(screenWidth, screenHeight);

    while (!glfwWindowShouldClose(window))
    {
        // 1. Atualiza a física
        solarsystem->onUpdate();

        // 2. Prepara a tela
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        if (height == 0) height = 1;
        glViewport(0, 0, width, height);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0, width, height, 0, -1, 1);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        // 3. Desenha os planetas
        solarsystem->onDisplay();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Limpa a memória ao sair
    delete solarsystem;
    glfwTerminate();
    return 0;
}