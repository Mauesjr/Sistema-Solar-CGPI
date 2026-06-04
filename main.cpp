#include <GLFW/glfw3.h>
#include "SistemaSolar.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl2.h"

SolarSystem* solarsystem;

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

int main(void) {
    GLFWwindow* window;

    if (!glfwInit())
        return -1;

    int screenWidth = 1280;
    int screenHeight = 720;
    
    window = glfwCreateWindow(screenWidth, screenHeight, "SolarSystem at LabEx", NULL, NULL);
    
    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    glfwSetKeyCallback(window, keyCallback);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL2_Init();

    solarsystem = new SolarSystem(screenWidth, screenHeight);

    // Placement Mode Variables
    bool isPlacementMode = false;
    bool autoOrbit = false; // Fixed: Default to false to prioritize Slingshot mechanics
    float spawnMass = 5.0f;
    float spawnColor[3] = { 1.0f, 1.0f, 1.0f }; 
    
    // Slingshot State Machine Variables
    bool isDragging = false;
    double dragStartX = 0.0, dragStartY = 0.0;
    double dragCurrentX = 0.0, dragCurrentY = 0.0;
    float slingshotMultiplier = 0.05f;
    bool wasLeftMouseButtonDown = false; // Tracks the previous frame's mouse state

    while (!glfwWindowShouldClose(window)) {
        
        ImGui_ImplOpenGL2_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Check raw GLFW mouse state
        bool isLeftMouseButtonDown = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;

        if (!io.WantCaptureMouse) {
            if (isPlacementMode) {
                if (autoOrbit) {
                    // Auto-Orbit: Triggered immediately when the button is first pressed
                    if (isLeftMouseButtonDown && !wasLeftMouseButtonDown) {
                        double mouseX, mouseY;
                        glfwGetCursorPos(window, &mouseX, &mouseY);
                        
                        float finalVx = 0.0f, finalVy = 0.0f;
                        float sunX = solarsystem->getSunPosX();
                        float sunY = solarsystem->getSunPosY();
                        
                        float dx = (float)mouseX - sunX;
                        float dy = (float)mouseY - sunY;
                        float distance = std::sqrt(dx*dx + dy*dy);
                        
                        if (distance > 0) {
                            float v_mag = std::sqrt((solarsystem->gravityMultiplier * solarsystem->sunMass) / distance);
                            finalVx = -(dy / distance) * v_mag;
                            finalVy = (dx / distance) * v_mag;
                        }

                        solarsystem->addBody((float)mouseX, (float)mouseY, finalVx, finalVy, spawnMass, spawnColor[0], spawnColor[1], spawnColor[2]);
                    }
                } else {
                    // Slingshot: Track press, hold, and release states
                    if (isLeftMouseButtonDown && !wasLeftMouseButtonDown) {
                        // Edge detect: Mouse just clicked
                        isDragging = true;
                        glfwGetCursorPos(window, &dragStartX, &dragStartY);
                        dragCurrentX = dragStartX;
                        dragCurrentY = dragStartY;
                    } 
                    else if (isLeftMouseButtonDown && isDragging) {
                        // Mouse held down: Update the current drag position
                        glfwGetCursorPos(window, &dragCurrentX, &dragCurrentY);
                    } 
                    else if (!isLeftMouseButtonDown && wasLeftMouseButtonDown && isDragging) {
                        // Edge detect: Mouse just released
                        isDragging = false;
                        float finalVx = (float)(dragStartX - dragCurrentX) * slingshotMultiplier;
                        float finalVy = (float)(dragStartY - dragCurrentY) * slingshotMultiplier;
                        
                        solarsystem->addBody((float)dragStartX, (float)dragStartY, finalVx, finalVy, spawnMass, spawnColor[0], spawnColor[1], spawnColor[2]);
                    }
                }
            } 
            else {
                // Drag the Sun
                if (isLeftMouseButtonDown) {
                    double mouseX, mouseY;
                    glfwGetCursorPos(window, &mouseX, &mouseY);
                    solarsystem->setSunPosition((float)mouseX, (float)mouseY);
                }
            }
        } else {
            // Abort dragging if the cursor moves over the ImGui menu
            isDragging = false; 
        }

        // Store the mouse state for the next frame's edge detection
        wasLeftMouseButtonDown = isLeftMouseButtonDown;

        solarsystem->onUpdate();

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

        solarsystem->onDisplay();

        // Render the slingshot tension line
        if (isDragging && !autoOrbit) {
            glBegin(GL_LINES);
            glColor3f(spawnColor[0], spawnColor[1], spawnColor[2]);
            glVertex2f((float)dragStartX, (float)dragStartY);
            
            glColor3f(1.0f, 0.0f, 0.0f); 
            glVertex2f((float)dragCurrentX, (float)dragCurrentY);
            glEnd();
        }

        // Build the control panel UI
        ImGui::Begin("Solar System Controls");
        ImGui::Text("Global Physics Settings:");
        ImGui::SliderFloat("Gravitational Force (G)", &solarsystem->gravityMultiplier, 0.1f, 10.0f);
        ImGui::SliderFloat("Sun Mass", &solarsystem->sunMass, 50.0f, 1000.0f);
        ImGui::Checkbox("Enable N-Body Gravity", &solarsystem->enableNBody);
        ImGui::Checkbox("Contain Planets (Screen Bounds)", &solarsystem->containPlanets);
        
        if (ImGui::Button("Reset System")) {
            delete solarsystem;
            solarsystem = new SolarSystem(screenWidth, screenHeight);
        }

        ImGui::Separator();
        
        ImGui::Text("Sandbox: Body Spawner");
        ImGui::Checkbox("Enable Placement Mode", &isPlacementMode);
        
        if (isPlacementMode) {
            ImGui::Indent();
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Click anywhere on the screen to spawn!");
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