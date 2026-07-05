#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
#include <vector>

// ImGui headers
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

// GLM headers
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Internal project headers
#include "shell/Shader.h"
#include "core/Particle.h"
#include "shell/RenderSystem.h"
#include "shell/Camera.h"

// Initialize camera positioned to view the entire 40x40x40 simulation tank
Camera camera(glm::vec3(20.0f, 20.0f, 80.0f));

// Mouse input state variables
float lastX = 1280.0f / 2.0f;
float lastY = 720.0f / 2.0f;
bool firstMouse = true;
bool captureMouse = false; 

// Application timing variables
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Callback to adjust the OpenGL viewport when the window size changes
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

// Callback to handle mouse movement for camera orientation
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    if (!captureMouse) {
        return; 
    }

    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; 

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// Process keyboard input for camera movement and application controls
void processInput(GLFWwindow *window) {
    static bool cKeyPressed = false;
    
    // Toggle mouse capture mode using the 'C' key
    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) {
        if (!cKeyPressed) {
            captureMouse = !captureMouse;
            if (captureMouse) {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                firstMouse = true; 
            } else {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            }
            cKeyPressed = true;
        }
    } else {
        cKeyPressed = false;
    }

    if (!captureMouse) {
        return;
    }

    // Camera positional movement
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        camera.ProcessKeyboard(UP, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        camera.ProcessKeyboard(DOWN, deltaTime);
}

