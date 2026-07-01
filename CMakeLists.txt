cmake_minimum_required(VERSION 3.10)
project(FluidSimulator3D)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find system dependencies
find_package(PkgConfig REQUIRED)
pkg_check_modules(GLFW REQUIRED glfw3)
find_package(OpenGL REQUIRED)

# List ImGui files (Updated to OpenGL 3 backend)
set(IMGUI_SOURCES
    src/imgui/imgui.cpp
    src/imgui/imgui_demo.cpp
    src/imgui/imgui_draw.cpp
    src/imgui/imgui_tables.cpp
    src/imgui/imgui_widgets.cpp
    src/imgui/imgui_impl_glfw.cpp
    src/imgui/imgui_impl_opengl3.cpp
)

# Core and Shell architecture sources
set(APP_SOURCES
    src/core/fluidsimulator.cpp
    src/shell/renderizador.cpp
    src/shell/aplicacao.cpp
    src/shell/Shader.cpp
    src/shell/Camera.cpp
    src/shell/RenderSystem.cpp
)

# Create the executable (including glad.c for modern OpenGL loading)
# Assuming glad.c is placed in src/
add_executable(${PROJECT_NAME} 
    src/main.cpp 
    src/glad.c
    ${APP_SOURCES}
    ${IMGUI_SOURCES}
)

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE 
    src/imgui
    src
    ${GLFW_INCLUDE_DIRS}
    ${OPENGL_INCLUDE_DIR}
)

# Link necessary libraries
target_link_libraries(${PROJECT_NAME} 
    ${GLFW_LIBRARIES}
    ${OPENGL_LIBRARIES}
    dl # Required for GLAD on Linux
)