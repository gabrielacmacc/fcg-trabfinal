#pragma once

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

// Headers das bibliotecas OpenGL
#include <external/glad/glad.h>  // Criação de contexto OpenGL 3.3
#include <external/GLFW/glfw3.h> // Criação de janelas do sistema operacional

// Headers da biblioteca GLM: criação de matrizes e vetores.
#include <external/glm/mat4x4.hpp>
#include <external/glm/vec4.hpp>
#include <external/glm/gtc/type_ptr.hpp>

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

GLuint g_NumLoadedTextures = 0;

// Variáveis que definem um programa de GPU (shaders). Veja função LoadShadersFromFiles().
GLuint g_GpuProgramID = 0;
GLint g_model_uniform;
GLint g_view_uniform;
GLint g_projection_uniform;
GLint g_object_id_uniform;
GLint g_bbox_min_uniform;
GLint g_bbox_max_uniform;

// Razão de proporção da janela (largura/altura). Veja função FramebufferSizeCallback().
float g_ScreenRatio = 1.0f;

// "g_LeftMouseButtonPressed = true" se o usuário está com o botão esquerdo do mouse
// pressionado no momento atual. Veja função MouseButtonCallback().
bool g_LeftMouseButtonPressed = false;
bool g_RightMouseButtonPressed = false;  // Análogo para botão direito do mouse
bool g_MiddleMouseButtonPressed = false; // Análogo para botão do meio do mouse

// Variáveis que definem a câmera em coordenadas esféricas, controladas pelo
// usuário através do mouse (veja função CursorPosCallback()). A posição
// efetiva da câmera é calculada dentro da função main(), dentro do loop de
// renderização.
float g_CameraTheta = 0.0f;     // Ângulo no plano ZX em relação ao eixo Z
float g_CameraPhi = 0.0f;       // Ângulo em relação ao eixo Y
float g_CameraDistance = 15.0f; // Distância da câmera para a origem

const float MAX_BOUNDARY = 9.0f;
const float MIN_BOUNDARY = -9.0f;

float previousTime = (float)glfwGetTime();
float pacmanPreviousTime = (float)glfwGetTime();

bool isFreeCamOn = false;

// Variável que controla o tipo de projeção utilizada: perspectiva ou ortográfica.
bool g_UsePerspectiveProjection = true;

// Variável que controla se o texto informativo será mostrado na tela.
bool g_ShowInfoText = false;

// Variáveis para controle de movimento do pacman
const float PACMAN_DISTANCE = 0.5f;

float pacman_freecam_size = 0.1f;
float pacman_lookat_size = 0.3f;
float pacman_size = pacman_lookat_size;

const float PACMAN_ORIGINAL_SPEED = 2.5f;
const float PACMAN_BOOST = 4.0f;
float PACMAN_SPEED = PACMAN_ORIGINAL_SPEED;
bool shouldBoostSpeed = false;

bool movePacmanForward = false;
bool movePacmanBackward = false;
bool movePacmanRight = false;
bool movePacmanLeft = false;

float pacman_initial_rotation = 3.14159f / 2;
float pacman_rotation = pacman_initial_rotation;

float t = 0.0f;

glm::vec4 initial_position_bezier = glm::vec4(1.0f, 10.0f, 3.0f, 1.0f);
glm::vec4 intermediate_position_bezier_1 = glm::vec4(6.0f, 7.0f, 6.0f, 1.0f);
glm::vec4 intermediate_position_bezier_2 = glm::vec4(-6.0f, 3.0f, 2.0f, 1.0f);
glm::vec4 final_position_bezier = glm::vec4(0.0f, -1.0f, -1.0f, 1.0f);

glm::vec4 curr_bezier_position = initial_position_bezier;
glm::vec4 pacman_position_initial = final_position_bezier;
glm::vec4 pacman_movement = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
glm::vec4 pacman_position_c = pacman_position_initial;

// Variáveis para controle de movimento do fantasma
float ghost_freecam_size = 0.2f;
float ghost_lookat_size = 0.4f;
float ghost_size = ghost_lookat_size;

const float GHOST_SPEED = 5.0f;
bool shouldStopGhost = false;

glm::vec4 ghost_position_initial = glm::vec4(9.2f, -1.0f, 8.8f, 0.0f);
glm::vec4 ghost_position_final = glm::vec4(-7.8f, -1.0f, -8.5f, 0.0f);
glm::vec4 ghost_movement = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
glm::vec4 ghost_position_c = ghost_position_initial;
