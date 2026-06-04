#pragma once
#ifndef SOLARSYSTEM_HPP
#define SOLARSYSTEM_HPP

#include <vector>
#include <deque>
#include <algorithm>

/**
 * @brief Estrutura matemática bidimensional para manipulação de vetores espaciais e físicos.
 * * Contém funções utilitárias para adição, subtração e cálculo de magnitude,
 * essenciais para o motor de física do sistema solar.
 */
struct Vector2 {
    float x; ///< Coordenada X do vetor
    float y; ///< Coordenada Y do vetor

    /**
     * @brief Construtor padrão do vetor.
     * @param _x Valor inicial de X (padrão é 0).
     * @param _y Valor inicial de Y (padrão é 0).
     */
    Vector2(float _x = 0, float _y = 0);

    /**
     * @brief Adiciona um vetor a este vetor (Modifica o vetor atual).
     * @param v O vetor a ser adicionado.
     */
    void add(const Vector2& v);

    /**
     * @brief Subtrai um vetor deste vetor (Modifica o vetor atual).
     * @param v O vetor a ser subtraído.
     */
    void sub(const Vector2& v);

    /**
     * @brief Multiplica o vetor por um escalar.
     * @param n O valor multiplicador.
     */
    void mult(float n);

    /**
     * @brief Divide o vetor por um escalar.
     * @param n O valor divisor.
     */
    void div(float n);

    /**
     * @brief Calcula a magnitude (comprimento) ao quadrado do vetor.
     * @return O valor da magnitude ao quadrado (evita o custo computacional da raiz quadrada).
     */
    float magSq() const;

    /**
     * @brief Calcula a magnitude (comprimento) exata do vetor.
     * @return O valor da magnitude.
     */
    float mag() const;

    /**
     * @brief Define a magnitude do vetor, preservando sua direção.
     * @param n A nova magnitude desejada.
     */
    void setMag(float n);

    /**
     * @brief Subtrai dois vetores e retorna o resultado como um novo vetor.
     * @param v1 Vetor de origem.
     * @param v2 Vetor a ser subtraído.
     * @return Um novo objeto Vector2 contendo o resultado da subtração.
     */
    static Vector2 sub(const Vector2& v1, const Vector2& v2);
};

/**
 * @brief Classe que representa um corpo celeste (planeta) no sistema.
 * * Armazena dados físicos como posição, velocidade, aceleração e massa,
 * além do rastro (path) gerado pelo movimento. Nenhuma lógica de renderização
 * gráfica (OpenGL) ocorre aqui.
 */
class Mover {
public:
    Vector2 pos; ///< Posição atual do corpo no espaço 2D
    Vector2 vel; ///< Velocidade atual do corpo
    Vector2 acc; ///< Aceleração atual do corpo
    float mass;  ///< Massa do corpo
    float r;     ///< Raio físico do corpo (calculado matematicamente com base na massa)
    
    float r_col; ///< Componente de cor Vermelha (Red) para o renderizador
    float g_col; ///< Componente de cor Verde (Green) para o renderizador
    float b_col; ///< Componente de cor Azul (Blue) para o renderizador
    
    bool isDead; ///< Flag que indica se o corpo colidiu e deve ser removido da simulação

    std::deque<Vector2> path; ///< Fila que armazena as últimas posições para gerar o rastro
    size_t maxPathLength;     ///< Tamanho máximo do rastro antes de começar a apagar as posições antigas

    /**
     * @brief Instancia um novo corpo celeste.
     * @param x Posição inicial no eixo X.
     * @param y Posição inicial no eixo Y.
     * @param vx Velocidade inicial no eixo X.
     * @param vy Velocidade inicial no eixo Y.
     * @param m Massa do corpo.
     * @param r_c Cor vermelha (0.0 a 1.0).
     * @param g_c Cor verde (0.0 a 1.0).
     * @param b_c Cor azul (0.0 a 1.0).
     */
    Mover(float x, float y, float vx, float vy, float m, float r_c, float g_c, float b_c);

    /**
     * @brief Aplica uma força externa ao corpo.
     * Aceleração é alterada baseada na Segunda Lei de Newton (Aceleração = Força / Massa).
     * @param force O vetor de força a ser aplicado.
     */
    void applyForce(Vector2 force);

    /**
     * @brief Atualiza a física do corpo (Integração de Euler).
     * Soma a aceleração na velocidade, a velocidade na posição, e zera a aceleração.
     * Também registra a posição atual no rastro.
     */
    void update();

