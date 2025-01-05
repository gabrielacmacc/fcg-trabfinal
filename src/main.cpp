//     Universidade Federal do Rio Grande do Sul
//             Instituto de Informática
//       Departamento de Informática Aplicada
//
//    INF01047 Fundamentos de Computação Gráfica
//               Prof. Eduardo Gastal
//
//

// "headers" padrões de C
#include <cmath>
#include <cstdio>
#include <cstdlib>

// Headers específicos de C++
#include <map>
#include <stack>
#include <string>
#include <vector>
#include <limits>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <iomanip>

// Headers das bibliotecas OpenGL
#include <external/glad/glad.h>  // Criação de contexto OpenGL 3.3
#include <external/GLFW/glfw3.h> // Criação de janelas do sistema operacional

// Headers da biblioteca GLM: criação de matrizes e vetores.
#include <external/glm/mat4x4.hpp>
#include <external/glm/vec4.hpp>
#include <external/glm/gtc/type_ptr.hpp>

// Headers da biblioteca para carregar modelos obj
#include "external/tiny_obj_loader.h"

// Headers locais, definidos na pasta "include/"
#include "matrices.h"
#include "objects/objects.hpp"
#include "callbacks/callbacks.hpp"
#include "collisions/collisions.hpp"
#include "globals/globals.hpp"
#include "textrendering/textrendering.hpp"
#include "utils/utils.h"
#include "utils/shader_utils.hpp"
#include "utils/texture_utils.hpp"

#define SPHERE 0
#define LABYRINTH_1 1
#define LABYRINTH_2 2
#define LABYRINTH_3 3
#define PLANE 4
#define BACKGROUND 5
#define PACMAN 6
#define CHERRY 7
#define COUNT_1 8
#define COUNT_2 9
#define COUNT_3 10
#define GHOST 11

// Declaração de funções utilizadas para pilha de matrizes de modelagem.
void PushMatrix(glm::mat4 M);
void PopMatrix(glm::mat4 &M);

// Declaração de várias funções utilizadas em main().  Essas estão definidas
// logo após a definição de main() neste arquivo.
void BuildTrianglesAndAddToVirtualScene(ObjModel *); // Constrói representação de um ObjModel como malha de triângulos para renderização
void DrawVirtualObject(const char *object_name);     // Desenha um objeto armazenado em g_VirtualScene
void PrintObjModelInfo(ObjModel *);                  // Função para debugging
glm::vec4 calculateBezierPosition(glm::vec4 p1, glm::vec4 p2, glm::vec4 p3, glm::vec4 p4, float t);
void MovePacman(glm::vec4 camera_view_unit, glm::vec4 camera_side_view_unit, float ellapsedTime, std::vector<glm::vec4> collision_directions);

// Declaração da classe paredes
using namespace std;

class Cherry
{
public:
    // Atributos:
    glm::mat4 modelMatrix;
    int objectId;
    int objectType;
    std::string objectName;
    glm::vec3 center;
    Sphere cherry_sphere;

    // Métodos:

    // Construtor
    Cherry(glm::mat4 modelMatrix, int objectId, int objectType, std::string objectName, glm::vec3 center, float radius)
        : modelMatrix(modelMatrix), objectId(objectId), objectType(objectType), objectName(objectName)
    {
        this->center = center;
        this->cherry_sphere = Sphere{center, radius};
    }

    void render()
    {
        glUniformMatrix4fv(g_model_uniform, 1, GL_FALSE, glm::value_ptr(modelMatrix));
        glUniform1i(g_object_id_uniform, objectType);
        DrawVirtualObject(objectName.c_str());
    }
};

class Ghost
{
public:
    // Atributos:
    glm::mat4 modelMatrix;
    int objectId;
    int objectType;
    std::string objectName;
    AABB ghost_bbox;

    // Métodos:

    // Construtor
    Ghost(glm::mat4 modelMatrix, int objectId, int objectType, std::string objectName, std::map<std::string, SceneObject> &g_VirtualScene)
        : modelMatrix(modelMatrix), objectId(objectId), objectType(objectType), objectName(objectName)
    {
        this->ghost_bbox = setBoundingBox(g_VirtualScene);
    }

    void render()
    {
        glUniformMatrix4fv(g_model_uniform, 1, GL_FALSE, glm::value_ptr(modelMatrix));
        glUniform1i(g_object_id_uniform, objectType);
        DrawVirtualObject(objectName.c_str());
    }

private:
    AABB setBoundingBox(std::map<std::string, SceneObject> &g_VirtualScene)
    {
        glm::vec3 bbox_min = g_VirtualScene[objectName].bbox_min;
        glm::vec3 bbox_max = g_VirtualScene[objectName].bbox_max;
        glm::vec4 minCorner = glm::vec4(bbox_min.x, bbox_min.y, bbox_min.z, 1.0f);
        glm::vec4 maxCorner = glm::vec4(bbox_max.x, bbox_max.y, bbox_max.z, 1.0f);
        return {modelMatrix * minCorner, modelMatrix * maxCorner};
    }
};

class Wall
{
public:
    // Atributos:
    glm::mat4 modelMatrix;
    int objectId;
    int objectType;
    std::string objectName;
    AABB wall_bbox;

    // Métodos:

    // Construtor
    Wall(glm::mat4 modelMatrix, int objectId, int objectType, std::string objectName, std::map<std::string, SceneObject> &g_VirtualScene)
        : modelMatrix(modelMatrix), objectId(objectId), objectType(objectType), objectName(objectName)
    {
        this->wall_bbox = setBoundingBox(g_VirtualScene);
    }

    void render()
    {
        glUniformMatrix4fv(g_model_uniform, 1, GL_FALSE, glm::value_ptr(modelMatrix));
        glUniform1i(g_object_id_uniform, objectType);
        DrawVirtualObject(objectName.c_str());
        // printf("%d [(min: %f, max:%f), (min: %f , max: %f), (min: %f , max: %f)] ", objectId, minMaxCorner.min.x, minMaxCorner.max.x, minMaxCorner.min.y, minMaxCorner.max.y, minMaxCorner.min.z, minMaxCorner.max.z);
    }

private:
    AABB setBoundingBox(std::map<std::string, SceneObject> &g_VirtualScene)
    {
        glm::vec3 bbox_min = g_VirtualScene[objectName].bbox_min;
        glm::vec3 bbox_max = g_VirtualScene[objectName].bbox_max;
        glm::vec4 minCorner = glm::vec4(bbox_min.x, bbox_min.y, bbox_min.z, 1.0f);
        glm::vec4 maxCorner = glm::vec4(bbox_max.x, bbox_max.y, bbox_max.z, 1.0f);
        return {modelMatrix * minCorner, modelMatrix * maxCorner};
    }
};

class Ball
{
public:
    // Atributos:
    glm::mat4 modelMatrix;
    int objectType;
    std::string objectName;
    Sphere b_sphere;
    // bool shouldRender;

    // Construtor
    Ball(glm::vec3 center, float radius, int objectType, std::string objectName)
        : modelMatrix(modelMatrix), objectType(objectType), objectName(objectName)
    {
        this->modelMatrix = Matrix_Translate(center.x, center.y, center.z) * Matrix_Scale(radius, radius, radius);
        this->b_sphere = Sphere{center, radius};
    }
    // Métodos:
    void render()
    {
        glUniformMatrix4fv(g_model_uniform, 1, GL_FALSE, glm::value_ptr(modelMatrix));
        glUniform1i(g_object_id_uniform, objectType);
        DrawVirtualObject(objectName.c_str());
        // printf("%d [(min: %f, max:%f), (min: %f , max: %f), (min: %f , max: %f)] ", objectId, minMaxCorner.min.x, minMaxCorner.max.x, minMaxCorner.min.y, minMaxCorner.max.y, minMaxCorner.min.z, minMaxCorner.max.z);
    }
};

void TextRendering_ShowWallsAABBs(GLFWwindow *window, Wall walls[], size_t size);

std::vector<Ball> instanciateLittleBalls();
std::vector<Cherry> instanciateCherries();

void renderCount(int current_count, char *count_first_digit, char *count_second_digit, char *count_third_digit);

// Abaixo definimos variáveis globais utilizadas em várias funções do código.

// A cena virtual é uma lista de objetos nomeados, guardados em um dicionário
// (map).  Veja dentro da função BuildTrianglesAndAddToVirtualScene() como que são incluídos
// objetos dentro da variável g_VirtualScene, e veja na função main() como
// estes são acessados.
std::map<std::string, SceneObject> g_VirtualScene;

// Pilha que guardará as matrizes de modelagem.
std::stack<glm::mat4> g_MatrixStack;

