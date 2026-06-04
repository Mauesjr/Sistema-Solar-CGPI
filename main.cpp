#include <GLFW/glfw3.h>
#include "SistemaSolar.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl2.h"

SolarSystem* solarsystem;

// Global Camera Variables
float cameraX = 0.0f;
float cameraY = 0.0f;
float cameraZoom = 1.0f;

// Scroll callback to handle Zooming
void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    ImGuiIO& io = ImGui::GetIO();
    // Only zoom if ImGui is not using the mouse
    if (!io.WantCaptureMouse) {
        cameraZoom += (float)yoffset * 0.1f;
        if (cameraZoom < 0.1f) cameraZoom = 0.1f;   // Prevent extreme zoom out / inverting
        if (cameraZoom > 10.0f) cameraZoom = 10.0f; // Prevent extreme zoom in
    }
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        unsigned char c = 0;
        if (key >= GLFW_KEY_A && key <= GLFW_KEY_Z) {
            c = (unsigned char)key;
            if (!(mods & GLFW_MOD_SHIFT)) c += 32;
        }
        else if (key >= GLFW_KEY_0 && key <= GLFW_KEY_9) c = (unsigned char)key;
        else if (key == GLFW_KEY_ESCAPE) c = 27;

        if (c == 27) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
    }
}