    /**
     * @brief Verifica colisão com as bordas da tela e inverte a velocidade se necessário.
     * @param screenWidth A largura do espaço da simulação.
     * @param screenHeight A altura do espaço da simulação.
     */
    void checkEdges(float screenWidth, float screenHeight);
};

/**
 * @brief Classe que representa o grande atrator central (o Sol) do sistema.
 * Responsável por exercer atração gravitacional sobre os objetos do tipo Mover.
 */
class Attractor {
public:
    Vector2 pos; ///< Posição do atrator no espaço bidimensional
    float mass;  ///< Massa do atrator (influencia a força da gravidade)
    float r;     ///< Raio físico do atrator

    /**
     * @brief Construtor do Atrator central.
     * @param x Posição no eixo X.
     * @param y Posição no eixo Y.
     * @param m Massa inicial do atrator.
     */
    Attractor(float x, float y, float m);

    /**
     * @brief Aplica a força de atração gravitacional sobre um corpo celeste.
     * Calcula a força baseada na Lei da Gravitação Universal de Newton, 
     * limitando as distâncias extremas para evitar anomalias matemáticas (como divisão por zero).
     * @param mover Referência para o corpo celeste que sofrerá a atração.
     * @param currentGravity O multiplicador global de gravidade definido na simulação.
     */
    void attract(Mover& mover, float currentGravity);
};

/**
 * @brief Gerenciador central da física do Sistema Solar.
 * * Este é o "Functional Core" puro da aplicação. Ele armazena o estado completo 
 * de todos os corpos celestes e calcula as interações físicas a cada frame.
 * Não possui nenhum acoplamento com bibliotecas de renderização gráfica.
 */
class SolarSystem {
private:
    float m_screenWidth;  ///< Largura da área de simulação
    float m_screenHeight; ///< Altura da área de simulação

public:
    std::vector<Mover> movers; ///< Lista contendo todos os planetas ativos na simulação
    Attractor* attractor;      ///< Ponteiro para o Sol (Atrator central)

    float gravityMultiplier;   ///< Força gravitacional global (G)
    float sunMass;             ///< Massa atual do Sol
    bool enableNBody;          ///< Flag para ativar interações gravitacionais entre os planetas
    bool containPlanets;       ///< Flag para manter os planetas rebatendo nas bordas da tela
    bool enableCollisions;     ///< Flag para ativar colisões inelásticas (fusão de massa)

    /**
     * @brief Inicializa a simulação matemática do Sistema Solar.
     * Cria o Sol no centro da tela e injeta os planetas iniciais com suas posições, 
     * velocidades e massas relativas.
     * @param screenWidth Largura lógica da tela.
     * @param screenHeight Altura lógica da tela.
     */
    SolarSystem(int screenWidth, int screenHeight);

    /**
     * @brief Destrutor padrão para limpar os dados alocados (Atrator).
     */
    ~SolarSystem();

    /**
     * @brief Avança o estado físico da simulação em um "tick" de tempo.
     * Ordem de execução:
     * 1. Atualiza massa/raio do Sol.
     * 2. Calcula atração do Sol sobre os planetas.
     * 3. Calcula atração N-Body (planetas atraindo planetas), se ativado.
     * 4. Calcula colisões inelásticas e fusão de massas, se ativado.
     * 5. Atualiza a cinemática (posição/velocidade) final de todos os corpos.
     */
    void onUpdate();

    /**
     * @brief Move o Sol para uma nova coordenada (usado pela interação do usuário).
     * @param x Nova coordenada X.
     * @param y Nova coordenada Y.
     */
    void setSunPosition(float x, float y);

    /**
     * @brief Cria um novo planeta e o adiciona à simulação.
     * @param x Coordenada X inicial.
     * @param y Coordenada Y inicial.
     * @param vx Velocidade inicial no eixo X.
     * @param vy Velocidade inicial no eixo Y.
     * @param m Massa do novo planeta.
     * @param r_c Cor vermelha (Visual).
     * @param g_c Cor verde (Visual).
     * @param b_c Cor azul (Visual).
     */
    void addBody(float x, float y, float vx, float vy, float m, float r_c, float g_c, float b_c);
    
    /**
     * @brief Recupera a posição X atual do Sol.
     * @return Posição X em float.
     */
    float getSunPosX() const;

    /**
     * @brief Recupera a posição Y atual do Sol.
     * @return Posição Y em float.
     */
    float getSunPosY() const;
};

#endif