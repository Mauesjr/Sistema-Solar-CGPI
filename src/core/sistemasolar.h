#pragma once
#ifndef SOLARSYSTEM_HPP
#define SOLARSYSTEM_HPP

#include <vector>
#include <deque>

/**
 * @brief Estrutura matemática bidimensional para manipulação de vetores espaciais e físicos.
 * * Contém funções utilitárias para adição, subtração e cálculo de magnitude.
 */
struct Vector2 {
    float x; ///< Coordenada X do vetor
    float y; ///< Coordenada Y do vetor

    Vector2(float _x = 0, float _y = 0);

    void add(const Vector2& v);
    void sub(const Vector2& v);
    void mult(float n);
    void div(float n);
    float magSq() const;
    float mag() const;
    void setMag(float n);

    static Vector2 sub(const Vector2& v1, const Vector2& v2);
};


/**
 * @brief Categorias de corpos celestes disponíveis na simulação.
 * Influenciam na densidade (cálculo de raio) e regras de colisão.
 */
enum class BodyType {
    STAR,       ///< Estrela (Densidade padrão, atua como fonte de gravidade)
    PLANET,     ///< Planeta (Densidade padrão)
    ASTEROID,   ///< Asteroide (Alta densidade, rochoso)
    BLACK_HOLE  ///< Buraco Negro (Singularidade, densidade extrema, absorve tudo)
};


/**
 * @brief Classe unificada que representa qualquer corpo celeste no sistema.
 * Armazena dados físicos como posição, velocidade, aceleração, massa e o 
 * tipo físico do corpo.
 */
class CelestialBody {
public:
    BodyType type; ///< Tipo físico do corpo celeste
    Vector2 pos;   ///< Posição atual do corpo no espaço 2D
    Vector2 vel;   ///< Velocidade atual do corpo
    Vector2 acc;   ///< Aceleração atual do corpo
    float mass;    ///< Massa do corpo
    float r;       ///< Raio físico do corpo (calculado pela massa e tipo)
    
    float r_col;   ///< Componente de cor Vermelha (Red) para o renderizador
    float g_col;   ///< Componente de cor Verde (Green) para o renderizador
    float b_col;   ///< Componente de cor Azul (Blue) para o renderizador
    
    bool isDead;   ///< Flag que indica se o corpo colidiu e deve ser removido da simulação

    std::deque<Vector2> path; ///< Fila que armazena as últimas posições para gerar o rastro
    size_t maxPathLength;     ///< Tamanho máximo do rastro antes de apagar posições antigas

    /**
     * @brief Instancia um novo corpo celeste.
     * @param t Tipo do corpo (Estrela, Planeta, Asteroide ou Buraco Negro).
     * @param x Posição inicial no eixo X.
     * @param y Posição inicial no eixo Y.
     * @param vx Velocidade inicial no eixo X.
     * @param vy Velocidade inicial no eixo Y.
     * @param m Massa do corpo.
     * @param r_c Cor vermelha (0.0 a 1.0).
     * @param g_c Cor verde (0.0 a 1.0).
     * @param b_c Cor azul (0.0 a 1.0).
     */
    CelestialBody(BodyType t, float x, float y, float vx, float vy, float m, float r_c, float g_c, float b_c);

    /**
     * @brief Aplica uma força externa ao corpo (Segunda Lei de Newton).
     * @param force O vetor de força a ser aplicado.
     */
    void applyForce(Vector2 force);

    /**
     * @brief Atualiza a física do corpo (Integração de Euler).
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
 * @brief Gerenciador central da física do Sistema Solar.
 * * Este é o "Functional Core" puro da aplicação. Ele armazena a lista única 
 * de corpos celestes e calcula as interações físicas a cada frame.
 */
class SolarSystem {
private:
    float m_screenWidth;  ///< Largura da área de simulação
    float m_screenHeight; ///< Altura da área de simulação

public:
    std::vector<CelestialBody> bodies; ///< Lista contendo todos os corpos ativos na simulação

    float gravityMultiplier; ///< Força gravitacional global (G)
    bool enableNBody;        ///< Flag para ativar interações gravitacionais N-Body
    bool containPlanets;     ///< Flag para manter os planetas rebatendo nas bordas da tela
    bool enableCollisions;   ///< Flag para ativar colisões inelásticas (fusão de massa)

    /**
     * @brief Inicializa a simulação matemática do Sistema Solar.
     * @param screenWidth Largura lógica da tela.
     * @param screenHeight Altura lógica da tela.
     */
    SolarSystem(int screenWidth, int screenHeight);

    /**
     * @brief Avança o estado físico da simulação em um "tick" de tempo.
     * Calcula as atrações gravitacionais, o horizonte de eventos de buracos negros,
     * as colisões inelásticas e atualiza a cinemática final.
     */
    void onUpdate();
    
    /**
     * @brief Cria um novo corpo celeste e o adiciona à simulação.
     * @param type A categoria do objeto.
     * @param x Coordenada X inicial.
     * @param y Coordenada Y inicial.
     * @param vx Velocidade inicial no eixo X.
     * @param vy Velocidade inicial no eixo Y.
     * @param m Massa do novo objeto.
     * @param r_c Cor vermelha (Visual).
     * @param g_c Cor verde (Visual).
     * @param b_c Cor azul (Visual).
     */
    void addBody(BodyType type, float x, float y, float vx, float vy, float m, float r_c, float g_c, float b_c);
    
    /**
     * @brief Procura o corpo com a maior massa para atuar como âncora orbital.
     * Usado pela interface gráfica para calcular a órbita estável (auto-orbit) do estilingue.
     * @return Vetor 2D com as coordenadas do corpo mais massivo.
     */
    Vector2 getDominantGravityCenter() const;

    /**
     * @brief Retorna o valor de massa do corpo mais massivo do sistema.
     * Usado para a fórmula de velocidade orbital na hora do lançamento.
     * @return O valor da massa em float.
     */
    float getDominantMass() const;
};

#endif // SOLARSYSTEM_HPP