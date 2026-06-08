cmake_minimum_required(VERSION 3.10)
project(SolarSystem)

set(CMAKE_CXX_STANDARD 17)

# Encontrar dependências do sistema
find_package(PkgConfig REQUIRED)
pkg_check_modules(GLFW REQUIRED glfw3)
find_package(OpenGL REQUIRED)

# Listar arquivos do ImGui
set(IMGUI_SOURCES
    src/imgui/imgui.cpp
    src/imgui/imgui_demo.cpp
    src/imgui/imgui_draw.cpp
    src/imgui/imgui_tables.cpp
    src/imgui/imgui_widgets.cpp
    src/imgui/imgui_impl_glfw.cpp
    src/imgui/imgui_impl_opengl2.cpp
)

# Criar o executável unindo seu código, o núcleo FCIS, a casca e o ImGui
add_executable(SolarSystem 
    src/main.cpp 
    src/core/sistemasolar.cpp
    src/shell/renderizador.cpp
    src/shell/aplicacao.cpp 
    ${IMGUI_SOURCES}
)

# Incluir os diretórios dos headers
target_include_directories(SolarSystem PRIVATE 
    src/imgui
    src  # Adicionado para facilitar as importações relativas
    ${GLFW_INCLUDE_DIRS}
    ${OPENGL_INCLUDE_DIR}
)

# Linkar as bibliotecas necessárias
target_link_libraries(SolarSystem 
    ${GLFW_LIBRARIES}
    ${OPENGL_LIBRARIES}
)


# Antigo CMakeLists.txt (sem a estrutura FCIS e sem as novas funcionalidades)


# cmake_minimum_required(VERSION 3.10)
# project(SolarSystem)

# set(CMAKE_CXX_STANDARD 17)

# # Encontrar dependências do sistema
# find_package(PkgConfig REQUIRED)
# pkg_check_modules(GLFW REQUIRED glfw3)
# find_package(OpenGL REQUIRED)

# # Listar arquivos do ImGui
# set(IMGUI_SOURCES
#     src/imgui/imgui.cpp
#     src/imgui/imgui_demo.cpp
#     src/imgui/imgui_draw.cpp
#     src/imgui/imgui_tables.cpp
#     src/imgui/imgui_widgets.cpp
#     src/imgui/imgui_impl_glfw.cpp
#     src/imgui/imgui_impl_opengl2.cpp
# )

# # Criar o executável unindo seu código e o ImGui
# add_executable(SolarSystem 
#     main.cpp 
#     ${IMGUI_SOURCES}
# )

# # Incluir os diretórios dos headers
# target_include_directories(SolarSystem PRIVATE 
#     src/imgui
#     ${GLFW_INCLUDE_DIRS}
#     ${OPENGL_INCLUDE_DIR}
# )

# # Linkar as bibliotecas necessárias
# target_link_libraries(SolarSystem 
#     ${GLFW_LIBRARIES}
#     ${OPENGL_LIBRARIES}
# )
