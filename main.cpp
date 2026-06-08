#include <GLFW/glfw3.h>
#include "SistemaSolar.h"

// ImGui headers for the user interface
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl2.h"

// Global pointer to the simulation engine
SolarSystem* solarsystem;

// Global Camera Variables for 2D Navigation
float cameraX = 0.0f;
float cameraY = 0.0f;
float cameraZoom = 1.0f;

// Scroll callback to handle Zooming in and out
void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    ImGuiIO& io = ImGui::GetIO();
    // Only apply zoom if ImGui is not actively capturing the mouse (e.g., hovering a menu)
    if (!io.WantCaptureMouse) {
        cameraZoom += (float)yoffset * 0.1f;
        // Prevent extreme zoom out or inversion of the rendering matrix
        if (cameraZoom < 0.1f) cameraZoom = 0.1f;   
        // Prevent extreme zoom in which could cause floating point precision issues
        if (cameraZoom > 15.0f) cameraZoom = 15.0f; 
    }
}

// Keyboard callback for standard inputs
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        unsigned char c = 0;
        if (key >= GLFW_KEY_A && key <= GLFW_KEY_Z) {
            c = (unsigned char)key;
            if (!(mods & GLFW_MOD_SHIFT)) c += 32;
        }
        else if (key >= GLFW_KEY_0 && key <= GLFW_KEY_9) c = (unsigned char)key;
        else if (key == GLFW_KEY_ESCAPE) c = 27; // ASCII code for Escape

        // Close the application when Escape is pressed
        if (c == 27) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
    }
}

