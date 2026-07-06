# Apaga a pasta build se ela já existir
if (Test-Path build) { Remove-Item -Recurse -Force build }

# Configura o CMake (forçando o uso do seu g++ do MSYS2)
cmake -S . -B build -G "MinGW Makefiles"
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

# Compila o projeto
cmake --build build
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

# Descobre o nome do executável gerado
$exe = Get-ChildItem -Path "build" -Filter "*.exe" | Select-Object -First 1
if ($exe) {
    & $exe.FullName
} else {
    Write-Host "Nenhum executável encontrado em build/"
}