int main(int argc, char *argv[])
{
    // Inicializamos a biblioteca GLFW, utilizada para criar uma janela do
    // sistema operacional, onde poderemos renderizar com OpenGL.
    int success = glfwInit();
    if (!success)
    {
        fprintf(stderr, "ERROR: glfwInit() failed.\n");
        std::exit(EXIT_FAILURE);
    }

    // Definimos o callback para impressão de erros da GLFW no terminal
    glfwSetErrorCallback(ErrorCallback);

    // Pedimos para utilizar OpenGL versão 3.3 (ou superior)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // Pedimos para utilizar o perfil "core", isto é, utilizaremos somente as
    // funções modernas de OpenGL.
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Criamos uma janela do sistema operacional, com 800 colunas e 600 linhas
    // de pixels, e com título "INF01047 ...".
    GLFWwindow *window;
    window = glfwCreateWindow(800, 600, "INF01047 - Giulia e Gabriela", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        fprintf(stderr, "ERROR: glfwCreateWindow() failed.\n");
        std::exit(EXIT_FAILURE);
    }

    // Definimos a função de callback que será chamada sempre que o usuário
    // pressionar alguma tecla do teclado ...
    glfwSetKeyCallback(window, KeyCallback);
    // ... ou clicar os botões do mouse ...
    glfwSetMouseButtonCallback(window, MouseButtonCallback);
    // ... ou movimentar o cursor do mouse em cima da janela ...
    glfwSetCursorPosCallback(window, CursorPosCallback);
    // ... ou rolar a "rodinha" do mouse.
    glfwSetScrollCallback(window, ScrollCallback);

    // Indicamos que as chamadas OpenGL deverão renderizar nesta janela
    glfwMakeContextCurrent(window);

    // Carregamento de todas funções definidas por OpenGL 3.3, utilizando a
    // biblioteca GLAD.
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    // Definimos a função de callback que será chamada sempre que a janela for
    // redimensionada, por consequência alterando o tamanho do "framebuffer"
    // (região de memória onde são armazenados os pixels da imagem).
    glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);
    FramebufferSizeCallback(window, 800, 600); // Forçamos a chamada do callback acima, para definir g_ScreenRatio.

    // Imprimimos no terminal informações sobre a GPU do sistema
    const GLubyte *vendor = glGetString(GL_VENDOR);
    const GLubyte *renderer = glGetString(GL_RENDERER);
    const GLubyte *glversion = glGetString(GL_VERSION);
    const GLubyte *glslversion = glGetString(GL_SHADING_LANGUAGE_VERSION);

    printf("GPU: %s, %s, OpenGL %s, GLSL %s\n", vendor, renderer, glversion, glslversion);

    // Carregamos os shaders de vértices e de fragmentos que serão utilizados
    LoadShadersFromFiles();
    LoadTexturesFromFiles();

    // Construímos a representação de objetos geométricos através de malhas de triângulos
    ObjModel spheremodel("../../resources/models/food/sphere.obj");
    ComputeNormals(&spheremodel);
    BuildTrianglesAndAddToVirtualScene(&spheremodel);

    ObjModel planemodel("../../resources/models/skybox/plane.obj");
    ComputeNormals(&planemodel);
    BuildTrianglesAndAddToVirtualScene(&planemodel);

    ObjModel cubemodel("../../resources/models/skybox/cube.obj");
    ComputeNormals(&cubemodel);
    BuildTrianglesAndAddToVirtualScene(&cubemodel);

    ObjModel piecetwo("../../resources/models/labyrinth/p2.obj");
    ComputeNormals(&piecetwo);
    BuildTrianglesAndAddToVirtualScene(&piecetwo);

    ObjModel piecetworotated("../../resources/models/labyrinth/p2-rotated.obj");
    ComputeNormals(&piecetworotated);
    BuildTrianglesAndAddToVirtualScene(&piecetworotated);

    ObjModel piecethree("../../resources/models/labyrinth/p3.obj");
    ComputeNormals(&piecethree);
    BuildTrianglesAndAddToVirtualScene(&piecethree);

    ObjModel piecethreerotated("../../resources/models/labyrinth/p3-rotated.obj");
    ComputeNormals(&piecethreerotated);
    BuildTrianglesAndAddToVirtualScene(&piecethreerotated);

    ObjModel pacmodel("../../resources/models/pacman/pacman.obj");
    ComputeNormals(&pacmodel);
    BuildTrianglesAndAddToVirtualScene(&pacmodel);

    ObjModel ghostmodel("../../resources/models/ghost/ghost.obj");
    ComputeNormals(&ghostmodel);
    BuildTrianglesAndAddToVirtualScene(&ghostmodel);

    ObjModel cherrymodel("../../resources/models/food/cherry.obj");
    ComputeNormals(&cherrymodel);
    BuildTrianglesAndAddToVirtualScene(&cherrymodel);

    ObjModel zeromodel("../../resources/models/numbers/000.obj");
    ComputeNormals(&zeromodel);
    BuildTrianglesAndAddToVirtualScene(&zeromodel);

    ObjModel onemodel("../../resources/models/numbers/001.obj");
    ComputeNormals(&onemodel);
    BuildTrianglesAndAddToVirtualScene(&onemodel);

    ObjModel twomodel("../../resources/models/numbers/002.obj");
    ComputeNormals(&twomodel);
    BuildTrianglesAndAddToVirtualScene(&twomodel);

    ObjModel threemodel("../../resources/models/numbers/003.obj");
    ComputeNormals(&threemodel);
    BuildTrianglesAndAddToVirtualScene(&threemodel);

    ObjModel forthmodel("../../resources/models/numbers/004.obj");
    ComputeNormals(&forthmodel);
    BuildTrianglesAndAddToVirtualScene(&forthmodel);

    ObjModel fivemodel("../../resources/models/numbers/005.obj");
    ComputeNormals(&fivemodel);
    BuildTrianglesAndAddToVirtualScene(&fivemodel);

    ObjModel sixmodel("../../resources/models/numbers/006.obj");
    ComputeNormals(&sixmodel);
    BuildTrianglesAndAddToVirtualScene(&sixmodel);

    ObjModel sevenmodel("../../resources/models/numbers/007.obj");
    ComputeNormals(&sevenmodel);
    BuildTrianglesAndAddToVirtualScene(&sevenmodel);

    ObjModel eightmodel("../../resources/models/numbers/008.obj");
    ComputeNormals(&eightmodel);
    BuildTrianglesAndAddToVirtualScene(&eightmodel);

    ObjModel ninemodel("../../resources/models/numbers/009.obj");
    ComputeNormals(&ninemodel);
    BuildTrianglesAndAddToVirtualScene(&ninemodel);

    if (argc > 1)
    {
        ObjModel model(argv[1]);
        BuildTrianglesAndAddToVirtualScene(&model);
    }

    // Inicializamos o código para renderização de texto.
    TextRendering_Init();

    // Habilitamos o Z-buffer. Veja slides 104-116 do documento Aula_09_Projecoes.pdf.
    glEnable(GL_DEPTH_TEST);

    // Habilitamos o Backface Culling. Veja slides 8-13 do documento Aula_02_Fundamentos_Matematicos.pdf, slides 23-34 do documento Aula_13_Clipping_and_Culling.pdf e slides 112-123 do documento Aula_14_Laboratorio_3_Revisao.pdf.
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    // chama a função que inicializa as bolinhas:
    std::vector<Ball> balls = instanciateLittleBalls();
    std::vector<Cherry> cherries = instanciateCherries();

    int initial_ball_count = balls.size();
    int eaten_ball_count = 0;

    char count_first_digit[4] = {0};
    char count_second_digit[4] = {0};
    char count_third_digit[4] = {0};

    // Ficamos em um loop infinito, renderizando, até que o usuário feche a janela
    while (!glfwWindowShouldClose(window))
    {
        // Aqui executamos as operações de renderização

        // Definimos a cor do "fundo" do framebuffer como branco.  Tal cor é
        // definida como coeficientes RGBA: Red, Green, Blue, Alpha; isto é:
        // Vermelho, Verde, Azul, Alpha (valor de transparência).
        // Conversaremos sobre sistemas de cores nas aulas de Modelos de Iluminação.
        //
        //           R     G     B     A
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

        // "Pintamos" todos os pixels do framebuffer com a cor definida acima,
        // e também resetamos todos os pixels do Z-buffer (depth buffer).
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Pedimos para a GPU utilizar o programa de GPU criado acima (contendo
        // os shaders de vértice e fragmentos).
        glUseProgram(g_GpuProgramID);

        float currentTime = (float)glfwGetTime();
        float ellapsedTime = currentTime - previousTime;
        previousTime = currentTime;

        // Computamos a posição da câmera utilizando coordenadas esféricas.  As
        // variáveis g_CameraDistance, g_CameraPhi, e g_CameraTheta são
        // controladas pelo mouse do usuário. Veja as funções CursorPosCallback()
        // e ScrollCallback().
        if (t <= 1)
        {
            t += 0.008;
            curr_bezier_position = calculateBezierPosition(initial_position_bezier, intermediate_position_bezier_1, intermediate_position_bezier_2, final_position_bezier, t);
            pacman_position_c = curr_bezier_position;
        }
        // printf("curr_bezier_position: %f %f %f ", curr_bezier_position.x, curr_bezier_position.y, curr_bezier_position.z);

        float g_CameraPhiSin = sin(g_CameraPhi);
        float g_CameraPhiCos = cos(g_CameraPhi);
        float g_CameraThetaSin = sin(g_CameraTheta);
        float g_CameraThetaCos = cos(g_CameraTheta);

        float r = g_CameraDistance;
        float y = g_CameraPhiSin >= 0 ? r * g_CameraPhiSin : 0; // Limita ângulo da free camera
        float z = r * g_CameraPhiCos * g_CameraThetaCos;
        float x = r * g_CameraPhiCos * g_CameraThetaSin;

        // Abaixo definimos as varáveis que efetivamente definem a câmera virtual.
        // Veja slides 195-227 e 229-234 do documento Aula_08_Sistemas_de_Coordenadas.pdf.
        glm::vec4 camera_position_c = glm::vec4(x, y, z, 1.0f); // Ponto "c", centro da câmera
        // printf("camera_position_c: %f, %f, %f", camera_position_c.x, camera_position_c.y, camera_position_c.z);
        glm::vec4 camera_lookat_l = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);  // Ponto "l", para onde a câmera (look-at) estará sempre olhando
        glm::vec4 camera_view_vector;                                   // Vetor "view", sentido para onde a câmera está virada
        glm::vec4 camera_view_unit;                                     // Vetor "view" unitário
        glm::vec4 camera_up_vector = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f); // Vetor "up" fixado para apontar para o "céu" (eixo Y global)
        glm::vec4 camera_distance;
        // pacman_movement += pacman_offset;
        // pacman_position_c += pacman_movement;
        pacman_offset = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);

        if (isFreeCamOn)
        {
            camera_view_vector = glm::vec4(-x, -y, -z, 0.0f);
            camera_view_unit = camera_view_vector / norm(camera_view_vector);
            camera_distance = PACMAN_DISTANCE * camera_view_unit;
            camera_distance.y = camera_view_unit.y - 0.3f;
            camera_position_c = pacman_position_c - camera_distance;

            pacman_rotation = -atan2(camera_view_unit.z, camera_view_unit.x);
        }
        else
        {
            float fixedHeight = r;
            camera_position_c = glm::vec4(0.0f, fixedHeight, 0.0f, 1.0f);
            camera_up_vector = glm::vec4(0.0f, 0.0f, -1.0f, 0.0f);
            camera_view_vector = camera_lookat_l - camera_position_c;
        }

        printf("Pacman (%f, %f, %f) - ", pacman_position_c.x, pacman_position_c.y, pacman_position_c.z);
        // printf("Camera (%f, %f, %f) - ", camera_position_c.x, camera_position_c.y, camera_position_c.z);
        // printf("View Unit (%f, %f, %f) - ", camera_view_unit.x, camera_view_unit.y, camera_view_unit.z);
        // printf("Camera distance (%f, %f, %f) - ", camera_distance.x, camera_distance.y, camera_distance.z);
        // printf("\n");

        camera_view_unit = camera_view_vector / norm(camera_view_vector);
        glm::vec4 camera_side_view = crossproduct(camera_up_vector, camera_view_unit);
        glm::vec4 camera_side_view_unit = camera_side_view / norm(camera_side_view);
        glm::vec4 camera_up_unit = camera_up_vector / norm(camera_up_vector);

        glm::vec4 camera_v_view_unit = camera_view_unit;
        camera_v_view_unit.y = 0.0f;
        glm::vec4 vertical_move_unit = isFreeCamOn ? camera_v_view_unit : camera_up_unit;

        // Note que, no sistema de coordenadas da câmera, os planos near e far
        // estão no sentido negativo! Veja slides 176-204 do documento Aula_09_Projecoes.pdf.
        float nearplane = -0.1f; // Posição do "near plane"
        float farplane = -40.0f; // Posição do "far plane"

        int objectIdCounter = 0;

        Wall walls[] = {
            // Direito superior
            {Matrix_Translate(3.5f, -1.0f, -2.9f) * Matrix_Scale(0.6f, 0.5f, 0.2f), objectIdCounter++, LABYRINTH_2, "p22", g_VirtualScene},
            {Matrix_Translate(3.0f, -1.0f, -4.3f) * Matrix_Scale(0.4f, 0.5f, 0.2f), objectIdCounter++, LABYRINTH_2, "p22", g_VirtualScene},
            {Matrix_Translate(5.7f, -1.0f, -6.5f) * Matrix_Scale(0.2f, 0.5f, 0.4f), objectIdCounter++, LABYRINTH_2, "p2", g_VirtualScene},
            {Matrix_Translate(3.3f, -1.0f, -6.3f) * Matrix_Scale(0.2f, 0.5f, 0.25f), objectIdCounter++, LABYRINTH_2, "p2", g_VirtualScene},
            {Matrix_Translate(4.5f, -1.0f, -5.0f) * Matrix_Scale(0.2f, 0.5f, 0.25f), objectIdCounter++, LABYRINTH_2, "p2", g_VirtualScene},
            {Matrix_Translate(2.2f, -1.0f, -7.0f) * Matrix_Scale(0.3f, 0.5f, 0.2f), objectIdCounter++, LABYRINTH_2, "p22", g_VirtualScene},
            {Matrix_Translate(7.0f, -1.0f, -4.0f) * Matrix_Scale(0.2f, 0.5f, 0.3f), objectIdCounter++, LABYRINTH_2, "p2", g_VirtualScene},
            {Matrix_Translate(6.5f, -1.0f, -6.8f) * Matrix_Scale(0.2f, 0.5f, 0.2f), objectIdCounter++, LABYRINTH_2, "p22", g_VirtualScene},
            {Matrix_Translate(6.4f, -1.0f, -4.0f) * Matrix_Scale(0.1f, 0.5f, 0.2f), objectIdCounter++, LABYRINTH_2, "p22", g_VirtualScene},
            {Matrix_Translate(4.5f, -1.0f, -2.0f) * Matrix_Scale(0.2f, 0.5f, 0.2f), objectIdCounter++, LABYRINTH_2, "p2", g_VirtualScene},

            // Esquerdo superior
            {Matrix_Translate(-3.5f, -1.0f, -2.9f) * Matrix_Scale(0.6f, 0.5f, 0.2f), objectIdCounter++, LABYRINTH_2, "p22", g_VirtualScene},
            {Matrix_Translate(-3.0f, -1.0f, -4.3f) * Matrix_Scale(0.4f, 0.5f, 0.2f), objectIdCounter++, LABYRINTH_2, "p22", g_VirtualScene},
            {Matrix_Translate(-5.7f, -1.0f, -6.5f) * Matrix_Scale(0.2f, 0.5f, 0.4f), objectIdCounter++, LABYRINTH_2, "p2", g_VirtualScene},
            {Matrix_Translate(-3.3f, -1.0f, -6.3f) * Matrix_Scale(0.2f, 0.5f, 0.25f), objectIdCounter++, LABYRINTH_2, "p2", g_VirtualScene},
            {Matrix_Translate(-4.5f, -1.0f, -5.0f) * Matrix_Scale(0.2f, 0.5f, 0.25f), objectIdCounter++, LABYRINTH_2, "p2", g_VirtualScene},
            {Matrix_Translate(-2.2f, -1.0f, -7.0f) * Matrix_Scale(0.3f, 0.5f, 0.2f), objectIdCounter++, LABYRINTH_2, "p22", g_VirtualScene},
            {Matrix_Translate(-7.0f, -1.0f, -4.0f) * Matrix_Scale(0.2f, 0.5f, 0.3f), objectIdCounter++, LABYRINTH_2, "p2", g_VirtualScene},
            {Matrix_Translate(-6.5f, -1.0f, -6.8f) * Matrix_Scale(0.2f, 0.5f, 0.2f), objectIdCounter++, LABYRINTH_2, "p22", g_VirtualScene},
            {Matrix_Translate(-6.4f, -1.0f, -4.0f) * Matrix_Scale(0.1f, 0.5f, 0.2f), objectIdCounter++, LABYRINTH_2, "p22", g_VirtualScene},
            {Matrix_Translate(-4.5f, -1.0f, -2.0f) * Matrix_Scale(0.2f, 0.5f, 0.2f), objectIdCounter++, LABYRINTH_2, "p2", g_VirtualScene},

            // Direito inferior
            {Matrix_Translate(3.5f, -1.0f, 2.9f) * Matrix_Scale(0.6f, 0.5f, 0.2f), objectIdCounter++, LABYRINTH_2, "p22", g_VirtualScene},
            {Matrix_Translate(3.0f, -1.0f, 4.3f) * Matrix_Scale(0.4f, 0.5f, 0.2f), objectIdCounter++, LABYRINTH_2, "p22", g_VirtualScene},
            {Matrix_Translate(5.7f, -1.0f, 6.5f) * Matrix_Scale(0.2f, 0.5f, 0.4f), objectIdCounter++, LABYRINTH_2, "p2", g_VirtualScene},
            {Matrix_Translate(3.3f, -1.0f, 6.3f) * Matrix_Scale(0.2f, 0.5f, 0.25f), objectIdCounter++, LABYRINTH_2, "p2", g_VirtualScene},
            {Matrix_Translate(4.5f, -1.0f, 5.0f) * Matrix_Scale(0.2f, 0.5f, 0.25f), objectIdCounter++, LABYRINTH_2, "p2", g_VirtualScene},
            {Matrix_Translate(2.2f, -1.0f, 7.0f) * Matrix_Scale(0.3f, 0.5f, 0.2f), objectIdCounter++, LABYRINTH_2, "p22", g_VirtualScene},
            {Matrix_Translate(7.0f, -1.0f, 4.0f) * Matrix_Scale(0.2f, 0.5f, 0.3f), objectIdCounter++, LABYRINTH_2, "p2", g_VirtualScene},
            {Matrix_Translate(6.5f, -1.0f, 6.8f) * Matrix_Scale(0.2f, 0.5f, 0.2f), objectIdCounter++, LABYRINTH_2, "p22", g_VirtualScene},
            {Matrix_Translate(6.4f, -1.0f, 4.0f) * Matrix_Scale(0.1f, 0.5f, 0.2f), objectIdCounter++, LABYRINTH_2, "p22", g_VirtualScene},
            {Matrix_Translate(4.5f, -1.0f, 2.0f) * Matrix_Scale(0.2f, 0.5f, 0.2f), objectIdCounter++, LABYRINTH_2, "p2", g_VirtualScene},

            // Esquerdo inferior
            {Matrix_Translate(-3.5f, -1.0f, 2.9f) * Matrix_Scale(0.6f, 0.5f, 0.2f), objectIdCounter++, LABYRINTH_2, "p22", g_VirtualScene},
            {Matrix_Translate(-3.0f, -1.0f, 4.3f) * Matrix_Scale(0.4f, 0.5f, 0.2f), objectIdCounter++, LABYRINTH_2, "p22", g_VirtualScene},
            {Matrix_Translate(-5.7f, -1.0f, 6.5f) * Matrix_Scale(0.2f, 0.5f, 0.4f), objectIdCounter++, LABYRINTH_2, "p2", g_VirtualScene},
            {Matrix_Translate(-3.3f, -1.0f, 6.3f) * Matrix_Scale(0.2f, 0.5f, 0.25f), objectIdCounter++, LABYRINTH_2, "p2", g_VirtualScene},
            {Matrix_Translate(-4.5f, -1.0f, 5.0f) * Matrix_Scale(0.2f, 0.5f, 0.25f), objectIdCounter++, LABYRINTH_2, "p2", g_VirtualScene},
            {Matrix_Translate(-2.2f, -1.0f, 7.0f) * Matrix_Scale(0.3f, 0.5f, 0.2f), objectIdCounter++, LABYRINTH_2, "p22", g_VirtualScene},
            {Matrix_Translate(-7.0f, -1.0f, 4.0f) * Matrix_Scale(0.2f, 0.5f, 0.3f), objectIdCounter++, LABYRINTH_2, "p2", g_VirtualScene},
            {Matrix_Translate(-6.5f, -1.0f, 6.8f) * Matrix_Scale(0.2f, 0.5f, 0.2f), objectIdCounter++, LABYRINTH_2, "p22", g_VirtualScene},
            {Matrix_Translate(-6.4f, -1.0f, 4.0f) * Matrix_Scale(0.1f, 0.5f, 0.2f), objectIdCounter++, LABYRINTH_2, "p22", g_VirtualScene},
            {Matrix_Translate(-4.5f, -1.0f, 2.0f) * Matrix_Scale(0.2f, 0.5f, 0.2f), objectIdCounter++, LABYRINTH_2, "p2", g_VirtualScene},

            // Centro lateriais
            {Matrix_Translate(-6.4f, -1.0f, 0.0f) * Matrix_Scale(0.2f, 0.5f, 0.5f), objectIdCounter++, LABYRINTH_2, "p2", g_VirtualScene},
            {Matrix_Translate(6.4f, -1.0f, 0.0f) * Matrix_Scale(0.2f, 0.5f, 0.5f), objectIdCounter++, LABYRINTH_2, "p2", g_VirtualScene},
            {Matrix_Translate(-7.0f, -1.0f, 0.0f) * Matrix_Scale(0.2f, 0.5f, 0.2f), objectIdCounter++, LABYRINTH_2, "p22", g_VirtualScene},
            {Matrix_Translate(7.0f, -1.0f, 0.0f) * Matrix_Scale(0.2f, 0.5f, 0.2f), objectIdCounter++, LABYRINTH_2, "p22", g_VirtualScene},

            // Centro
            {Matrix_Translate(0.0f, -1.0f, -4.0f) * Matrix_Scale(0.2f, 0.5f, 0.4f), objectIdCounter++, LABYRINTH_2, "p2", g_VirtualScene},
            {Matrix_Translate(0.0f, -1.0f, -5.5f) * Matrix_Scale(0.6f, 0.5f, 0.2f), objectIdCounter++, LABYRINTH_2, "p22", g_VirtualScene},
            {Matrix_Translate(0.0f, -1.0f, -7.0f) * Matrix_Scale(0.2f, 0.5f, 0.1f), objectIdCounter++, LABYRINTH_2, "p2", g_VirtualScene},

            {Matrix_Translate(0.0f, -1.0f, 4.0f) * Matrix_Scale(0.2f, 0.5f, 0.4f), objectIdCounter++, LABYRINTH_2, "p2", g_VirtualScene},
            {Matrix_Translate(0.0f, -1.0f, 5.5f) * Matrix_Scale(0.6f, 0.5f, 0.2f), objectIdCounter++, LABYRINTH_2, "p22", g_VirtualScene},
            {Matrix_Translate(0.0f, -1.0f, 7.0f) * Matrix_Scale(0.2f, 0.5f, 0.1f), objectIdCounter++, LABYRINTH_2, "p2", g_VirtualScene},

            {Matrix_Translate(0.0f, -1.0f, 1.0f) * Matrix_Scale(0.4f, 0.5f, 0.3f), objectIdCounter++, LABYRINTH_3, "p3", g_VirtualScene},
            {Matrix_Translate(2.1f, -1.0f, -0.2f) * Matrix_Scale(0.3f, 0.5f, 0.2f), objectIdCounter++, LABYRINTH_3, "p33", g_VirtualScene},
            {Matrix_Translate(-2.1f, -1.0f, -0.2f) * Matrix_Scale(0.3f, 0.5f, 0.2f), objectIdCounter++, LABYRINTH_3, "p33", g_VirtualScene},
            {Matrix_Translate(1.5f, -1.0f, -1.1f) * Matrix_Scale(0.125f, 0.5f, 0.3f), objectIdCounter++, LABYRINTH_3, "p3", g_VirtualScene},
            {Matrix_Translate(-1.5f, -1.0f, -1.1f) * Matrix_Scale(0.125f, 0.5f, 0.3f), objectIdCounter++, LABYRINTH_3, "p3", g_VirtualScene}};
        printf("\n");
        // TextRendering_ShowWallsAABBs(window, walls, sizeof(walls) / sizeof(walls[0]));

        Sphere pacman_sphere = {pacman_position_c, pacman_size + 0.05f};
        std::vector<glm::vec4> all_collision_directions;
        glDepthFunc(GL_ALWAYS);

        glm::mat4 skyModel = Matrix_Scale(farplane / 4, farplane / 4, farplane / 4);
        glUniformMatrix4fv(g_model_uniform, 1, GL_FALSE, glm::value_ptr(skyModel));
        glUniform1i(g_object_id_uniform, BACKGROUND);
        DrawVirtualObject("Cube");

        glm::vec3 skyboxMin = glm::vec3(farplane / 4, farplane / 2, farplane / 4);
        glm::vec3 skyboxMax = glm::vec3(-farplane / 4, -farplane / 2, -farplane / 4);

        AABB sky_bbox = {skyboxMin, skyboxMax};

        glDepthFunc(GL_LESS);

        for (Wall &wall : walls)
        {
            wall.render();
            // Teste de colisão com paredes do labirinto
            glm::vec4 collision_direction = checkSphereToAABBCollisionDirection(wall.wall_bbox, pacman_sphere);
            if (norm(collision_direction) > 0)
            {
                all_collision_directions.push_back(collision_direction);
            }
        }
        // printf("collision directions: ");
        // for (auto &cd : all_collision_directions)
        // {
        //     printf("(%f, %f, %f) ", cd.x, cd.y, cd.z);
        // };

        std::vector<int> remove_indexes = {};

        int index = 0;
        for (Ball &ball : balls)
        {
            bool ate = checkSphereToSphereCollision(pacman_sphere, ball.b_sphere);
            if (ate)
            {
                // Aplicar transformação geométrica ao comer a bolinha
                remove_indexes.push_back(index);
            }
            else
            {
                ball.render();
            }

            index++;
        }

        for (int idx : remove_indexes)
        {
            balls.erase(balls.begin() + idx);
            eaten_ball_count += 1;
        }

        std::vector<int> remove_cherry_indexes = {};

        int cherry_index = 0;
        for (Cherry &cherry : cherries)
        {
            bool ate = checkSphereToSphereCollision(pacman_sphere, cherry.cherry_sphere);
            if (ate)
            {
                remove_cherry_indexes.push_back(cherry_index);
            }
            else
            {
                if (isFreeCamOn)
                {
                    cherry.modelMatrix = Matrix_Translate(cherry.center.x, cherry.center.y, cherry.center.z) * Matrix_Rotate_X(3.14159f) * Matrix_Rotate_Z(3.14159f) * Matrix_Scale(0.001f, 0.001f, 0.001f);
                }
                else
                {
                    cherry.modelMatrix = Matrix_Translate(cherry.center.x, cherry.center.y, cherry.center.z) * Matrix_Rotate_X(3.14159f / 2) * Matrix_Rotate_Z(3.14159f) * Matrix_Scale(0.002f, 0.002f, 0.002f);
                }
                cherry.render();
            }
            cherry_index++;
        }

        for (int idx : remove_cherry_indexes)
        {
            cherries.erase(cherries.begin() + idx);
        }

        renderCount(eaten_ball_count, count_first_digit, count_second_digit, count_third_digit);

        // Testes de colisão com as paredes limítrofes: colisão esfera-plano
        glm::vec4 collision_direction_sky = checkSphereToPlaneCollision(sky_bbox, pacman_sphere);
        all_collision_directions.push_back(collision_direction_sky);

        MovePacman(vertical_move_unit, camera_side_view_unit, ellapsedTime, all_collision_directions);

        // Computamos a matriz "View" utilizando os parâmetros da câmera para
        // definir o sistema de coordenadas da câmera.  Veja slides 2-14, 184-190 e 236-242 do documento Aula_08_Sistemas_de_Coordenadas.pdf.
        glm::mat4 view = Matrix_Camera_View(camera_position_c, camera_view_vector, camera_up_vector);

        // Agora computamos a matriz de Projeção.
        glm::mat4 projection;

        if (g_UsePerspectiveProjection)
        {
            // Projeção Perspectiva.
            // Para definição do field of view (FOV), veja slides 205-215 do documento Aula_09_Projecoes.pdf.
            float field_of_view = 3.141592 / 3.0f;
            projection = Matrix_Perspective(field_of_view, g_ScreenRatio, nearplane, farplane);
        }
        else
        {
            // Projeção Ortográfica.
            // Para definição dos valores l, r, b, t ("left", "right", "bottom", "top"),
            // PARA PROJEÇÃO ORTOGRÁFICA veja slides 219-224 do documento Aula_09_Projecoes.pdf.
            // Para simular um "zoom" ortográfico, computamos o valor de "t"
            // utilizando a variável g_CameraDistance.
            float t = 1.5f * g_CameraDistance / 2.5f;
            float b = -t;
            float r = t * g_ScreenRatio;
            float l = -r;
            projection = Matrix_Orthographic(l, r, b, t, nearplane, farplane);
        }

        glm::mat4 model = Matrix_Identity(); // Transformação identidade de modelagem

        // Enviamos as matrizes "view" e "projection" para a placa de vídeo
        // (GPU). Veja o arquivo "shader_vertex.glsl", onde estas são
        // efetivamente aplicadas em todos os pontos.
        glUniformMatrix4fv(g_view_uniform, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(g_projection_uniform, 1, GL_FALSE, glm::value_ptr(projection));

        model = Matrix_Translate(0.0f, -1.0f, 0.0f) * Matrix_Scale(farplane / 4, 1.0f, farplane / 4);
        glUniformMatrix4fv(g_model_uniform, 1, GL_FALSE, glm::value_ptr(model));
        glUniform1i(g_object_id_uniform, PLANE);
        DrawVirtualObject("the_plane");

        model = Matrix_Translate(pacman_position_c.x, pacman_position_c.y, pacman_position_c.z) * Matrix_Rotate_Y(pacman_rotation) * Matrix_Scale(pacman_size, pacman_size, pacman_size);
        glUniformMatrix4fv(g_model_uniform, 1, GL_FALSE, glm::value_ptr(model));
        glUniform1i(g_object_id_uniform, PACMAN);
        DrawVirtualObject("pacman");

        // model = Matrix_Translate(7.0f, -1.0f, 7.0f) * Matrix_Scale(pacman_size, pacman_size, pacman_size);
        // glUniformMatrix4fv(g_model_uniform, 1, GL_FALSE, glm::value_ptr(model));
        // glUniform1i(g_object_id_uniform, GHOST);
        // DrawVirtualObject("ghost");

        model = Matrix_Translate(1.0f, isFreeCamOn ? 2.0f : -1.0f, isFreeCamOn ? (farplane / 4) : 0.0f) * Matrix_Rotate_X(isFreeCamOn ? 0.0f : 3.14159f / 2) * Matrix_Rotate_Z(isFreeCamOn ? 0.0f : 3.14159f) * Matrix_Rotate_Y(isFreeCamOn ? 0.0f : 3.14159f);
        glUniformMatrix4fv(g_model_uniform, 1, GL_FALSE, glm::value_ptr(model));
        glUniform1i(g_object_id_uniform, COUNT_1);
        DrawVirtualObject(count_first_digit);

        model = Matrix_Translate(0.0f, isFreeCamOn ? 2.0f : -1.0f, isFreeCamOn ? (farplane / 4) : 0.0f) * Matrix_Rotate_X(isFreeCamOn ? 0.0f : 3.14159f / 2) * Matrix_Rotate_Z(isFreeCamOn ? 0.0f : 3.14159f) * Matrix_Rotate_Y(isFreeCamOn ? 0.0f : 3.14159f);
        glUniformMatrix4fv(g_model_uniform, 1, GL_FALSE, glm::value_ptr(model));
        glUniform1i(g_object_id_uniform, COUNT_2);
        DrawVirtualObject(count_second_digit);

        model = Matrix_Translate(-1.0f, isFreeCamOn ? 2.0f : -1.0f, isFreeCamOn ? (farplane / 4) : 0.0f) * Matrix_Rotate_X(isFreeCamOn ? 0.0f : 3.14159f / 2) * Matrix_Rotate_Z(isFreeCamOn ? 0.0f : 3.14159f) * Matrix_Rotate_Y(isFreeCamOn ? 0.0f : 3.14159f);
        glUniformMatrix4fv(g_model_uniform, 1, GL_FALSE, glm::value_ptr(model));
        glUniform1i(g_object_id_uniform, COUNT_3);
        DrawVirtualObject(count_third_digit);

        if (isFreeCamOn)
        {
            model = Matrix_Translate(-1.0f, 2.0f, -farplane / 4) * Matrix_Rotate_Y(3.14159f);
            glUniformMatrix4fv(g_model_uniform, 1, GL_FALSE, glm::value_ptr(model));
            glUniform1i(g_object_id_uniform, COUNT_1);
            DrawVirtualObject(count_first_digit);

            model = Matrix_Translate(0.0f, 2.0f, -farplane / 4) * Matrix_Rotate_Y(3.14159f);
            glUniformMatrix4fv(g_model_uniform, 1, GL_FALSE, glm::value_ptr(model));
            glUniform1i(g_object_id_uniform, COUNT_2);
            DrawVirtualObject(count_second_digit);

            model = Matrix_Translate(1.0f, 2.0f, -farplane / 4) * Matrix_Rotate_Y(3.14159f);
            glUniformMatrix4fv(g_model_uniform, 1, GL_FALSE, glm::value_ptr(model));
            glUniform1i(g_object_id_uniform, COUNT_3);
            DrawVirtualObject(count_third_digit);
        }

        // Imprimimos na informação sobre a matriz de projeção sendo utilizada.
        TextRendering_ShowProjection(window);

        // Imprimimos na tela informação sobre o número de quadros renderizados
        // por segundo (frames per second).
        TextRendering_ShowFramesPerSecond(window);

        // O framebuffer onde OpenGL executa as operações de renderização não
        // é o mesmo que está sendo mostrado para o usuário, caso contrário
        // seria possível ver artefatos conhecidos como "screen tearing". A
        // chamada abaixo faz a troca dos buffers, mostrando para o usuário
        // tudo que foi renderizado pelas funções acima.
        // Veja o link: https://en.wikipedia.org/w/index.php?title=Multiple_buffering&oldid=793452829#Double_buffering_in_computer_graphics
        glfwSwapBuffers(window);

        // Verificamos com o sistema operacional se houve alguma interação do
        // usuário (teclado, mouse, ...). Caso positivo, as funções de callback
        // definidas anteriormente usando glfwSet*Callback() serão chamadas
        // pela biblioteca GLFW.
        glfwPollEvents();
    }

    // Finalizamos o uso dos recursos do sistema operacional
    glfwTerminate();

    // Fim do programa
    return 0;
}