int main(void) {
    GLFWwindow* window;

    if (!glfwInit()) return -1;

    int screenWidth = 1280;
    int screenHeight = 720;
    
    window = glfwCreateWindow(screenWidth, screenHeight, "SolarSystem at LabEx", NULL, NULL);
    
    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    
    // Register callbacks
    glfwSetKeyCallback(window, keyCallback);
    glfwSetScrollCallback(window, scrollCallback); // Register zoom callback

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL2_Init();

    solarsystem = new SolarSystem(screenWidth, screenHeight);

    // Physics / Placement variables
    bool isPlacementMode = false;
    bool autoOrbit = false;
    float spawnMass = 5.0f;
    float spawnColor[3] = { 1.0f, 1.0f, 1.0f }; 
    
    bool isDragging = false;
    float dragStartX = 0.0f, dragStartY = 0.0f;
    float dragCurrentX = 0.0f, dragCurrentY = 0.0f;
    float slingshotMultiplier = 0.05f;
    bool wasLeftMouseButtonDown = false; 

    // Panning state variables
    bool wasRightMouseButtonDown = false;
    double panStartMouseX = 0.0, panStartMouseY = 0.0;
    float panStartCameraX = 0.0f, panStartCameraY = 0.0f;

    while (!glfwWindowShouldClose(window)) {
        
        ImGui_ImplOpenGL2_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // 1. Math Helper: Convert Screen Space (Pixels) to World Space (Simulation)
        auto screenToWorld = [&](double sx, double sy, float& wx, float& wy) {
            wx = (float)(sx - screenWidth / 2.0) / cameraZoom + (screenWidth / 2.0f) - cameraX;
            wy = (float)(sy - screenHeight / 2.0) / cameraZoom + (screenHeight / 2.0f) - cameraY;
        };

        bool isLeftMouseButtonDown = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
        bool isRightMouseButtonDown = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;

        if (!io.WantCaptureMouse) {
            
            // --- CAMERA PANNING LOGIC (Right Mouse Button) ---
            if (isRightMouseButtonDown && !wasRightMouseButtonDown) {
                glfwGetCursorPos(window, &panStartMouseX, &panStartMouseY);
                panStartCameraX = cameraX;
                panStartCameraY = cameraY;
            } 
            else if (isRightMouseButtonDown) {
                double currentMouseX, currentMouseY;
                glfwGetCursorPos(window, &currentMouseX, &currentMouseY);
                // Divide by zoom so the panning speed matches the scale perfectly
                cameraX = panStartCameraX + (float)(currentMouseX - panStartMouseX) / cameraZoom;
                cameraY = panStartCameraY + (float)(currentMouseY - panStartMouseY) / cameraZoom;
            }
            
            // --- PHYSICS PLACEMENT LOGIC (Left Mouse Button) ---
            if (isPlacementMode) {
                if (autoOrbit) {
                    if (isLeftMouseButtonDown && !wasLeftMouseButtonDown) {
                        double mouseX, mouseY;
                        glfwGetCursorPos(window, &mouseX, &mouseY);
                        
                        float worldX, worldY;
                        screenToWorld(mouseX, mouseY, worldX, worldY); // Convert!
                        
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
                        screenToWorld(mouseX, mouseY, dragStartX, dragStartY); // Convert!
                        dragCurrentX = dragStartX;
                        dragCurrentY = dragStartY;
                    } 
                    else if (isLeftMouseButtonDown && isDragging) {
                        double mouseX, mouseY;
                        glfwGetCursorPos(window, &mouseX, &mouseY);
                        screenToWorld(mouseX, mouseY, dragCurrentX, dragCurrentY); // Convert!
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
                    screenToWorld(mouseX, mouseY, worldX, worldY); // Convert!
                    
                    solarsystem->setSunPosition(worldX, worldY);
                }
            }
        } else {
            isDragging = false; 
        }

        wasLeftMouseButtonDown = isLeftMouseButtonDown;
        wasRightMouseButtonDown = isRightMouseButtonDown;

        // Step physics
        solarsystem->onUpdate();

        // Render prep
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        if (height == 0) height = 1;
        glViewport(0, 0, width, height);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0, width, height, 0, -1, 1);

        // --- CAMERA MATRIX TRANSFORMATIONS ---
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        
        // Transform origin to center of screen for zooming
        glTranslatef(screenWidth / 2.0f, screenHeight / 2.0f, 0.0f);
        // Apply Zoom
        glScalef(cameraZoom, cameraZoom, 1.0f);
        // Return origin
        glTranslatef(-screenWidth / 2.0f, -screenHeight / 2.0f, 0.0f);
        // Apply Pan Offset
        glTranslatef(cameraX, cameraY, 0.0f);
        // -------------------------------------

        solarsystem->onDisplay();

        if (isDragging && !autoOrbit) {
            glBegin(GL_LINES);
            glColor3f(spawnColor[0], spawnColor[1], spawnColor[2]);
            glVertex2f(dragStartX, dragStartY);
            glColor3f(1.0f, 0.0f, 0.0f); 
            glVertex2f(dragCurrentX, dragCurrentY);
            glEnd();
        }

        // Build UI (UI is completely unaffected by the camera matrix)
        ImGui::Begin("Solar System Controls");
        
        ImGui::Text("Camera Settings:");
        if (ImGui::Button("Reset Camera")) {
            cameraX = 0.0f;
            cameraY = 0.0f;
            cameraZoom = 1.0f;
        }
        ImGui::Text("Zoom: %.2fx", cameraZoom);
        ImGui::Text("Tip: Use Scroll Wheel to Zoom, Right-Click to Pan");
        ImGui::Separator();

        ImGui::Text("Global Physics Settings:");
        ImGui::SliderFloat("Gravitational Force (G)", &solarsystem->gravityMultiplier, 0.1f, 10.0f);
        ImGui::SliderFloat("Sun Mass", &solarsystem->sunMass, 50.0f, 1000.0f);
        
        ImGui::Checkbox("Enable N-Body Gravity", &solarsystem->enableNBody);
        ImGui::Checkbox("Contain Planets (Screen Bounds)", &solarsystem->containPlanets);
        ImGui::Checkbox("Enable Collisions (Mass Merging)", &solarsystem->enableCollisions);
        
        if (ImGui::Button("Reset System")) {
            delete solarsystem;
            solarsystem = new SolarSystem(screenWidth, screenHeight);
            cameraX = 0.0f; // Also reset camera on system reset
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