int main(void) {
    GLFWwindow* window;

    // Initialize the GLFW library
    if (!glfwInit()) return -1;

    // Set initial window dimensions
    int screenWidth = 1280;
    int screenHeight = 720;
    
    // Create a windowed mode window and its OpenGL context
    // The fourth parameter is NULL, ensuring the window is not full-screen
    window = glfwCreateWindow(screenWidth, screenHeight, "SolarSystem at LabEx", NULL, NULL);
    
    if (!window) {
        glfwTerminate();
        return -1;
    }

    // Make the window's context current
    glfwMakeContextCurrent(window);
    // Enable v-sync (1 frame per monitor refresh)
    glfwSwapInterval(1);
    
    // Register GLFW input callbacks
    glfwSetKeyCallback(window, keyCallback);
    glfwSetScrollCallback(window, scrollCallback); 

    // Initialize Dear ImGui Context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    
    // Set ImGui theme to Dark Mode
    ImGui::StyleColorsDark();

    // Setup ImGui Platform/Renderer backends for GLFW and OpenGL2
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL2_Init();

    // Instantiate the physics engine
    solarsystem = new SolarSystem(screenWidth, screenHeight);

    // Sandbox Menu & Placement Variables
    bool isPlacementMode = false;
    bool autoOrbit = false;
    float spawnMass = 5.0f;
    float spawnColor[3] = { 1.0f, 1.0f, 1.0f }; // Default to white
    // Maps to the BodyType enum in SistemaSolar.h (0 = Star, 1 = Planet, 2 = Asteroid, 3 = Black Hole)
    int selectedBodyType = 1; 
    
    // Slingshot State Machine Variables
    bool isDragging = false;
    float dragStartX = 0.0f, dragStartY = 0.0f;
    float dragCurrentX = 0.0f, dragCurrentY = 0.0f;
    float slingshotMultiplier = 0.05f; // Controls the elastic tension of the slingshot
    bool wasLeftMouseButtonDown = false; // Tracks edge cases for mouse clicks

    // Panning State Variables
    bool wasRightMouseButtonDown = false;
    double panStartMouseX = 0.0, panStartMouseY = 0.0;
    float panStartCameraX = 0.0f, panStartCameraY = 0.0f;

    // Main application loop
    while (!glfwWindowShouldClose(window)) {
        
        // Start the Dear ImGui frame
        ImGui_ImplOpenGL2_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Math Helper: Converts Screen Space (pixels) to World Space (simulation coordinates)
        // This is crucial for interacting with the physics engine when the camera is zoomed or panned
        auto screenToWorld = [&](double sx, double sy, float& wx, float& wy) {
            wx = (float)(sx - screenWidth / 2.0) / cameraZoom + (screenWidth / 2.0f) - cameraX;
            wy = (float)(sy - screenHeight / 2.0) / cameraZoom + (screenHeight / 2.0f) - cameraY;
        };

        // Read raw GLFW mouse states
        bool isLeftMouseButtonDown = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
        bool isRightMouseButtonDown = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;

        // Process inputs only if the mouse is not interacting with the ImGui panel
        if (!io.WantCaptureMouse) {
            
            // --- CAMERA PANNING LOGIC ---
            if (isRightMouseButtonDown && !wasRightMouseButtonDown) {
                // Edge detect: Mouse just clicked. Record the starting positions.
                glfwGetCursorPos(window, &panStartMouseX, &panStartMouseY);
                panStartCameraX = cameraX;
                panStartCameraY = cameraY;
            } 
            else if (isRightMouseButtonDown) {
                // Mouse held down: Calculate delta and apply to camera position
                double currentMouseX, currentMouseY;
                glfwGetCursorPos(window, &currentMouseX, &currentMouseY);
                // Divide by cameraZoom so the panning speed matches the visual scale perfectly
                cameraX = panStartCameraX + (float)(currentMouseX - panStartMouseX) / cameraZoom;
                cameraY = panStartCameraY + (float)(currentMouseY - panStartMouseY) / cameraZoom;
            }
            
            // --- PHYSICS PLACEMENT LOGIC ---
            if (isPlacementMode) {
                // Cast the integer from the UI dropdown to the correct strongly-typed enum
                BodyType type = static_cast<BodyType>(selectedBodyType);

                if (autoOrbit) {
                    // Auto-Orbit: Triggered immediately when the button is first pressed
                    if (isLeftMouseButtonDown && !wasLeftMouseButtonDown) {
                        double mouseX, mouseY;
                        glfwGetCursorPos(window, &mouseX, &mouseY);
                        
                        float worldX, worldY;
                        screenToWorld(mouseX, mouseY, worldX, worldY); 
                        
                        float finalVx = 0.0f, finalVy = 0.0f;
                        
                        // Dynamically find the dominant mass (e.g., the heaviest Star) to orbit around
                        Vector2 dominantPos = solarsystem->getDominantGravityCenter();
                        float domMass = solarsystem->getDominantMass();
                        
                        float dx = worldX - dominantPos.x;
                        float dy = worldY - dominantPos.y;
                        float distance = std::sqrt(dx*dx + dy*dy);
                        
                        if (distance > 0) {
                            // Orbital velocity formula: v = sqrt(G * M / r)
                            float v_mag = std::sqrt((solarsystem->gravityMultiplier * domMass) / distance);
                            // Set perpendicular vector for circular orbit
                            finalVx = -(dy / distance) * v_mag;
                            finalVy = (dx / distance) * v_mag;
                        }

                        solarsystem->addBody(type, worldX, worldY, finalVx, finalVy, spawnMass, spawnColor[0], spawnColor[1], spawnColor[2]);
                    }
                } else {
                    // Slingshot Mechanics (Angry Birds style)
                    if (isLeftMouseButtonDown && !wasLeftMouseButtonDown) {
                        // Edge detect: Start dragging
                        isDragging = true;
                        double mouseX, mouseY;
                        glfwGetCursorPos(window, &mouseX, &mouseY);
                        screenToWorld(mouseX, mouseY, dragStartX, dragStartY); 
                        dragCurrentX = dragStartX;
                        dragCurrentY = dragStartY;
                    } 
                    else if (isLeftMouseButtonDown && isDragging) {
                        // Update tension line
                        double mouseX, mouseY;
                        glfwGetCursorPos(window, &mouseX, &mouseY);
                        screenToWorld(mouseX, mouseY, dragCurrentX, dragCurrentY); 
                    } 
                    else if (!isLeftMouseButtonDown && wasLeftMouseButtonDown && isDragging) {
                        // Edge detect: Release and fire
                        isDragging = false;
                        // Calculate vector: Start - Current (Pull back to shoot forward)
                        float finalVx = (dragStartX - dragCurrentX) * slingshotMultiplier;
                        float finalVy = (dragStartY - dragCurrentY) * slingshotMultiplier;
                        
                        solarsystem->addBody(type, dragStartX, dragStartY, finalVx, finalVy, spawnMass, spawnColor[0], spawnColor[1], spawnColor[2]);
                    }
                }
            } 
        } else {
            // Cancel drag operations if the mouse moves over the UI
            isDragging = false; 
        }

        // Store the mouse states for the next frame's edge detection
        wasLeftMouseButtonDown = isLeftMouseButtonDown;
        wasRightMouseButtonDown = isRightMouseButtonDown;

        // ---------------------------------------------------------
        // PHYSICS STEP
        // Execute the mathematical update for gravity and kinematics
        // ---------------------------------------------------------
        solarsystem->onUpdate();

        // ---------------------------------------------------------
        // RENDERING PREPARATION
        // ---------------------------------------------------------
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Handle window resizing dynamically
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        if (height == 0) height = 1;
        glViewport(0, 0, width, height);

        // Setup Orthographic Projection
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0, width, height, 0, -1, 1);

        // --- CAMERA MATRIX TRANSFORMATIONS ---
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        
        // Transform origin to the center of the screen before zooming
        glTranslatef(screenWidth / 2.0f, screenHeight / 2.0f, 0.0f);
        // Apply Zoom scale
        glScalef(cameraZoom, cameraZoom, 1.0f);
        // Return origin to top-left
        glTranslatef(-screenWidth / 2.0f, -screenHeight / 2.0f, 0.0f);
        // Apply Pan Offset
        glTranslatef(cameraX, cameraY, 0.0f);

        // ---------------------------------------------------------
        // DRAW SCENE
        // ---------------------------------------------------------
        solarsystem->onDisplay();

        // Render the slingshot tension line if currently aiming
        if (isDragging && !autoOrbit) {
            glBegin(GL_LINES);
            // Anchor point (uses selected body color)
            glColor3f(spawnColor[0], spawnColor[1], spawnColor[2]);
            glVertex2f(dragStartX, dragStartY);
            // End point (red to signify tension)
            glColor3f(1.0f, 0.0f, 0.0f); 
            glVertex2f(dragCurrentX, dragCurrentY);
            glEnd();
        }

        // ---------------------------------------------------------
        // BUILD IMGUI INTERFACE
        // ---------------------------------------------------------
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
        ImGui::SliderFloat("Planet Visual Scale", &solarsystem->visualScale, 1.0f, 15.0f);
        ImGui::Checkbox("Enable N-Body Gravity", &solarsystem->enableNBody);
        ImGui::Checkbox("Enable Collisions (Mass Merging)", &solarsystem->enableCollisions);
        ImGui::Checkbox("Contain Planets (Screen Bounds)", &solarsystem->containPlanets);
        
        if (ImGui::Button("Reset System")) {
            delete solarsystem;
            solarsystem = new SolarSystem(screenWidth, screenHeight);
            // Center the camera when resetting the physics
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
            
            // Dropdown menu for selecting celestial body types
            const char* bodyTypes[] = { "Star", "Planet", "Asteroid", "Black Hole" };
            ImGui::Combo("Body Type", &selectedBodyType, bodyTypes, IM_ARRAYSIZE(bodyTypes));

            ImGui::SliderFloat("New Mass", &spawnMass, 0.1f, 1000.0f);
            
            // Helper button to set thematic colors based on the chosen entity
            if (ImGui::Button("Reset Color for Type")) {
                if (selectedBodyType == 0) { spawnColor[0] = 1.0f; spawnColor[1] = 0.8f; spawnColor[2] = 0.0f; } // Star (Yellow)
                else if (selectedBodyType == 1) { spawnColor[0] = 0.5f; spawnColor[1] = 0.5f; spawnColor[2] = 0.5f; } // Planet (Grey)
                else if (selectedBodyType == 2) { spawnColor[0] = 0.6f; spawnColor[1] = 0.6f; spawnColor[2] = 0.6f; } // Asteroid (Light Grey)
                else if (selectedBodyType == 3) { spawnColor[0] = 0.0f; spawnColor[1] = 0.0f; spawnColor[2] = 0.0f; } // Black Hole (Black)
            }

            ImGui::Checkbox("Auto-Calculate Stable Orbit", &autoOrbit);
            ImGui::ColorEdit3("Body Color", spawnColor);
            ImGui::Unindent();
        }

        ImGui::Separator();
        ImGui::Text("Performance: %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
        ImGui::End();

        // Render ImGui data over the OpenGL scene
        ImGui::Render();
        ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());

        // Swap front and back buffers
        glfwSwapBuffers(window);
        // Poll for and process events
        glfwPollEvents();
    }

    // Cleanup resources
    ImGui_ImplOpenGL2_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    delete solarsystem;
    glfwTerminate();
    return 0;
}