#include "shell/aplicacao.h"

int main(void) {
    // 1. Instancia a nossa classe principal da Casca Imperativa (Shell)
    Aplicacao app(1280, 720, "Sistema Solar - CGPI");

    // 2. Tenta inicializar a janela, o OpenGL e o ImGui
    if (!app.inicializar()) {
        return -1; // Se a placa de vídeo ou a janela falharem, encerra com erro
    }

    // 3. Se tudo deu certo, roda o loop infinito da simulação
    app.executarLoop();

    return 0; // Quando o loop terminar (usuário fechar a janela), sai com sucesso
}