glm::vec4 calculateBezierPosition(glm::vec4 p1, glm::vec4 p2, glm::vec4 p3, glm::vec4 p4, float t)
{
    float b03 = pow((1 - t), 3);
    float b13 = 3 * t * pow((1 - t), 2);
    float b23 = 3 * pow(t, 2) * (1 - t);
    float b33 = pow(t, 3);

    return b03 * p1 + b13 * p2 + b23 * p3 + b33 * p4;
}

std::vector<Ball> instanciateLittleBalls()
{
    std::vector<Ball> balls;

    // Quadrado de fora:
    float z = 8.0f;
    for (int i = 0; i < 33; i++)
    {
        Ball right = {glm::vec3(8.5f, -0.8f, z), 0.1f, SPHERE, "the_sphere"};
        balls.push_back(right);

        Ball left = {glm::vec3(-8.5, -0.8f, z), 0.1f, SPHERE, "the_sphere"};
        balls.push_back(left);

        z -= 0.5f;
    }
    float x = 8.0f;
    for (int i = 0; i < 33; i++)
    {
        Ball up = {glm::vec3(x, -0.8f, 8.5), 0.1f, SPHERE, "the_sphere"};
        balls.push_back(up);

        Ball down = {glm::vec3(x, -0.8f, -8.5), 0.1f, SPHERE, "the_sphere"};
        balls.push_back(down);

        x -= 0.5f;
    }

    // Adiciona as bolinhas dos cantos manualmente para não sobrescrever:
    Ball outerCornerBall = {glm::vec3(8.5, -0.8f, 8.5), 0.1f, SPHERE, "the_sphere"};
    balls.push_back(outerCornerBall);
    outerCornerBall = {glm::vec3(-8.5, -0.8f, 8.5), 0.1f, SPHERE, "the_sphere"};
    balls.push_back(outerCornerBall);
    outerCornerBall = {glm::vec3(8.5f, -0.8f, -8.5), 0.1f, SPHERE, "the_sphere"};
    balls.push_back(outerCornerBall);
    outerCornerBall = {glm::vec3(-8.5, -0.8f, -8.5), 0.1f, SPHERE, "the_sphere"};
    balls.push_back(outerCornerBall);

    // Quadrado de dentro:
    z = -1.5f;
    for (int i = 0; i < 7; i++)
    {
        Ball inner_left = {glm::vec3(3.2f, -0.8f, z), 0.1f, SPHERE, "the_sphere"};
        balls.push_back(inner_left);

        Ball inner_right = {glm::vec3(-3.35f, -0.8f, z), 0.1f, SPHERE, "the_sphere"};
        balls.push_back(inner_right);

        z += 0.5f; // de -1.5 a 3.0
    }
    x = -2.85f;
    for (int i = 0; i < 12; i++)
    {
        Ball inner_up = {glm::vec3(x, -0.8f, -2.0f), 0.1f, SPHERE, "the_sphere"};
        balls.push_back(inner_up);

        Ball inner_down = {glm::vec3(x, -0.8f, 2.0f), 0.1f, SPHERE, "the_sphere"};
        balls.push_back(inner_down);

        x += 0.5f;
    }
    // Adiciona as bolinhas dos cantos manualmente para não sobrescrever:
    Ball innerCornerBall = {glm::vec3(-3.35, -0.8f, -2.0f), 0.1f, SPHERE, "the_sphere"};
    balls.push_back(innerCornerBall);
    innerCornerBall = {glm::vec3(-3.35, -0.8f, 2.0f), 0.1f, SPHERE, "the_sphere"};
    balls.push_back(innerCornerBall);
    innerCornerBall = {glm::vec3(3.2, -0.8f, -2.0f), 0.1f, SPHERE, "the_sphere"};
    balls.push_back(innerCornerBall);
    innerCornerBall = {glm::vec3(3.2, -0.8f, 2.0f), 0.1f, SPHERE, "the_sphere"};
    balls.push_back(innerCornerBall);

    return balls;
};

