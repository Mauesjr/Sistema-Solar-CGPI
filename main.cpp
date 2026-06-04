#include <GLFW/glfw3.h>
#include "SistemaSolar.h"

// ImGui headers
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl2.h" // Correct OpenGL2 header

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

    // Windowed mode configuration
    int screenWidth = 1920;
    int screenHeight = 1080;
    
    // The fourth parameter is NULL, ensuring the window is not full-screen
    window = glfwCreateWindow(screenWidth, screenHeight, "SolarSystem at LabEx", NULL, NULL);
    
    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    glfwSetKeyCallback(window, keyCallback);

    // Initialize ImGui Context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();

    // Setup ImGui Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL2_Init(); // Correct OpenGL2 initialization

    solarsystem = new SolarSystem(screenWidth, screenHeight);

    // --- NEW VARIABLES FOR PLACEMENT MODE ---
    bool isPlacementMode = false;
    float spawnMass = 5.0f;
    bool autoOrbit = true;
    float spawnVel[2] = { 0.0f, 0.0f }; // Array for X and Y velocity
    float spawnColor[3] = { 1.0f, 1.0f, 1.0f }; // Array for RGB color (Default White)
    // ----------------------------------------

    while (!glfwWindowShouldClose(window)) {
        // Start the Dear ImGui frame
        ImGui_ImplOpenGL2_NewFrame(); // Correct OpenGL2 frame setup
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // --- UPDATED MOUSE INTERACTION LOGIC ---
        if (!io.WantCaptureMouse) {
            if (isPlacementMode) {
                if (ImGui::IsMouseClicked(0)) {
                    double mouseX, mouseY;
                    glfwGetCursorPos(window, &mouseX, &mouseY);
                    
                    float finalVx = spawnVel[0];
                    float finalVy = spawnVel[1];

                    // Mathematically calculate a perfect circular orbit
                    if (autoOrbit) {
                        // Assuming you make attractor public or create a getAttractorPos() method
                        // For this example, let's pretend we have a method: solarsystem->getSunPosX()
                        // (You'll need to adjust this based on how you expose the sun's position)
                        
                        float sunX = solarsystem->getSunPosX();
                        float sunY = solarsystem->getSunPosY();
                        
                        float dx = (float)mouseX - sunX;
                        float dy = (float)mouseY - sunY;
                        float distance = std::sqrt(dx*dx + dy*dy);
                        
                        if (distance > 0) {
                            // Orbital velocity formula: v = sqrt(G * M / r)
                            float v_mag = std::sqrt((solarsystem->gravityMultiplier * solarsystem->sunMass) / distance);
                            
                            // Perpendicular vector for circular orbit (-dy, dx) normalized
                            finalVx = -(dy / distance) * v_mag;
                            finalVy = (dx / distance) * v_mag;
                        }
                    }

                    solarsystem->addBody(
                        (float)mouseX, (float)mouseY, 
                        finalVx, finalVy, 
                        spawnMass, 
                        spawnColor[0], spawnColor[1], spawnColor[2]
                    );
                }
            } 
            else {
                if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
                    double mouseX, mouseY;
                    glfwGetCursorPos(window, &mouseX, &mouseY);
                    solarsystem->setSunPosition((float)mouseX, (float)mouseY);
                }
            }
        }

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

        // Draw the solar system objects
        solarsystem->onDisplay();

        // Build the control panel UI
        ImGui::Begin("Solar System Controls");
        ImGui::Text("Adjust physics variables in real time:");

        ImGui::SliderFloat("Gravitational Force (G)", &solarsystem->gravityMultiplier, 0.1f, 10.0f);
        ImGui::SliderFloat("Sun Mass", &solarsystem->sunMass, 1.0f, 1000.0f);
        ImGui::Checkbox("Enable N-Body Gravity", &solarsystem->enableNBody);
        ImGui::Checkbox("Contain Planets (Screen Bounds)", &solarsystem->containPlanets);

        if (ImGui::Button("Reset Simulation")) {
            // Re-instatiate the system to original values
            delete solarsystem;
            solarsystem = new SolarSystem(screenWidth, screenHeight);
        }


        ImGui::Separator();

        // New Sandbox Menu
        ImGui::Text("Sandbox: Body Spawner");
        ImGui::Checkbox("Enable Placement Mode", &isPlacementMode);
        
        if (isPlacementMode) {
            ImGui::Indent();
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Click anywhere on the screen to spawn!");
            ImGui::SliderFloat("New Mass", &spawnMass, 0.1f, 50.0f);
            
            ImGui::Checkbox("Auto-Calculate Stable Orbit", &autoOrbit);
            if (!autoOrbit) {
                ImGui::SliderFloat2("Manual Velocity (X, Y)", spawnVel, -10.0f, 10.0f);
            }
            
            ImGui::ColorEdit3("Body Color", spawnColor);
            ImGui::Unindent();
        }

        ImGui::Separator();
        ImGui::Text("Performance: %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
        ImGui::End();

        // Render ImGui over the OpenGL scene
        ImGui::Render();
        ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData()); // Correct OpenGL2 rendering

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup ImGui
    ImGui_ImplOpenGL2_Shutdown(); // Correct OpenGL2 shutdown
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    delete solarsystem;
    glfwTerminate();
    return 0;
}