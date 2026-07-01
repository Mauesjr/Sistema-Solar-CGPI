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

// Include the new Shader utility and core components
#include "shell/Shader.h"
#include "core/Particle.h"
#include "shell/RenderSystem.h"
#include "shell/Camera.h"

// Camera instantiation (placed slightly back and looking at the grid)
Camera camera(glm::vec3(10.0f, 10.0f, 30.0f));
float lastX = 1280.0f / 2.0;
float lastY = 720.0f / 2.0;
bool firstMouse = true;
bool captureMouse = false; // Toggle with a key to free the mouse for ImGui

// Timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Callback for window resizing
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    if (!captureMouse) return; // Do not move camera if we are interacting with ImGui

    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // Reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

void processInput(GLFWwindow *window) {
    // Toggle mouse capture (Press 'C' to toggle between Camera control and UI control)
    static bool cKeyPressed = false;
    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) {
        if (!cKeyPressed) {
            captureMouse = !captureMouse;
            if (captureMouse) {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                firstMouse = true; // Reset mouse to prevent jumping
            } else {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            }
            cKeyPressed = true;
        }
    } else {
        cKeyPressed = false;
    }

    if (!captureMouse) return;

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
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Error: Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // Set OpenGL version to 4.5 Core
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create the application window
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Fluid Simulator 3D - Shader Test", NULL, NULL);
    if (!window) {
        std::cerr << "Error: Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSwapInterval(1); // Enable V-Sync
    glfwSetCursorPosCallback(window, mouse_callback);

    // Load OpenGL function pointers using GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Error: Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // APPLICATION SCOPE BLOCK
    // This scope ensures that all OpenGL objects (like RenderSystem and Shader)
    // are properly destructed BEFORE glfwTerminate() is called. 
    // This prevents the Segmentation Fault on application exit.
    {
        // Define the simulation size and initial particle state
        const unsigned int NUM_PARTICLES = 40000;
        std::vector<Particle> particles(NUM_PARTICLES);

        // Initialize particles in a 3D grid formation
        int index = 0;
        float spacing = 0.5f;
        for (int x = 0; x < 40; x++) {
            for (int y = 0; y < 40; y++) {
                for (int z = 0; z < 25; z++) {
                    if (index < NUM_PARTICLES) {
                        particles[index].position = glm::vec4(x * spacing, y * spacing, z * spacing, 0.0f);
                        particles[index].velocity = glm::vec4(0.0f);
                        index++;
                    }
                }
            }
        }

        // Initialize the rendering system and upload particles to the SSBO
        RenderSystem renderer;
        renderer.init(particles);

        // Instantiate the shader for rendering particles
        // If compilation fails, the Shader class will print the error to the console.
        Shader testShader("../shaders/test.vert", "../shaders/test.frag");
        std::string shaderStatus = "Shader Program ID: " + std::to_string(testShader.ID);
        
        // Bind the shader program
        testShader.use();

        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        ImGui::StyleColorsDark();

        // Setup Platform/Renderer backends for ImGui
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 450");

        // Perform a quick GLM math verification
        glm::vec4 testVector(1.0f, 0.0f, 0.0f, 1.0f);
        glm::mat4 testMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(5.0f, 3.0f, 0.0f));
        glm::vec4 resultVector = testMatrix * testVector;
        
        std::string glmResultText = "GLM Test Vector result: (" + 
                                    std::to_string(resultVector.x) + ", " + 
                                    std::to_string(resultVector.y) + ", " + 
                                    std::to_string(resultVector.z) + ")";

        // Main application loop
        while (!glfwWindowShouldClose(window)) {
            // Per-frame time logic
            float currentFrame = static_cast<float>(glfwGetTime());
            deltaTime = currentFrame - lastFrame;
            lastFrame = currentFrame;

            // Input processing (keyboard movement and mouse capture toggle)
            processInput(window);
            glfwPollEvents();

            // Start the ImGui frame
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            // Build the UI Window with dependency and shader statuses
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
            
            // Add instructions for the new camera system
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "Controls:");
            ImGui::Text("Press 'C' to toggle Mouse Capture");
            ImGui::Text("WASD + Space/Shift to fly");
            ImGui::End();

            // Get current window size for the viewport
            int display_w, display_h;
            glfwGetFramebufferSize(window, &display_w, &display_h);
            glViewport(0, 0, display_w, display_h);
            
            // Single clear call for the entire frame color buffer
            glClearColor(0.15f, 0.15f, 0.20f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            // Ensure shader is active before sending uniforms
            testShader.use();

            // Calculate and pass the Projection matrix
            // Prevent division by zero if the window is minimized
            float aspect = (display_h == 0) ? 1.0f : (float)display_w / (float)display_h;
            glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), aspect, 0.1f, 1000.0f);
            testShader.setMat4("projection", projection);

            // Calculate and pass the View matrix
            glm::mat4 view = camera.GetViewMatrix();
            testShader.setMat4("view", view);

            // Draw the fluid particles from the SSBO in 3D space
            renderer.draw(testShader, NUM_PARTICLES);

            // Render ImGui over the 3D scene
            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            // Swap front and back buffers
            glfwSwapBuffers(window);
        }
    } // END OF APPLICATION SCOPE BLOCK

    // Cleanup ImGui resources
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // Destroy the window and terminate GLFW
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}