std::vector<Cherry> instanciateCherries()
{
    std::vector<Cherry> cherries;
    int objectIdCounter = 0;

    float x = -4.5f;
    float z = -7.0f;
    for (int i = 0; i < 4; i++)
    {
        Cherry cherry = {Matrix_Translate(x, -0.5f, z) * Matrix_Rotate_X(3.14159f / 2) * Matrix_Rotate_Z(3.14159f) * Matrix_Scale(0.002f, 0.002f, 0.002f), objectIdCounter++, CHERRY, "Cherry", glm::vec3(x, -0.5f, z), 0.5f};
        cherries.push_back(cherry);

        if (i % 2 == 0)
            x = -x;

        z = -z;
    }

    return cherries;
}

glm::vec4 cancelCollisionMovement(glm::vec4 movement, std::vector<glm::vec4> collision_directions)
{
    for (auto &cd : collision_directions)
    {
        float result = dotproduct(cd, movement);
        printf("dot product: %f", result);
        if (result > 0)
        {
            if (cd.x != 0)
            {
                movement.x = 0;
            }
            else if (cd.y != 0)
            {
                movement.y = 0;
            }
            else if (cd.z != 0)
            {
                movement.z = 0;
            }
        }
    }
    return movement;
}

void renderCount(int current_count, char *count_first_digit, char *count_second_digit, char *count_third_digit)
{
    std::ostringstream oss;
    oss << std::setfill('0') << std::setw(3) << current_count;
    std::string numStr = oss.str();

    snprintf(count_first_digit, 4, "N_%c", numStr[2]);
    snprintf(count_second_digit, 4, "N_%c", numStr[1]);
    snprintf(count_third_digit, 4, "N_%c", numStr[0]);
}