int main() {
    // Initialize the GLFW library
    if (!glfwInit()) {
        std::cerr << "Error: Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // Configure OpenGL context for version 4.5 Core Profile
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(1280, 720, "Fluid Simulator 3D - Stable Rollback", NULL, NULL);
    if (!window) {
        std::cerr << "Error: Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSwapInterval(1); 
    glfwSetCursorPosCallback(window, mouse_callback);

    // Initialize GLAD to load OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Error: Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Enable required OpenGL features for 3D rendering
    glEnable(GL_PROGRAM_POINT_SIZE);
    glEnable(GL_DEPTH_TEST);

    // Enclose application logic in a scope to ensure destructors run before glfwTerminate()
    {
        // Define the simulation scale
        const unsigned int NUM_PARTICLES = 40000;
        std::vector<Particle> particles(NUM_PARTICLES);

        int index = 0;
        float spacing = 0.5f;
        
        // Offset coordinates to suspend the initial fluid block in the center of the tank
        float offsetX = 10.0f; 
        float offsetY = 15.0f; 
        float offsetZ = 10.0f; 

        // Populate the initial grid of particles
        for (int x = 0; x < 40; x++) {
            for (int y = 0; y < 40; y++) {
                for (int z = 0; z < 25; z++) {
                    if (index < NUM_PARTICLES) {
                        particles[index].position = glm::vec4(
                            (x * spacing) + offsetX, 
                            (y * spacing) + offsetY, 
                            (z * spacing) + offsetZ, 
                            0.0f
                        );
                        particles[index].velocity = glm::vec4(0.0f);
                        index++;
                    }
                }
            }
        }

        RenderSystem renderer;
        renderer.init(particles);

        // Define the structure for spatial hashing pairs
        struct SpatialEntry {
            unsigned int particleIndex;
            unsigned int hash;
        };
        
        // Calculate the next power of two required by the Bitonic Merge Sort algorithm
        unsigned int nextPow2 = 1;
        while (nextPow2 < NUM_PARTICLES) {
            nextPow2 <<= 1;
        }

        // Initialize Shader Storage Buffer Object (SSBO) for the cell offsets lookup table
        unsigned int cellOffsetsSSBO;
        glGenBuffers(1, &cellOffsetsSSBO);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, cellOffsetsSSBO);
        glBufferData(GL_SHADER_STORAGE_BUFFER, NUM_PARTICLES * sizeof(unsigned int), nullptr, GL_DYNAMIC_DRAW);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, cellOffsetsSSBO);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

        // Initialize SSBO for the spatial indices array
        unsigned int spatialIndicesSSBO;
        glGenBuffers(1, &spatialIndicesSSBO);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, spatialIndicesSSBO);
        glBufferData(GL_SHADER_STORAGE_BUFFER, nextPow2 * sizeof(SpatialEntry), nullptr, GL_DYNAMIC_DRAW);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, spatialIndicesSSBO);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

        // Compile and load all shader programs
        Shader testShader("../shaders/test.vert", "../shaders/test.frag");
        Shader boxShader("../shaders/box.vert", "../shaders/box.frag");
        Shader physicsComputeShader("../shaders/physics.comp");
        Shader hashShader("../shaders/hash.comp");
        Shader sortShader("../shaders/sort.comp");
        Shader offsetsShader("../shaders/offsets.comp");
        
        std::string shaderStatus = "Shader Program ID: " + std::to_string(testShader.ID);

        // Initialize ImGui context and backends
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        ImGui::StyleColorsDark();
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 450");

        // Execute mathematical verification for GLM integration
        glm::vec4 testVector(1.0f, 0.0f, 0.0f, 1.0f);
        glm::mat4 testMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(5.0f, 3.0f, 0.0f));
        glm::vec4 resultVector = testMatrix * testVector;
        std::string glmResultText = "GLM Test Vector result: (" + 
                                    std::to_string(resultVector.x) + ", " + 
                                    std::to_string(resultVector.y) + ", " + 
                                    std::to_string(resultVector.z) + ")";

        // Define initial environmental variables
        glm::vec3 gravityForce(0.0f, -9.81f, 0.0f);
        float collisionDamping = 0.3f;
        
        // Define initial Smoothed Particle Hydrodynamics (SPH) parameters
        float particleRadius = 0.25f;
        float smoothingRadius = 1.5f;
        float targetDensity = 1.0f;
        float pressureMultiplier = 200.0f;
        float viscosityStrength = 0.1f;

        // Main rendering and simulation loop
        while (!glfwWindowShouldClose(window)) {
            
            // Calculate frame timing
            float currentFrame = static_cast<float>(glfwGetTime());
            deltaTime = currentFrame - lastFrame;
            lastFrame = currentFrame;

            // Handle input events
            processInput(window);
            glfwPollEvents();

            // Prepare ImGui frame
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            // Render user interface components
            ImGui::Begin("Dependency & Utility Status");
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "All dependencies are working!");
            ImGui::Separator();
            ImGui::Text("OpenGL Version: %s", glGetString(GL_VERSION));
            ImGui::Text("GLSL Version: %s", glGetString(GL_SHADING_LANGUAGE_VERSION));
            ImGui::Spacing();
            ImGui::Text("Shader Compiler Verification:");
            ImGui::Text("%s", shaderStatus.c_str());
            ImGui::Spacing();
            ImGui::Text("GLM Math Verification:");
            ImGui::Text("%s", glmResultText.c_str());
            
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "Controls:");
            ImGui::Text("Press 'C' to toggle Mouse Capture");
            ImGui::Text("WASD + Space/Shift to fly");
            
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::TextColored(ImVec4(0.0f, 0.8f, 1.0f, 1.0f), "Live Physics Tuning");
            ImGui::SliderFloat3("Gravity", glm::value_ptr(gravityForce), -20.0f, 20.0f);
            ImGui::SliderFloat("Wall Damping", &collisionDamping, 0.0f, 1.0f);

            ImGui::Separator();
            ImGui::TextColored(ImVec4(0.0f, 0.8f, 1.0f, 1.0f), "SPH Properties");
            ImGui::SliderFloat("Smoothing Radius", &smoothingRadius, 0.5f, 3.0f);
            ImGui::SliderFloat("Target Density", &targetDensity, 0.1f, 5.0f);
            ImGui::SliderFloat("Pressure Multiplier", &pressureMultiplier, 10.0f, 1000.0f);
            ImGui::SliderFloat("Viscosity", &viscosityStrength, 0.0f, 2.0f);
            ImGui::SliderFloat("Particle Radius", &particleRadius, 0.05f, 1.0f); 

            ImGui::Separator();
            
            // Execute physical state reset if requested by the user
            if (ImGui::Button("Reset Simulation", ImVec2(-1, 30))) {
                renderer.resetParticles(particles);
            }
            ImGui::End();

            // Update viewport configuration
            int display_w, display_h;
            glfwGetFramebufferSize(window, &display_w, &display_h);
            glViewport(0, 0, display_w, display_h);
            
            // Clear color and depth buffers for the new frame
            glClearColor(0.15f, 0.15f, 0.20f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // Compute optimal workgroup distribution
            int workgroups = (NUM_PARTICLES + 1023) / 1024;
            
            // Apply a fixed timestep to ensure mathematical stability in SPH computations
            float physicsDeltaTime = 0.008f;

            // Execute Compute Phase 1: Spatial Hashing
            hashShader.use();
            hashShader.setInt("numParticles", NUM_PARTICLES);
            hashShader.setFloat("smoothingRadius", smoothingRadius);
            glDispatchCompute(workgroups, 1, 1);
            glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

            // Execute Compute Phase 2: Bitonic Merge Sort
            sortShader.use();
            sortShader.setUInt("numParticles", NUM_PARTICLES);
            int sortThreads = nextPow2 / 2;
            int sortWorkgroups = (sortThreads + 1023) / 1024;

            for (unsigned int stage = 2; stage <= nextPow2; stage <<= 1) {
                for (unsigned int step = stage >> 1; step > 0; step >>= 1) {
                    sortShader.setUInt("stage", stage);
                    sortShader.setUInt("step", step);
                    glDispatchCompute(sortWorkgroups, 1, 1);
                    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
                }
            }

            // Execute Compute Phase 3: Spatial Index Offsets Table Generation
            offsetsShader.use();
            offsetsShader.setUInt("numParticles", NUM_PARTICLES);
            offsetsShader.setInt("currentPass", 0); // Pass 0 clears the buffer
            glDispatchCompute(workgroups, 1, 1);
            glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
            
            offsetsShader.setInt("currentPass", 1); // Pass 1 maps the boundaries
            glDispatchCompute(workgroups, 1, 1);
            glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

            // Execute Compute Phase 4: SPH Integration
            physicsComputeShader.use();
            physicsComputeShader.setFloat("deltaTime", physicsDeltaTime);
            physicsComputeShader.setInt("numParticles", NUM_PARTICLES);
            physicsComputeShader.setVec3("gravity", gravityForce);
            physicsComputeShader.setFloat("damping", collisionDamping);
            physicsComputeShader.setFloat("smoothingRadius", smoothingRadius);
            physicsComputeShader.setFloat("targetDensity", targetDensity);
            physicsComputeShader.setFloat("pressureMultiplier", pressureMultiplier);
            physicsComputeShader.setFloat("viscosityStrength", viscosityStrength);

            physicsComputeShader.setInt("currentPass", 0); // Density accumulation pass
            glDispatchCompute(workgroups, 1, 1);
            glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
            
            physicsComputeShader.setInt("currentPass", 1); // Force calculation and integration pass
            glDispatchCompute(workgroups, 1, 1);
            glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

            // Set up camera transformation matrices
            float aspect = (display_h == 0) ? 1.0f : (float)display_w / (float)display_h;
            glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), aspect, 0.1f, 1000.0f);
            glm::mat4 view = camera.GetViewMatrix();

            // Execute Graphics Pipeline: Draw 3D instanced fluid particles
            testShader.use();
            testShader.setMat4("projection", projection);
            testShader.setMat4("view", view);
            testShader.setFloat("particleRadius", particleRadius);
            renderer.draw(testShader, NUM_PARTICLES);

            // Execute Graphics Pipeline: Draw the simulation bounding volume
            boxShader.use();
            boxShader.setMat4("projection", projection);
            boxShader.setMat4("view", view);
            renderer.drawBoundingBox(boxShader);

            // Dispatch UI rendering commands
            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            // Swap double buffers to present the final frame
            glfwSwapBuffers(window);
        }
    } 

    // Release dynamically allocated ImGui resources
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // Terminate windowing system and close process
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}