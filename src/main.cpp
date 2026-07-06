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

// Orbit camera state
glm::vec3 orbitTarget(20.0f, 20.0f, 20.0f);
float orbitRadius = 65.0f;
float orbitTheta = glm::radians(-90.0f);
float orbitPhi = glm::radians(20.0f);
bool autoRotate = false;
float autoRotateSpeed = 0.3f;

// Mouse interaction (repel)
glm::vec3 interactionPoint(0.0f);
bool interactionActive = false;
bool interactionEnabled = false;
float interactionRadius = 6.0f;
float interactionForce = 50.0f;

// Mouse interaction (pull)
bool interactionPull = false;
float pullRadius = 6.0f;
float pullForce = 50.0f;

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
    static bool xKeyPressed = false;
    
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

    // Toggle auto-rotation using the 'X' key
    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) {
        if (!xKeyPressed) {
            autoRotate = !autoRotate;
            if (autoRotate && captureMouse) {
                captureMouse = false;
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            }
            xKeyPressed = true;
        }
    } else {
        xKeyPressed = false;
    }

    // Toggle mouse-fluid interaction using the 'R' key
    static bool aKeyPressed = false;
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
        if (!aKeyPressed) {
            interactionEnabled = !interactionEnabled;
            if (interactionEnabled) interactionPull = false;
            aKeyPressed = true;
        }
    } else {
        aKeyPressed = false;
    }

    // Toggle pull mode using the 'P' key
    static bool pKeyPressed = false;
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
        if (!pKeyPressed) {
            interactionPull = !interactionPull;
            if (interactionPull) interactionEnabled = false;
            pKeyPressed = true;
        }
    } else {
        pKeyPressed = false;
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
        const unsigned int HASH_TABLE_SIZE = NUM_PARTICLES * 4;
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
        glBufferData(GL_SHADER_STORAGE_BUFFER, HASH_TABLE_SIZE * sizeof(unsigned int), nullptr, GL_DYNAMIC_DRAW);
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
        Shader testShader("shaders/test.vert", "shaders/test.frag");
        Shader boxShader("shaders/box.vert", "shaders/box.frag");
        Shader physicsComputeShader("shaders/physics.comp");
        Shader hashShader("shaders/hash.comp");
        Shader sortShader("shaders/sort.comp");
        Shader offsetsShader("shaders/offsets.comp");
        Shader fluidDepthShader("shaders/fluid_depth.vert", "shaders/fluid_depth.frag");
        Shader fluidBlurShader("shaders/fluid_surface.vert", "shaders/fluid_blur.frag");
        Shader fluidSurfaceShader("shaders/fluid_surface.vert", "shaders/fluid_surface.frag");
        
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

        int colorMode = 0;
        float boxOpacity = 0.5f;

        bool fluidMode = false;
        float fluidBlurRadius = 4.0f;
        float fluidFresnelPower = 3.0f;

        unsigned int fluidDepthFBO, fluidDepthTexture, fluidDepthRBO;
        unsigned int fluidBlurFBO[2], fluidBlurTexture[2];
        unsigned int fluidQuadVAO, fluidQuadVBO, fluidQuadEBO;
        int fluidLastWidth = 0, fluidLastHeight = 0;

        {
            glGenFramebuffers(1, &fluidDepthFBO);
            glGenTextures(1, &fluidDepthTexture);
            glGenFramebuffers(2, fluidBlurFBO);
            glGenTextures(2, fluidBlurTexture);
        }

        // Fullscreen quad for fluid surface rendering
        {
            float quadVertices[] = {
                -1.0f, -1.0f,  0.0f, 0.0f,
                 1.0f, -1.0f,  1.0f, 0.0f,
                 1.0f,  1.0f,  1.0f, 1.0f,
                -1.0f,  1.0f,  0.0f, 1.0f
            };
            unsigned int quadIndices[] = { 0, 1, 2, 2, 3, 0 };
            glGenVertexArrays(1, &fluidQuadVAO);
            glGenBuffers(1, &fluidQuadVBO);
            glGenBuffers(1, &fluidQuadEBO);
            glBindVertexArray(fluidQuadVAO);
            glBindBuffer(GL_ARRAY_BUFFER, fluidQuadVBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, fluidQuadEBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quadIndices), quadIndices, GL_STATIC_DRAW);
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
            glEnableVertexAttribArray(1);
            glBindVertexArray(0);
        }


        // Main rendering and simulation loop
        while (!glfwWindowShouldClose(window)) {
            
            // Calculate frame timing
            float currentFrame = static_cast<float>(glfwGetTime());
            deltaTime = currentFrame - lastFrame;
            lastFrame = currentFrame;

            // Handle input events
            glfwPollEvents();
            processInput(window);

            // Recreate fluid FBO textures if screen size changed
            int fbo_w, fbo_h;
            glfwGetFramebufferSize(window, &fbo_w, &fbo_h);
            if (fbo_w != fluidLastWidth || fbo_h != fluidLastHeight) {
                fluidLastWidth = fbo_w;
                fluidLastHeight = fbo_h;

                // Depth FBO with depth renderbuffer
                glBindTexture(GL_TEXTURE_2D, fluidDepthTexture);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, fbo_w, fbo_h, 0, GL_RG, GL_FLOAT, nullptr);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                glBindFramebuffer(GL_FRAMEBUFFER, fluidDepthFBO);
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fluidDepthTexture, 0);
                glGenRenderbuffers(1, &fluidDepthRBO);
                glBindRenderbuffer(GL_RENDERBUFFER, fluidDepthRBO);
                glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, fbo_w, fbo_h);
                glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, fluidDepthRBO);

                // Blur FBOs
                for (int i = 0; i < 2; i++) {
                    glBindTexture(GL_TEXTURE_2D, fluidBlurTexture[i]);
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, fbo_w, fbo_h, 0, GL_RG, GL_FLOAT, nullptr);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                    glBindFramebuffer(GL_FRAMEBUFFER, fluidBlurFBO[i]);
                    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fluidBlurTexture[i], 0);
                }
                glBindFramebuffer(GL_FRAMEBUFFER, 0);
            }

            // Update auto-rotation angle
            if (autoRotate) {
                orbitTheta += autoRotateSpeed * deltaTime;
            }

            // Prepare ImGui frame
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            // Render user interface components
            ImGui::Begin("Dependency & Utility Status");
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "All dependencies are working!");
            ImGui::SameLine();
            ImGui::Text("    FPS: %.1f", ImGui::GetIO().Framerate);
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
            ImGui::Text("[C] Mouse Capture: %s", captureMouse ? "ON" : "OFF");
            ImGui::Text("[X] Auto-Rotation: %s", autoRotate ? "ON" : "OFF");
            ImGui::Text("[R] Fluid Push: %s", interactionEnabled ? "ON" : "OFF");
            ImGui::Text("[P] Pull Particles: %s", interactionPull ? "ON" : "OFF");
            if (captureMouse) {
                ImGui::Text("WASD + Space/Shift to fly");
            }
            
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
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.5f, 1.0f), "Visuals");
            ImGui::Combo("Particle Color", &colorMode, "Default Blue\0Speed\0Density\0Pressure\0");
            ImGui::SliderFloat("Box Opacity", &boxOpacity, 0.0f, 1.0f);
            ImGui::Checkbox("Fluid Mode (Screen-Space)", &fluidMode);
            if (fluidMode) {
                ImGui::SliderFloat("Blur Radius", &fluidBlurRadius, 1.0f, 5.0f);
                ImGui::SliderFloat("Fresnel Power", &fluidFresnelPower, 1.0f, 10.0f);
            }

            ImGui::Separator();
            ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "Mouse Interaction");
            if (interactionEnabled) {
                ImGui::SliderFloat("Push Radius", &interactionRadius, 1.0f, 15.0f);
                ImGui::SliderFloat("Push Strength", &interactionForce, 1.0f, 150.0f);
                ImGui::Text("Left-click on tank to repel fluid");
            }
            if (interactionPull) {
                ImGui::SliderFloat("Pull Radius", &pullRadius, 1.0f, 15.0f);
                ImGui::SliderFloat("Pull Strength", &pullForce, 1.0f, 150.0f);
                ImGui::Text("Left-click on tank to attract particles");
            }

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

            // Set up camera transformation matrices
            float aspect = (display_h == 0) ? 1.0f : (float)display_w / (float)display_h;
            glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), aspect, 0.1f, 1000.0f);
            glm::mat4 view;
            if (autoRotate) {
                glm::vec3 pos(
                    orbitTarget.x + orbitRadius * cos(orbitPhi) * sin(orbitTheta),
                    orbitTarget.y + orbitRadius * sin(orbitPhi),
                    orbitTarget.z + orbitRadius * cos(orbitPhi) * cos(orbitTheta)
                );
                view = glm::lookAt(pos, orbitTarget, glm::vec3(0.0f, 1.0f, 0.0f));
            } else {
                view = camera.GetViewMatrix();
            }

            // Mouse Ray Intersection for Fluid Interaction
            bool anyInteraction = interactionEnabled || interactionPull;
            if (anyInteraction && glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && !captureMouse && !io.WantCaptureMouse) {
                double mx, my;
                glfwGetCursorPos(window, &mx, &my);
                float ndcX = (2.0f * mx) / display_w - 1.0f;
                float ndcY = 1.0f - (2.0f * my) / display_h;
                glm::vec4 rayClip(ndcX, ndcY, -1.0f, 1.0f);
                glm::vec4 rayEye = glm::inverse(projection) * rayClip;
                rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0f, 0.0f);
                glm::vec3 rayDir = glm::normalize(glm::vec3(glm::inverse(view) * rayEye));
                glm::vec3 camPos = glm::vec3(glm::inverse(view)[3]);
                glm::vec3 targetVec = orbitTarget - camPos;
                float t = glm::dot(targetVec, rayDir);
                interactionPoint = camPos + t * rayDir;
                interactionActive = true;
            } else {
                interactionActive = false;
            }

            // Compute optimal workgroup distribution
            int workgroups = (NUM_PARTICLES + 1023) / 1024;
            int hashTableWorkgroups = (HASH_TABLE_SIZE + 1023) / 1024;
            
            // Apply a fixed timestep to ensure mathematical stability in SPH computations
            float physicsDeltaTime = 0.008f;

            // Execute Compute Phase 1: Spatial Hashing
            hashShader.use();
            hashShader.setInt("numParticles", NUM_PARTICLES);
            hashShader.setFloat("smoothingRadius", smoothingRadius);
            hashShader.setUInt("hashTableSize", HASH_TABLE_SIZE);
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
            offsetsShader.setUInt("hashTableSize", HASH_TABLE_SIZE);
            offsetsShader.setInt("currentPass", 0); // Pass 0 clears the buffer
            glDispatchCompute(hashTableWorkgroups, 1, 1);
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
            physicsComputeShader.setUInt("hashTableSize", HASH_TABLE_SIZE);
            physicsComputeShader.setVec3("interactionPoint", interactionPoint);
            physicsComputeShader.setFloat("interactionRadius", interactionRadius);
            physicsComputeShader.setFloat("interactionForce", interactionForce);
            physicsComputeShader.setBool("interactionActive", interactionActive);
            physicsComputeShader.setBool("interactionEnabled", interactionEnabled);
            physicsComputeShader.setBool("interactionPull", interactionPull);
            physicsComputeShader.setFloat("pullRadius", pullRadius);
            physicsComputeShader.setFloat("pullForce", pullForce);

            physicsComputeShader.setInt("currentPass", 0); // Density accumulation pass
            glDispatchCompute(workgroups, 1, 1);
            glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
            
            physicsComputeShader.setInt("currentPass", 1); // Force calculation and integration pass
            glDispatchCompute(workgroups, 1, 1);
            glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

            if (fluidMode) {
                // --- Fluid Mode: Screen-Space Depth + Blur + Surface ---
                glDisable(GL_DEPTH_TEST);

                // Pass 1: Render particle depth
                glBindFramebuffer(GL_FRAMEBUFFER, fluidDepthFBO);
                glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                glEnable(GL_DEPTH_TEST);
                fluidDepthShader.use();
                fluidDepthShader.setMat4("projection", projection);
                fluidDepthShader.setMat4("view", view);
                fluidDepthShader.setFloat("particleRadius", particleRadius);
                fluidDepthShader.setInt("colorMode", colorMode);
                renderer.draw(fluidDepthShader, NUM_PARTICLES);

                // Pass 2: Horizontal blur
                glBindFramebuffer(GL_FRAMEBUFFER, fluidBlurFBO[0]);
                glClear(GL_COLOR_BUFFER_BIT);
                fluidBlurShader.use();
                fluidBlurShader.setInt("depthTexture", 0);
                fluidBlurShader.setInt("horizontal", 1);
                fluidBlurShader.setFloat("blurRadius", fluidBlurRadius);
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, fluidDepthTexture);
                glBindVertexArray(fluidQuadVAO);
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

                // Pass 3: Vertical blur
                glBindFramebuffer(GL_FRAMEBUFFER, fluidBlurFBO[1]);
                glClear(GL_COLOR_BUFFER_BIT);
                fluidBlurShader.setInt("horizontal", 0);
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, fluidBlurTexture[0]);
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

                // Pass 4: Surface reconstruction
                glBindFramebuffer(GL_FRAMEBUFFER, 0);
                glClearColor(0.15f, 0.15f, 0.20f, 1.0f);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                fluidSurfaceShader.use();
                fluidSurfaceShader.setMat4("projection", projection);
                fluidSurfaceShader.setMat4("view", view);
                fluidSurfaceShader.setFloat("fresnelPower", fluidFresnelPower);
                fluidSurfaceShader.setInt("colorMode", colorMode);
                fluidSurfaceShader.setVec2("screenSize", glm::vec2((float)fbo_w, (float)fbo_h));
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, fluidBlurTexture[1]);
                fluidSurfaceShader.setInt("depthTexture", 0);
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

                glBindVertexArray(0);
                glEnable(GL_DEPTH_TEST);

                // Draw bounding box over the fluid surface
                boxShader.use();
                boxShader.setMat4("projection", projection);
                boxShader.setMat4("view", view);
                boxShader.setFloat("boxOpacity", boxOpacity);
                renderer.drawBoundingBox(boxShader);
            } else {
                // --- Standard Mode: Instanced sphere rendering ---
                testShader.use();
                testShader.setMat4("projection", projection);
                testShader.setMat4("view", view);
                testShader.setFloat("particleRadius", particleRadius);
                testShader.setInt("colorMode", colorMode);
                testShader.setVec3("cameraPos", glm::vec3(glm::inverse(view)[3]));
                renderer.draw(testShader, NUM_PARTICLES);

                boxShader.use();
                boxShader.setMat4("projection", projection);
                boxShader.setMat4("view", view);
                boxShader.setFloat("boxOpacity", boxOpacity);
                renderer.drawBoundingBox(boxShader);
            }

            // Dispatch UI rendering commands
            glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            // Swap double buffers to present the final frame
            glfwSwapBuffers(window);
        }

        // Cleanup fluid rendering resources
        glDeleteFramebuffers(1, &fluidDepthFBO);
        glDeleteTextures(1, &fluidDepthTexture);
        glDeleteRenderbuffers(1, &fluidDepthRBO);
        glDeleteFramebuffers(2, fluidBlurFBO);
        glDeleteTextures(2, fluidBlurTexture);
        glDeleteVertexArrays(1, &fluidQuadVAO);
        glDeleteBuffers(1, &fluidQuadVBO);
        glDeleteBuffers(1, &fluidQuadEBO);

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