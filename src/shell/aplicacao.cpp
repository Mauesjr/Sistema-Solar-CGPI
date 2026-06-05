#include "aplicacao.h"
#include "renderizador.h"

// ImGui
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl2.h"

#include <cmath>

// ==========================================
// Callbacks Estáticos do GLFW
// ==========================================
void Aplicacao::scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    ImGuiIO& io = ImGui::GetIO();
    if (!io.WantCaptureMouse) {
        Aplicacao* app = static_cast<Aplicacao*>(glfwGetWindowUserPointer(window));
        if (app) {
            app->m_cameraZoom += (float)yoffset * 0.1f;
            if (app->m_cameraZoom < 0.1f) app->m_cameraZoom = 0.1f;   
            if (app->m_cameraZoom > 15.0f) app->m_cameraZoom = 15.0f; // Ajuste do seu colega para permitir mais zoom
        }
    }
}

void Aplicacao::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        if (key == GLFW_KEY_ESCAPE) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
    }
}

// ==========================================
// Construtor e Destrutor
// ==========================================
Aplicacao::Aplicacao(int largura, int altura, const char* titulo) {
    m_screenWidth = largura;
    m_screenHeight = altura;
    m_titulo = titulo;
    m_window = nullptr;

    m_cameraX = 0.0f;
    m_cameraY = 0.0f;
    m_cameraZoom = 1.0f;
    
    m_planetVisualScale = 1.0f; // Ajustado para bater com a escala real por padrão
    m_isPlacementMode = false;
    m_autoOrbit = false;
    m_spawnMass = 5.0f;
    m_spawnColor[0] = 1.0f; m_spawnColor[1] = 1.0f; m_spawnColor[2] = 1.0f;
    m_selectedBodyType = 1; // 1 = PLANETA (Padrão inicial)
    
    m_isDragging = false;
    m_wasLeftMouseButtonDown = false;
    m_wasRightMouseButtonDown = false;

    m_sistemaSolar = new SolarSystem(m_screenWidth, m_screenHeight);
}

Aplicacao::~Aplicacao() {
    delete m_sistemaSolar;
    
    ImGui_ImplOpenGL2_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    if (m_window) glfwDestroyWindow(m_window);
    glfwTerminate();
}

// ==========================================
// Inicialização
// ==========================================
bool Aplicacao::inicializar() {
    if (!glfwInit()) return false;
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.015f, 0.015f, 0.035f, 1.0f);
    
    m_window = glfwCreateWindow(m_screenWidth, m_screenHeight, m_titulo, NULL, NULL);
    if (!m_window) {
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(m_window);
    glfwSwapInterval(1); 
    
    glfwSetWindowUserPointer(m_window, this);
    glfwSetKeyCallback(m_window, keyCallback);
    glfwSetScrollCallback(m_window, scrollCallback);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(m_window, true);
    ImGui_ImplOpenGL2_Init();

    return true;
}

// ==========================================
// Funções Utilitárias e Lógica de Input
// ==========================================
void Aplicacao::converterTelaParaMundo(double sx, double sy, float& wx, float& wy) {
    wx = (float)(sx - m_screenWidth / 2.0) / m_cameraZoom + (m_screenWidth / 2.0f) - m_cameraX;
    wy = (float)(sy - m_screenHeight / 2.0) / m_cameraZoom + (m_screenHeight / 2.0f) - m_cameraY;
}

