# Apaga a pasta build se ela já existir
if (Test-Path build) { Remove-Item -Recurse -Force build }

# Configura o CMake (forçando o uso do seu g++ do MSYS2)
cmake -S . -B build -G "MinGW Makefiles"

# Compila o projeto
cmake --build build

# Executa o programa compilado
.\build\SolarSystem.exe