void MovePacman(glm::vec4 camera_view_unit, glm::vec4 camera_side_view_unit, float ellapsedTime, std::vector<glm::vec4> collision_directions)
{
    if (movePacmanBackward)
    {
        pacman_movement = -camera_view_unit * PACMAN_SPEED * ellapsedTime;
        pacman_movement = cancelCollisionMovement(pacman_movement, collision_directions);

        pacman_position_c += pacman_movement;
        pacman_rotation = -3.14159f / 2;
    }

    if (movePacmanForward)
    {
        pacman_movement = camera_view_unit * PACMAN_SPEED * ellapsedTime;
        pacman_movement = cancelCollisionMovement(pacman_movement, collision_directions);
        pacman_position_c += pacman_movement;
        pacman_rotation = 3.14159f / 2;
    }

    if (movePacmanRight)
    {
        pacman_movement = -camera_side_view_unit * PACMAN_SPEED * ellapsedTime;
        pacman_movement = cancelCollisionMovement(pacman_movement, collision_directions);
        pacman_position_c += pacman_movement;
        pacman_rotation = 0.0f;
    }

    if (movePacmanLeft)
    {
        pacman_movement = camera_side_view_unit * PACMAN_SPEED * ellapsedTime;
        pacman_movement = cancelCollisionMovement(pacman_movement, collision_directions);
        pacman_position_c += pacman_movement;
        pacman_rotation = 3.14159f;
    }
    // printf("Pacman movement: (%f, %f, %f)", pacman_movement.x, pacman_movement.y, pacman_movement.z);
}