void Aplicacao::processarEntrada() {
    ImGuiIO& io = ImGui::GetIO();
    bool isLeftMouseButtonDown = glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
    bool isRightMouseButtonDown = glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;

    if (!io.WantCaptureMouse) {
        // --- PANNING DA CÂMERA ---
        if (isRightMouseButtonDown && !m_wasRightMouseButtonDown) {
            glfwGetCursorPos(m_window, &m_panStartMouseX, &m_panStartMouseY);
            m_panStartCameraX = m_cameraX;
            m_panStartCameraY = m_cameraY;
        } 
        else if (isRightMouseButtonDown) {
            double currentMouseX, currentMouseY;
            glfwGetCursorPos(m_window, &currentMouseX, &currentMouseY);
            m_cameraX = m_panStartCameraX + (float)(currentMouseX - m_panStartMouseX) / m_cameraZoom;
            m_cameraY = m_panStartCameraY + (float)(currentMouseY - m_panStartMouseY) / m_cameraZoom;
        }
        
        // --- COLOCAR PLANETAS ---
        if (m_isPlacementMode) {
            BodyType type = static_cast<BodyType>(m_selectedBodyType);

            if (m_autoOrbit) {
                if (isLeftMouseButtonDown && !m_wasLeftMouseButtonDown) {
                    double mouseX, mouseY;
                    glfwGetCursorPos(m_window, &mouseX, &mouseY);
                    
                    float worldX, worldY;
                    converterTelaParaMundo(mouseX, mouseY, worldX, worldY); 
                    
                    float finalVx = 0.0f, finalVy = 0.0f;
                    
                    // Busca dinâmica pelo objeto mais pesado
                    Vector2 dominantPos = m_sistemaSolar->getDominantGravityCenter();
                    float domMass = m_sistemaSolar->getDominantMass();
                    
                    float dx = worldX - dominantPos.x;
                    float dy = worldY - dominantPos.y;
                    float distance = std::sqrt(dx*dx + dy*dy);
                    
                    if (distance > 0) {
                        float v_mag = std::sqrt((m_sistemaSolar->gravityMultiplier * domMass) / distance);
                        finalVx = -(dy / distance) * v_mag;
                        finalVy = (dx / distance) * v_mag;
                    }

                    m_sistemaSolar->addBody(type, worldX, worldY, finalVx, finalVy, m_spawnMass, m_spawnColor[0], m_spawnColor[1], m_spawnColor[2]);
                }
            } else {
                if (isLeftMouseButtonDown && !m_wasLeftMouseButtonDown) {
                    m_isDragging = true;
                    double mouseX, mouseY;
                    glfwGetCursorPos(m_window, &mouseX, &mouseY);
                    converterTelaParaMundo(mouseX, mouseY, m_dragStartX, m_dragStartY); 
                    m_dragCurrentX = m_dragStartX;
                    m_dragCurrentY = m_dragStartY;
                } 
                else if (isLeftMouseButtonDown && m_isDragging) {
                    double mouseX, mouseY;
                    glfwGetCursorPos(m_window, &mouseX, &mouseY);
                    converterTelaParaMundo(mouseX, mouseY, m_dragCurrentX, m_dragCurrentY);
                    calcularPrevisaoTrajetoria();
                } 
                else if (!isLeftMouseButtonDown && m_wasLeftMouseButtonDown && m_isDragging) {
                    m_isDragging = false;
                    float slingshotMultiplier = 0.05f;
                    float finalVx = (m_dragStartX - m_dragCurrentX) * slingshotMultiplier;
                    float finalVy = (m_dragStartY - m_dragCurrentY) * slingshotMultiplier;
                    
                    m_sistemaSolar->addBody(type, m_dragStartX, m_dragStartY, finalVx, finalVy, m_spawnMass, m_spawnColor[0], m_spawnColor[1], m_spawnColor[2]);
                }
            }
        } 
    } else {
        m_isDragging = false; 
    }

    m_wasLeftMouseButtonDown = isLeftMouseButtonDown;
    m_wasRightMouseButtonDown = isRightMouseButtonDown;
}

