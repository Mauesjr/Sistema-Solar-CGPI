#include <GLFW/glfw3.h>
#include <cmath>

// Importações da nossa nova arquitetura FCIS
#include "core/SistemaSolar.h"
#include "shell/Renderizador.h"

// ImGui
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl2.h"

SolarSystem* solarsystem;

// Variáveis Globais da Câmera
float cameraX = 0.0f;
float cameraY = 0.0f;
float cameraZoom = 1.0f;

// Callbacks de Input
void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    ImGuiIO& io = ImGui::GetIO();
    if (!io.WantCaptureMouse) {
        cameraZoom += (float)yoffset * 0.1f;
        if (cameraZoom < 0.1f) cameraZoom = 0.1f;   
        if (cameraZoom > 10.0f) cameraZoom = 10.0f; 
    }
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        if (key == GLFW_KEY_ESCAPE) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
    }
}

int main(void) {
    if (!glfwInit()) return -1;

    int screenWidth = 1280;
    int screenHeight = 720;
    
    GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "SolarSystem - Functional Core", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    
    glfwSetKeyCallback(window, keyCallback);
    glfwSetScrollCallback(window, scrollCallback);

    // Setup do ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL2_Init();

    // Instancia o nosso Núcleo Funcional
    solarsystem = new SolarSystem(screenWidth, screenHeight);

    // ==========================================
    // Variáveis da Interface e Interação (Shell)
    // ==========================================
    float planetVisualScale = 4.0f; // Isso saiu do núcleo e agora pertence apenas à interface!
    
    bool isPlacementMode = false;
    bool autoOrbit = false;
    float spawnMass = 5.0f;
    float spawnColor[3] = { 1.0f, 1.0f, 1.0f }; 
    
    bool isDragging = false;
    float dragStartX = 0.0f, dragStartY = 0.0f;
    float dragCurrentX = 0.0f, dragCurrentY = 0.0f;
    float slingshotMultiplier = 0.05f;
    bool wasLeftMouseButtonDown = false; 

    bool wasRightMouseButtonDown = false;
    double panStartMouseX = 0.0, panStartMouseY = 0.0;
    float panStartCameraX = 0.0f, panStartCameraY = 0.0f;

    // ==========================================
    // Loop Principal Imperativo
    // ==========================================
    while (!glfwWindowShouldClose(window)) {
        
        ImGui_ImplOpenGL2_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        auto screenToWorld = [&](double sx, double sy, float& wx, float& wy) {
            wx = (float)(sx - screenWidth / 2.0) / cameraZoom + (screenWidth / 2.0f) - cameraX;
            wy = (float)(sy - screenHeight / 2.0) / cameraZoom + (screenHeight / 2.0f) - cameraY;
        };

        bool isLeftMouseButtonDown = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
        bool isRightMouseButtonDown = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;

        // --- LÓGICA DE MOUSE / I/O ---
        if (!io.WantCaptureMouse) {
            
            if (isRightMouseButtonDown && !wasRightMouseButtonDown) {
                glfwGetCursorPos(window, &panStartMouseX, &panStartMouseY);
                panStartCameraX = cameraX;
                panStartCameraY = cameraY;
            } 
            else if (isRightMouseButtonDown) {
                double currentMouseX, currentMouseY;
                glfwGetCursorPos(window, &currentMouseX, &currentMouseY);
                cameraX = panStartCameraX + (float)(currentMouseX - panStartMouseX) / cameraZoom;
                cameraY = panStartCameraY + (float)(currentMouseY - panStartMouseY) / cameraZoom;
            }
            
            if (isPlacementMode) {
                if (autoOrbit) {
                    if (isLeftMouseButtonDown && !wasLeftMouseButtonDown) {
                        double mouseX, mouseY;
                        glfwGetCursorPos(window, &mouseX, &mouseY);
                        
                        float worldX, worldY;
                        screenToWorld(mouseX, mouseY, worldX, worldY); 
                        
                        float finalVx = 0.0f, finalVy = 0.0f;
                        float sunX = solarsystem->getSunPosX();
                        float sunY = solarsystem->getSunPosY();
                        
                        float dx = worldX - sunX;
                        float dy = worldY - sunY;
                        float distance = std::sqrt(dx*dx + dy*dy);
                        
                        if (distance > 0) {
                            float v_mag = std::sqrt((solarsystem->gravityMultiplier * solarsystem->sunMass) / distance);
                            finalVx = -(dy / distance) * v_mag;
                            finalVy = (dx / distance) * v_mag;
                        }

                        solarsystem->addBody(worldX, worldY, finalVx, finalVy, spawnMass, spawnColor[0], spawnColor[1], spawnColor[2]);
                    }
                } else {
                    if (isLeftMouseButtonDown && !wasLeftMouseButtonDown) {
                        isDragging = true;
                        double mouseX, mouseY;
                        glfwGetCursorPos(window, &mouseX, &mouseY);
                        screenToWorld(mouseX, mouseY, dragStartX, dragStartY); 
                        dragCurrentX = dragStartX;
                        dragCurrentY = dragStartY;
                    } 
                    else if (isLeftMouseButtonDown && isDragging) {
                        double mouseX, mouseY;
                        glfwGetCursorPos(window, &mouseX, &mouseY);
                        screenToWorld(mouseX, mouseY, dragCurrentX, dragCurrentY); 
                    } 
                    else if (!isLeftMouseButtonDown && wasLeftMouseButtonDown && isDragging) {
                        isDragging = false;
                        float finalVx = (dragStartX - dragCurrentX) * slingshotMultiplier;
                        float finalVy = (dragStartY - dragCurrentY) * slingshotMultiplier;
                        
                        solarsystem->addBody(dragStartX, dragStartY, finalVx, finalVy, spawnMass, spawnColor[0], spawnColor[1], spawnColor[2]);
                    }
                }
            } 
            else {
                if (isLeftMouseButtonDown) {
                    double mouseX, mouseY;
                    glfwGetCursorPos(window, &mouseX, &mouseY);
                    float worldX, worldY;
                    screenToWorld(mouseX, mouseY, worldX, worldY); 
                    solarsystem->setSunPosition(worldX, worldY);
                }
            }
        } else {
            isDragging = false; 
        }

        wasLeftMouseButtonDown = isLeftMouseButtonDown;
        wasRightMouseButtonDown = isRightMouseButtonDown;

        // ==========================================
        // 1. CHAMA O NÚCLEO FUNCIONAL (Matemática pura)
        // ==========================================
        solarsystem->onUpdate();

        // ==========================================
        // 2. PREPARA O RENDERIZADOR (OpenGL)
        // ==========================================
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
        glTranslatef(screenWidth / 2.0f, screenHeight / 2.0f, 0.0f);
        glScalef(cameraZoom, cameraZoom, 1.0f);
        glTranslatef(-screenWidth / 2.0f, -screenHeight / 2.0f, 0.0f);
        glTranslatef(cameraX, cameraY, 0.0f);

        // ==========================================
        // 3. DESENHA A CENA USANDO A PONTE GRÁFICA
        // ==========================================
        Renderizador::renderizarSistema(*solarsystem, planetVisualScale);

        // Desenha a linha de arrasto (Efeito elástico do mouse)
        if (isDragging && !autoOrbit) {
            glBegin(GL_LINES);
            glColor3f(spawnColor[0], spawnColor[1], spawnColor[2]);
            glVertex2f(dragStartX, dragStartY);
            glColor3f(1.0f, 0.0f, 0.0f); 
            glVertex2f(dragCurrentX, dragCurrentY);
            glEnd();
        }

        // ==========================================
        // 4. DESENHA A INTERFACE (ImGui)
        // ==========================================
        ImGui::Begin("Controles do Sistema Solar");
        
        ImGui::Text("Camera Settings:");
        if (ImGui::Button("Reset Camera")) {
            cameraX = 0.0f;
            cameraY = 0.0f;
            cameraZoom = 1.0f;
        }
        ImGui::Text("Zoom: %.2fx", cameraZoom);
        ImGui::Separator();

        ImGui::Text("Global Physics Settings:");
        ImGui::SliderFloat("Gravitational Force (G)", &solarsystem->gravityMultiplier, 0.1f, 10.0f);
        ImGui::SliderFloat("Sun Mass", &solarsystem->sunMass, 50.0f, 1000.0f);
        
        // O Visual Scale agora aponta para a nossa variável local
        ImGui::SliderFloat("Planet Visual Scale", &planetVisualScale, 1.0f, 15.0f);
        
        ImGui::Checkbox("Enable N-Body Gravity", &solarsystem->enableNBody);
        ImGui::Checkbox("Enable Collisions (Mass Merging)", &solarsystem->enableCollisions);
        ImGui::Checkbox("Contain Planets (Screen Bounds)", &solarsystem->containPlanets);
        
        if (ImGui::Button("Reset System")) {
            delete solarsystem;
            solarsystem = new SolarSystem(screenWidth, screenHeight);
            cameraX = 0.0f; 
            cameraY = 0.0f;
            cameraZoom = 1.0f;
        }

        ImGui::Separator();
        
        ImGui::Text("Sandbox: Body Spawner");
        ImGui::Checkbox("Enable Placement Mode", &isPlacementMode);
        
        if (isPlacementMode) {
            ImGui::Indent();
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Left-Click & Drag to spawn!");
            ImGui::SliderFloat("New Mass", &spawnMass, 0.1f, 50.0f);
            ImGui::Checkbox("Auto-Calculate Stable Orbit", &autoOrbit);
            ImGui::ColorEdit3("Body Color", spawnColor);
            ImGui::Unindent();
        }

        ImGui::Separator();
        ImGui::Text("Performance: %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    ImGui_ImplOpenGL2_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    delete solarsystem;
    glfwTerminate();
    return 0;
}