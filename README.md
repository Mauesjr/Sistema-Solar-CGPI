# Simulador de Sistema Solar N-Body

Bem-vindo à documentação oficial do **Simulador de Sistema Solar**. 
Este projeto é uma simulação física interativa baseada no Problema dos N-Corpos (N-Body Problem), desenvolvida inteiramente em C++ moderno e renderizada via OpenGL.

---

## Visão Geral do Projeto

O objetivo deste software é criar um ambiente "Sandbox" educacional e interativo. Através dele, é possível observar as leis da gravitação universal em tempo real, inserir novos astros e visualizar simulações de mecânica orbital e horizontes de eventos.

### Principais Funcionalidades
* **Motor Físico N-Body:** Integração de Euler semi-implícita com conservação de energia.
* **Previsão de Trajetória:** Cálculo de órbitas futuras utilizando um micro-universo de simulação em tempo real.
* **Buracos Negros e Colisões:** Lógica de fusão inelástica de massas e absorção por horizonte de eventos.
* **Interface Dinâmica:** Painel de controle em tempo real utilizando a biblioteca Dear ImGui.

---

## Arquitetura de Software

O projeto adota a arquitetura **FCIS (Functional Core, Imperative Shell)** para garantir estabilidade e código limpo:

1. **Functional Core (O Núcleo Físico):** Classes estritamente matemáticas (`SolarSystem`, `CelestialBody`). Isoladas e determinísticas.
2. **Imperative Shell (A Casca de Controle):** Classes responsáveis por capturar inputs, UI e gráficos (`Aplicacao`, `Renderizador`).

---

## Como Compilar e Rodar

Este projeto utiliza scripts em PowerShell para facilitar a compilação via MSYS2/MinGW64.

1. Clone ou baixe o repositório.
2. Abra o terminal na raiz do projeto.
3. Execute o script de build:
   ```powershell
   .\build.ps1