// Função que desenha um objeto armazenado em g_VirtualScene. Veja definição
// dos objetos na função BuildTrianglesAndAddToVirtualScene().
void DrawVirtualObject(const char *object_name)
{
    // "Ligamos" o VAO. Informamos que queremos utilizar os atributos de
    // vértices apontados pelo VAO criado pela função BuildTrianglesAndAddToVirtualScene(). Veja
    // comentários detalhados dentro da definição de BuildTrianglesAndAddToVirtualScene().
    glBindVertexArray(g_VirtualScene[object_name].vertex_array_object_id);

    // Setamos as variáveis "bbox_min" e "bbox_max" do fragment shader
    // com os parâmetros da axis-aligned bounding box (AABB) do modelo.
    glm::vec3 bbox_min = g_VirtualScene[object_name].bbox_min;
    glm::vec3 bbox_max = g_VirtualScene[object_name].bbox_max;
    glUniform4f(g_bbox_min_uniform, bbox_min.x, bbox_min.y, bbox_min.z, 1.0f);
    glUniform4f(g_bbox_max_uniform, bbox_max.x, bbox_max.y, bbox_max.z, 1.0f);

    // Pedimos para a GPU rasterizar os vértices dos eixos XYZ
    // apontados pelo VAO como linhas. Veja a definição de
    // g_VirtualScene[""] dentro da função BuildTrianglesAndAddToVirtualScene(), e veja
    // a documentação da função glDrawElements() em
    // http://docs.gl/gl3/glDrawElements.
    glDrawElements(
        g_VirtualScene[object_name].rendering_mode,
        g_VirtualScene[object_name].num_indices,
        GL_UNSIGNED_INT,
        (void *)(g_VirtualScene[object_name].first_index * sizeof(GLuint)));

    // "Desligamos" o VAO, evitando assim que operações posteriores venham a
    // alterar o mesmo. Isso evita bugs.
    glBindVertexArray(0);
}