// ==========================================
// Interface do Usuário (ImGui)
// ==========================================
void Aplicacao::desenharInterfaceUsuario() {
    ImGui::Begin("Controles do Sistema Solar");
    
    ImGui::Text("Camera Settings:");
    if (ImGui::Button("Reset Camera")) {
        m_cameraX = 0.0f;
        m_cameraY = 0.0f;
        m_cameraZoom = 1.0f;
    }
    ImGui::Text("Zoom: %.2fx", m_cameraZoom);
    ImGui::Text("Tip: Use Scroll Wheel to Zoom, Right-Click to Pan");
    ImGui::Separator();

    ImGui::Text("Global Physics Settings:");
    ImGui::SliderFloat("Gravitational Force (G)", &m_sistemaSolar->gravityMultiplier, 0.1f, 10.0f);
    ImGui::SliderFloat("Planet Visual Scale", &m_planetVisualScale, 1.0f, 15.0f);
    
    ImGui::Checkbox("Enable N-Body Gravity", &m_sistemaSolar->enableNBody);
    ImGui::Checkbox("Enable Collisions", &m_sistemaSolar->enableCollisions);
    ImGui::Checkbox("Contain Planets", &m_sistemaSolar->containPlanets);
    
    if (ImGui::Button("Reset System")) {
        delete m_sistemaSolar;
        m_sistemaSolar = new SolarSystem(m_screenWidth, m_screenHeight);
        m_cameraX = 0.0f; 
        m_cameraY = 0.0f;
        m_cameraZoom = 1.0f;
    }

    ImGui::Separator();
    
    ImGui::Text("Sandbox: Body Spawner");
    ImGui::Checkbox("Enable Placement Mode", &m_isPlacementMode);
    
    if (m_isPlacementMode) {
        ImGui::Indent();
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Left-Click & Drag to spawn!");
        
        
        const char* bodyTypes[] = { "Star", "Planet", "Asteroid", "Black Hole" };
        if (ImGui::Combo("Body Type", &m_selectedBodyType, bodyTypes, IM_ARRAYSIZE(bodyTypes))) {

            switch (m_selectedBodyType) {
            case 0: // Star
                m_spawnMass = 500.0f;
                m_spawnColor[0] = 1.0f; m_spawnColor[1] = 0.8f; m_spawnColor[2] = 0.0f;
                break;
            case 1: // Planet
                m_spawnMass = 5.0f;
                m_spawnColor[0] = 0.5f; m_spawnColor[1] = 0.5f; m_spawnColor[2] = 0.5f;
                break;
            case 2: // Asteroid
                m_spawnMass = 0.5f;
                m_spawnColor[0] = 0.6f; m_spawnColor[1] = 0.6f; m_spawnColor[2] = 0.6f;
                break;
            case 3: // Black Hole
                m_spawnMass = 2000.0f; // Muito pesado!
                m_spawnColor[0] = 0.0f; m_spawnColor[1] = 0.0f; m_spawnColor[2] = 0.0f;
                break;
            }
        }

        ImGui::SliderFloat("New Mass", &m_spawnMass, 0.1f, 1000.0f);
        
        // Botão ajudante de cores
        if (ImGui::Button("Reset Color for Type")) {
            if (m_selectedBodyType == 0) { m_spawnColor[0] = 1.0f; m_spawnColor[1] = 0.8f; m_spawnColor[2] = 0.0f; } // Estrela
            else if (m_selectedBodyType == 1) { m_spawnColor[0] = 0.5f; m_spawnColor[1] = 0.5f; m_spawnColor[2] = 0.5f; } // Planeta
            else if (m_selectedBodyType == 2) { m_spawnColor[0] = 0.6f; m_spawnColor[1] = 0.6f; m_spawnColor[2] = 0.6f; } // Asteroide
            else if (m_selectedBodyType == 3) { m_spawnColor[0] = 0.0f; m_spawnColor[1] = 0.0f; m_spawnColor[2] = 0.0f; } // Buraco Negro
        }

        ImGui::Checkbox("Auto-Calculate Stable Orbit", &m_autoOrbit);
        ImGui::ColorEdit3("Body Color", m_spawnColor);
        ImGui::Unindent();
    }

    ImGui::Separator();
    ImGuiIO& io = ImGui::GetIO();
    ImGui::Text("Performance: %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
    ImGui::End();
}

// ==========================================
// Loop Principal
// ==========================================
void Aplicacao::executarLoop() {
    while (!glfwWindowShouldClose(m_window)) {
        
        ImGui_ImplOpenGL2_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        processarEntrada();
        m_sistemaSolar->onUpdate();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        int width, height;
        glfwGetFramebufferSize(m_window, &width, &height);
        if (height == 0) height = 1;
        glViewport(0, 0, width, height);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0, width, height, 0, -1, 1);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glTranslatef(m_screenWidth / 2.0f, m_screenHeight / 2.0f, 0.0f);
        glScalef(m_cameraZoom, m_cameraZoom, 1.0f);
        glTranslatef(-m_screenWidth / 2.0f, -m_screenHeight / 2.0f, 0.0f);
        glTranslatef(m_cameraX, m_cameraY, 0.0f);

        // Chama o Renderizador Puro
        Renderizador::renderizarSistema(*m_sistemaSolar, m_planetVisualScale);

        if (m_isDragging && !m_autoOrbit) {
            glBegin(GL_LINES);
            glColor3f(m_spawnColor[0], m_spawnColor[1], m_spawnColor[2]);
            glVertex2f(m_dragStartX, m_dragStartY);
            glColor3f(1.0f, 0.0f, 0.0f); 
            glVertex2f(m_dragCurrentX, m_dragCurrentY);
            glEnd();
        }
        if (m_isDragging && !m_predictedPath.empty()) {
            glBegin(GL_LINE_STRIP);
            glColor4f(1.0f, 1.0f, 1.0f, 0.5f); // Branco, 50% de transparência
            for (const auto& p : m_predictedPath) {
                glVertex2f(p.x, p.y);
            }
            glEnd();
        }

        desenharInterfaceUsuario();
        ImGui::Render();
        ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(m_window);
        glfwPollEvents();
    }
}

void Aplicacao::calcularPrevisaoTrajetoria() {
    m_predictedPath.clear();

    // 1. Pega os dados básicos
    float sunX = m_sistemaSolar->getDominantGravityCenter().x;
    float sunY = m_sistemaSolar->getDominantGravityCenter().y;
    float sunMass = m_sistemaSolar->getDominantMass();
    float G = m_sistemaSolar->gravityMultiplier;

    // 2. Define a velocidade inicial baseada no estilingue
    float slingshotMultiplier = 0.05f;
    Vector2 pos(m_dragStartX, m_dragStartY);
    Vector2 vel((m_dragStartX - m_dragCurrentX) * slingshotMultiplier, 
                (m_dragStartY - m_dragCurrentY) * slingshotMultiplier);

    // 3. Simula 200 passos no futuro
    for (int i = 0; i < 200; i++) {
        float dx = sunX - pos.x;
        float dy = sunY - pos.y;
        float distSq = dx*dx + dy*dy;
        if (distSq < 100.0f) distSq = 100.0f; // Softening

        // Aceleração = (G * M) / r^2
        float acc = (G * sunMass) / distSq;
        
        // Aplica direção da aceleração
        float dist = std::sqrt(distSq);
        vel.x += (dx / dist) * acc;
        vel.y += (dy / dist) * acc;

        // Atualiza posição
        pos.x += vel.x;
        pos.y += vel.y;

        m_predictedPath.push_back(pos);
    }
}