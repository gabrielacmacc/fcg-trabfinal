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
#include "objects/ball.hpp"
#include "objects/cherry.hpp"
#include "objects/ghost.hpp"
#include "objects/numbers.hpp"
#include "objects/objects.hpp"
#include "objects/pacman.hpp"
#include "objects/wall.hpp"
#include "callbacks/callbacks.hpp"
#include "collisions/collisions.hpp"
#include "globals/globals.hpp"
#include "textrendering/textrendering.hpp"
#include "utils/error_utils.h"
#include "utils/shader_utils.hpp"
#include "utils/texture_utils.hpp"

// Declaração de funções utilizadas para pilha de matrizes de modelagem.
void PushMatrix(glm::mat4 M);
void PopMatrix(glm::mat4 &M);

using namespace std;

// Pilha que guardará as matrizes de modelagem.
std::stack<glm::mat4> g_MatrixStack;

glm::vec4 camera_position_c;  // Ponto "c", centro da câmera
glm::vec4 camera_lookat_l;    // Ponto "l", para onde a câmera (look-at) estará sempre olhando
glm::vec4 camera_view_vector; // Vetor "view", sentido para onde a câmera está virada
glm::vec4 camera_view_unit;   // Vetor "view" unitário
glm::vec4 camera_distance;
glm::vec4 camera_up_vector; // Vetor "up" fixado para apontar para o "céu" (eixo Y global)
glm::vec4 camera_up_unit;
glm::vec4 camera_side_view;
glm::vec4 camera_side_view_unit;

Ghost first_ghost;
Ghost second_ghost;
std::vector<Ball> balls;
std::vector<Cherry> cherries;
std::vector<Wall> walls;
int initial_ball_count;
int eaten_ball_count;

void initialize_game();

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
    LoadObjects();

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
    initialize_game();

    char count_first_digit[4] = {0};
    char count_second_digit[4] = {0};
    char count_third_digit[4] = {0};

    // Ficamos em um loop infinito, renderizando, até que o usuário feche a janela
    while (!glfwWindowShouldClose(window))
    {

        if (should_restart)
        {
            initialize_game();
        }
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
        float elapsedTime = currentTime - previousTime;
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

        float g_CameraPhiSin = sin(g_CameraPhi);
        float g_CameraPhiCos = cos(g_CameraPhi);
        float g_CameraThetaSin = sin(g_CameraTheta);
        float g_CameraThetaCos = cos(g_CameraTheta);

        float r = g_CameraDistance;
        float y = 0.0f; // Limita mais ainda ângulo da free camera
        float z = r * g_CameraPhiCos * g_CameraThetaCos;
        float x = r * g_CameraPhiCos * g_CameraThetaSin;

        // Abaixo definimos as varáveis que efetivamente definem a câmera virtual.
        camera_position_c = glm::vec4(x, y, z, 1.0f);
        camera_lookat_l = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
        camera_up_vector = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);

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
            camera_position_c = glm::vec4(0.0f, r, 0.0f, 1.0f);
            camera_up_vector = glm::vec4(0.0f, 0.0f, -1.0f, 0.0f);
            camera_view_vector = camera_lookat_l - camera_position_c;
        }

        camera_view_unit = camera_view_vector / norm(camera_view_vector);
        camera_side_view = crossproduct(camera_up_vector, camera_view_unit);
        camera_side_view_unit = camera_side_view / norm(camera_side_view);
        camera_up_unit = camera_up_vector / norm(camera_up_vector);

        glm::vec4 camera_v_view_unit = camera_view_unit;
        camera_v_view_unit.y = 0.0f;
        glm::vec4 vertical_move_unit = isFreeCamOn ? camera_v_view_unit : camera_up_unit;

        // Note que, no sistema de coordenadas da câmera, os planos near e far
        // estão no sentido negativo! Veja slides 176-204 do documento Aula_09_Projecoes.pdf.
        float nearplane = -0.1f; // Posição do "near plane"
        float farplane = -40.0f; // Posição do "far plane"

        Sphere pacman_sphere = {pacman_position_c, pacman_size + 0.1f};
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

        checkWallsCollision(walls, pacman_sphere, all_collision_directions);
        checkLittleBallsCollision(balls, pacman_sphere, eaten_ball_count);
        checkCherriesCollision(cherries, pacman_sphere);

        // checkGhostsCollision(ghost, pacman_sphere);
        // checkGhostsCollision(second_ghost, pacman_sphere);

        if (shouldBoostSpeed)
        {
            pacmanPreviousTime = previousTime;
            BoostPacmanSpeed(pacmanPreviousTime);
        }

        renderCount(eaten_ball_count, count_first_digit, count_second_digit, count_third_digit);

        // Testes de colisão com as paredes limítrofes: colisão esfera-plano
        glm::vec4 collision_direction_sky = checkSphereToPlaneCollision(sky_bbox, pacman_sphere);
        all_collision_directions.push_back(collision_direction_sky);

        MovePacman(vertical_move_unit, camera_side_view_unit, elapsedTime, all_collision_directions);

        first_ghost.move(elapsedTime);
        second_ghost.move(elapsedTime);
        won = balls.size() == 0;
        game_over = first_ghost.collided(pacman_sphere) || second_ghost.collided(pacman_sphere) || won;

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

        first_ghost.render();
        second_ghost.render();

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

void initialize_game()
{
    inicialize_globals();
    balls = instanciateLittleBalls();
    cherries = instanciateCherries();
    walls = instanciateWalls();
    first_ghost = instanciateGhost(FIRST);
    second_ghost = instanciateGhost(SECOND);

    initial_ball_count = balls.size();
    eaten_ball_count = 0;
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

// set makeprg=cd\ ..\ &&\ make\ run\ >/dev/null
// vim: set spell spelllang=pt_br :