// Função que pega a matriz M e guarda a mesma no topo da pilha
void PushMatrix(glm::mat4 M)
{
    g_MatrixStack.push(M);
}

// Função que remove a matriz atualmente no topo da pilha e armazena a mesma na variável M
void PopMatrix(glm::mat4 &M)
{
    if (g_MatrixStack.empty())
    {
        M = Matrix_Identity();
    }
    else
    {
        M = g_MatrixStack.top();
        g_MatrixStack.pop();
    }
}

// Constrói triângulos para futura renderização a partir de um ObjModel.
void BuildTrianglesAndAddToVirtualScene(ObjModel *model)
{
    GLuint vertex_array_object_id;
    glGenVertexArrays(1, &vertex_array_object_id);
    glBindVertexArray(vertex_array_object_id);

    std::vector<GLuint> indices;
    std::vector<float> model_coefficients;
    std::vector<float> normal_coefficients;
    std::vector<float> texture_coefficients;

    for (size_t shape = 0; shape < model->shapes.size(); ++shape)
    {
        size_t first_index = indices.size();
        size_t num_triangles = model->shapes[shape].mesh.num_face_vertices.size();

        const float minval = std::numeric_limits<float>::min();
        const float maxval = std::numeric_limits<float>::max();

        glm::vec3 bbox_min = glm::vec3(maxval, maxval, maxval);
        glm::vec3 bbox_max = glm::vec3(minval, minval, minval);

        for (size_t triangle = 0; triangle < num_triangles; ++triangle)
        {
            assert(model->shapes[shape].mesh.num_face_vertices[triangle] == 3);

            for (size_t vertex = 0; vertex < 3; ++vertex)
            {
                tinyobj::index_t idx = model->shapes[shape].mesh.indices[3 * triangle + vertex];

                indices.push_back(first_index + 3 * triangle + vertex);

                const float vx = model->attrib.vertices[3 * idx.vertex_index + 0];
                const float vy = model->attrib.vertices[3 * idx.vertex_index + 1];
                const float vz = model->attrib.vertices[3 * idx.vertex_index + 2];
                // printf("tri %d vert %d = (%.2f, %.2f, %.2f)\n", (int)triangle, (int)vertex, vx, vy, vz);
                model_coefficients.push_back(vx);   // X
                model_coefficients.push_back(vy);   // Y
                model_coefficients.push_back(vz);   // Z
                model_coefficients.push_back(1.0f); // W

                bbox_min.x = std::min(bbox_min.x, vx);
                bbox_min.y = std::min(bbox_min.y, vy);
                bbox_min.z = std::min(bbox_min.z, vz);
                bbox_max.x = std::max(bbox_max.x, vx);
                bbox_max.y = std::max(bbox_max.y, vy);
                bbox_max.z = std::max(bbox_max.z, vz);

                // Inspecionando o código da tinyobjloader, o aluno Bernardo
                // Sulzbach (2017/1) apontou que a maneira correta de testar se
                // existem normais e coordenadas de textura no ObjModel é
                // comparando se o índice retornado é -1. Fazemos isso abaixo.

                if (idx.normal_index != -1)
                {
                    const float nx = model->attrib.normals[3 * idx.normal_index + 0];
                    const float ny = model->attrib.normals[3 * idx.normal_index + 1];
                    const float nz = model->attrib.normals[3 * idx.normal_index + 2];
                    normal_coefficients.push_back(nx);   // X
                    normal_coefficients.push_back(ny);   // Y
                    normal_coefficients.push_back(nz);   // Z
                    normal_coefficients.push_back(0.0f); // W
                }

                if (idx.texcoord_index != -1)
                {
                    const float u = model->attrib.texcoords[2 * idx.texcoord_index + 0];
                    const float v = model->attrib.texcoords[2 * idx.texcoord_index + 1];
                    texture_coefficients.push_back(u);
                    texture_coefficients.push_back(v);
                }
            }
        }

        size_t last_index = indices.size() - 1;

        SceneObject theobject;
        theobject.name = model->shapes[shape].name;
        theobject.first_index = first_index;                  // Primeiro índice
        theobject.num_indices = last_index - first_index + 1; // Número de indices
        theobject.rendering_mode = GL_TRIANGLES;              // Índices correspondem ao tipo de rasterização GL_TRIANGLES.
        theobject.vertex_array_object_id = vertex_array_object_id;

        theobject.bbox_min = bbox_min;
        theobject.bbox_max = bbox_max;

        g_VirtualScene[model->shapes[shape].name] = theobject;
    }

    GLuint VBO_model_coefficients_id;
    glGenBuffers(1, &VBO_model_coefficients_id);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_model_coefficients_id);
    glBufferData(GL_ARRAY_BUFFER, model_coefficients.size() * sizeof(float), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, model_coefficients.size() * sizeof(float), model_coefficients.data());
    GLuint location = 0;            // "(location = 0)" em "shader_vertex.glsl"
    GLint number_of_dimensions = 4; // vec4 em "shader_vertex.glsl"
    glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(location);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    if (!normal_coefficients.empty())
    {
        GLuint VBO_normal_coefficients_id;
        glGenBuffers(1, &VBO_normal_coefficients_id);
        glBindBuffer(GL_ARRAY_BUFFER, VBO_normal_coefficients_id);
        glBufferData(GL_ARRAY_BUFFER, normal_coefficients.size() * sizeof(float), NULL, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, normal_coefficients.size() * sizeof(float), normal_coefficients.data());
        location = 1;             // "(location = 1)" em "shader_vertex.glsl"
        number_of_dimensions = 4; // vec4 em "shader_vertex.glsl"
        glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(location);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    if (!texture_coefficients.empty())
    {
        GLuint VBO_texture_coefficients_id;
        glGenBuffers(1, &VBO_texture_coefficients_id);
        glBindBuffer(GL_ARRAY_BUFFER, VBO_texture_coefficients_id);
        glBufferData(GL_ARRAY_BUFFER, texture_coefficients.size() * sizeof(float), NULL, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, texture_coefficients.size() * sizeof(float), texture_coefficients.data());
        location = 2;             // "(location = 1)" em "shader_vertex.glsl"
        number_of_dimensions = 2; // vec2 em "shader_vertex.glsl"
        glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(location);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    GLuint indices_id;
    glGenBuffers(1, &indices_id);

    // "Ligamos" o buffer. Note que o tipo agora é GL_ELEMENT_ARRAY_BUFFER.
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices_id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, indices.size() * sizeof(GLuint), indices.data());
    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); // XXX Errado!
    //

    // "Desligamos" o VAO, evitando assim que operações posteriores venham a
    // alterar o mesmo. Isso evita bugs.
    glBindVertexArray(0);
}

// void TextRendering_ShowWallsAABBs(GLFWwindow *window, Wall walls[], size_t size)
// {
//     if (!g_ShowInfoText)
//         return;

//     float pad = TextRendering_LineHeight(window);
//     char buffer[140];

//     float yPosition = 1.0f - pad; // Posição inicial

//     for (int i = 0; i < size; i++)
//     {
//         Wall &wall = walls[i];

//         // Printa as coordenadas max e min de AABB para debug
//         // snprintf(buffer, 140,
//         //          "Wall %d coordinates: Min(X: %.2f, Y: %.2f, Z: %.2f) Max(X: %.2f, Y: %.2f, Z: %.2f, Offx:%.2f,Offy:%.2f,Offz:%.2f, Coll: %s)\n",
//         //          i,
//         //          wall.minMaxCorner.min.x, wall.minMaxCorner.min.y, wall.minMaxCorner.min.z,
//         //          wall.minMaxCorner.max.x, wall.minMaxCorner.max.y, wall.minMaxCorner.max.z,
//         //         //  wall.collisionOffset.x,
//         //         //  wall.collisionOffset.y,
//         //         //  wall.collisionOffset.z,
//         //         //  wall.isColliding ? "T" : "F");

//         TextRendering_PrintString(window, buffer, -1.0f + pad / 10, yPosition, 1.0f);
//         yPosition -= pad * 1.5f;
//     }
// }

// Função para debugging: imprime no terminal todas informações de um modelo
// geométrico carregado de um arquivo ".obj".
// Veja: https://github.com/syoyo/tinyobjloader/blob/22883def8db9ef1f3ffb9b404318e7dd25fdbb51/loader_example.cc#L98
void PrintObjModelInfo(ObjModel *model)
{
    const tinyobj::attrib_t &attrib = model->attrib;
    const std::vector<tinyobj::shape_t> &shapes = model->shapes;
    const std::vector<tinyobj::material_t> &materials = model->materials;

    printf("# of vertices  : %d\n", (int)(attrib.vertices.size() / 3));
    printf("# of normals   : %d\n", (int)(attrib.normals.size() / 3));
    printf("# of texcoords : %d\n", (int)(attrib.texcoords.size() / 2));
    printf("# of shapes    : %d\n", (int)shapes.size());
    printf("# of materials : %d\n", (int)materials.size());

    for (size_t v = 0; v < attrib.vertices.size() / 3; v++)
    {
        printf("  v[%ld] = (%f, %f, %f)\n", static_cast<long>(v),
               static_cast<const double>(attrib.vertices[3 * v + 0]),
               static_cast<const double>(attrib.vertices[3 * v + 1]),
               static_cast<const double>(attrib.vertices[3 * v + 2]));
    }

    for (size_t v = 0; v < attrib.normals.size() / 3; v++)
    {
        printf("  n[%ld] = (%f, %f, %f)\n", static_cast<long>(v),
               static_cast<const double>(attrib.normals[3 * v + 0]),
               static_cast<const double>(attrib.normals[3 * v + 1]),
               static_cast<const double>(attrib.normals[3 * v + 2]));
    }

    for (size_t v = 0; v < attrib.texcoords.size() / 2; v++)
    {
        printf("  uv[%ld] = (%f, %f)\n", static_cast<long>(v),
               static_cast<const double>(attrib.texcoords[2 * v + 0]),
               static_cast<const double>(attrib.texcoords[2 * v + 1]));
    }

    // For each shape
    for (size_t i = 0; i < shapes.size(); i++)
    {
        printf("shape[%ld].name = %s\n", static_cast<long>(i),
               shapes[i].name.c_str());
        printf("Size of shape[%ld].indices: %lu\n", static_cast<long>(i),
               static_cast<unsigned long>(shapes[i].mesh.indices.size()));

        size_t index_offset = 0;

        assert(shapes[i].mesh.num_face_vertices.size() ==
               shapes[i].mesh.material_ids.size());

        printf("shape[%ld].num_faces: %lu\n", static_cast<long>(i),
               static_cast<unsigned long>(shapes[i].mesh.num_face_vertices.size()));

        // For each face
        for (size_t f = 0; f < shapes[i].mesh.num_face_vertices.size(); f++)
        {
            size_t fnum = shapes[i].mesh.num_face_vertices[f];

            printf("  face[%ld].fnum = %ld\n", static_cast<long>(f),
                   static_cast<unsigned long>(fnum));

            // For each vertex in the face
            for (size_t v = 0; v < fnum; v++)
            {
                tinyobj::index_t idx = shapes[i].mesh.indices[index_offset + v];
                printf("    face[%ld].v[%ld].idx = %d/%d/%d\n", static_cast<long>(f),
                       static_cast<long>(v), idx.vertex_index, idx.normal_index,
                       idx.texcoord_index);
            }

            printf("  face[%ld].material_id = %d\n", static_cast<long>(f),
                   shapes[i].mesh.material_ids[f]);

            index_offset += fnum;
        }

        printf("shape[%ld].num_tags: %lu\n", static_cast<long>(i),
               static_cast<unsigned long>(shapes[i].mesh.tags.size()));
        for (size_t t = 0; t < shapes[i].mesh.tags.size(); t++)
        {
            printf("  tag[%ld] = %s ", static_cast<long>(t),
                   shapes[i].mesh.tags[t].name.c_str());
            printf(" ints: [");
            for (size_t j = 0; j < shapes[i].mesh.tags[t].intValues.size(); ++j)
            {
                printf("%ld", static_cast<long>(shapes[i].mesh.tags[t].intValues[j]));
                if (j < (shapes[i].mesh.tags[t].intValues.size() - 1))
                {
                    printf(", ");
                }
            }
            printf("]");

            printf(" floats: [");
            for (size_t j = 0; j < shapes[i].mesh.tags[t].floatValues.size(); ++j)
            {
                printf("%f", static_cast<const double>(
                                 shapes[i].mesh.tags[t].floatValues[j]));
                if (j < (shapes[i].mesh.tags[t].floatValues.size() - 1))
                {
                    printf(", ");
                }
            }
            printf("]");

            printf(" strings: [");
            for (size_t j = 0; j < shapes[i].mesh.tags[t].stringValues.size(); ++j)
            {
                printf("%s", shapes[i].mesh.tags[t].stringValues[j].c_str());
                if (j < (shapes[i].mesh.tags[t].stringValues.size() - 1))
                {
                    printf(", ");
                }
            }
            printf("]");
            printf("\n");
        }
    }

    for (size_t i = 0; i < materials.size(); i++)
    {
        printf("material[%ld].name = %s\n", static_cast<long>(i),
               materials[i].name.c_str());
        printf("  material.Ka = (%f, %f ,%f)\n",
               static_cast<const double>(materials[i].ambient[0]),
               static_cast<const double>(materials[i].ambient[1]),
               static_cast<const double>(materials[i].ambient[2]));
        printf("  material.Kd = (%f, %f ,%f)\n",
               static_cast<const double>(materials[i].diffuse[0]),
               static_cast<const double>(materials[i].diffuse[1]),
               static_cast<const double>(materials[i].diffuse[2]));
        printf("  material.Ks = (%f, %f ,%f)\n",
               static_cast<const double>(materials[i].specular[0]),
               static_cast<const double>(materials[i].specular[1]),
               static_cast<const double>(materials[i].specular[2]));
        printf("  material.Tr = (%f, %f ,%f)\n",
               static_cast<const double>(materials[i].transmittance[0]),
               static_cast<const double>(materials[i].transmittance[1]),
               static_cast<const double>(materials[i].transmittance[2]));
        printf("  material.Ke = (%f, %f ,%f)\n",
               static_cast<const double>(materials[i].emission[0]),
               static_cast<const double>(materials[i].emission[1]),
               static_cast<const double>(materials[i].emission[2]));
        printf("  material.Ns = %f\n",
               static_cast<const double>(materials[i].shininess));
        printf("  material.Ni = %f\n", static_cast<const double>(materials[i].ior));
        printf("  material.dissolve = %f\n",
               static_cast<const double>(materials[i].dissolve));
        printf("  material.illum = %d\n", materials[i].illum);
        printf("  material.map_Ka = %s\n", materials[i].ambient_texname.c_str());
        printf("  material.map_Kd = %s\n", materials[i].diffuse_texname.c_str());
        printf("  material.map_Ks = %s\n", materials[i].specular_texname.c_str());
        printf("  material.map_Ns = %s\n",
               materials[i].specular_highlight_texname.c_str());
        printf("  material.map_bump = %s\n", materials[i].bump_texname.c_str());
        printf("  material.map_d = %s\n", materials[i].alpha_texname.c_str());
        printf("  material.disp = %s\n", materials[i].displacement_texname.c_str());
        printf("  <<PBR>>\n");
        printf("  material.Pr     = %f\n", materials[i].roughness);
        printf("  material.Pm     = %f\n", materials[i].metallic);
        printf("  material.Ps     = %f\n", materials[i].sheen);
        printf("  material.Pc     = %f\n", materials[i].clearcoat_thickness);
        printf("  material.Pcr    = %f\n", materials[i].clearcoat_thickness);
        printf("  material.aniso  = %f\n", materials[i].anisotropy);
        printf("  material.anisor = %f\n", materials[i].anisotropy_rotation);
        printf("  material.map_Ke = %s\n", materials[i].emissive_texname.c_str());
        printf("  material.map_Pr = %s\n", materials[i].roughness_texname.c_str());
        printf("  material.map_Pm = %s\n", materials[i].metallic_texname.c_str());
        printf("  material.map_Ps = %s\n", materials[i].sheen_texname.c_str());
        printf("  material.norm   = %s\n", materials[i].normal_texname.c_str());
        std::map<std::string, std::string>::const_iterator it(
            materials[i].unknown_parameter.begin());
        std::map<std::string, std::string>::const_iterator itEnd(
            materials[i].unknown_parameter.end());

        for (; it != itEnd; it++)
        {
            printf("  material.%s = %s\n", it->first.c_str(), it->second.c_str());
        }
        printf("\n");
    }
}

// set makeprg=cd\ ..\ &&\ make\ run\ >/dev/null
// vim: set spell spelllang=pt_br :
