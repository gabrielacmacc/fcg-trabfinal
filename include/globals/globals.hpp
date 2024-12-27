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

// Ângulos de Euler que controlam a rotação de um dos cubos da cena virtual
float g_AngleX = 0.0f;
float g_AngleY = 0.0f;
float g_AngleZ = 0.0f;

// Variáveis que definem a câmera em coordenadas esféricas, controladas pelo
// usuário através do mouse (veja função CursorPosCallback()). A posição
// efetiva da câmera é calculada dentro da função main(), dentro do loop de
// renderização.
float g_CameraTheta = 0.0f;     // Ângulo no plano ZX em relação ao eixo Z
float g_CameraPhi = 0.0f;       // Ângulo em relação ao eixo Y
float g_CameraDistance = 15.0f; // Distância da câmera para a origem

// Variáveis que controlam a posição e movimentação da câmera
glm::vec4 camera_movement = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
glm::vec4 camera_offset = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);

const float CAMERA_SPEED = 3.0f;

float previousTime = (float)glfwGetTime();

float r_initial = g_CameraDistance;
float x_initial = r_initial * cos(g_CameraPhi) * sin(g_CameraTheta);
float y_initial = r_initial * sin(g_CameraPhi);
float z_initial = r_initial * cos(g_CameraPhi) * cos(g_CameraTheta);

glm::vec4 camera_position_initial = glm::vec4(x_initial, y_initial, z_initial, 1.0f);

bool moveForward = false;
bool moveBackward = false;
bool moveRight = false;
bool moveLeft = false;
bool isFreeCamOn = false;
bool isColliding = false;

// Variável que controla o tipo de projeção utilizada: perspectiva ou ortográfica.
bool g_UsePerspectiveProjection = true;

// Variável que controla se o texto informativo será mostrado na tela.
bool g_ShowInfoText = false;

// Variáveis para a bounding box da câmera
float camera_width = 0.5f;
float camera_height = 0.5f;
float camera_depth = 0.5f;

glm::vec3 camera_bbox_aux = glm::vec3(camera_width, camera_height, camera_depth);

// Variáveis para controle de movimento do pacman
float pacmanDistance = 0.5f;

glm::vec3 pacman_freecam_size = glm::vec3(0.1f, 0.1f, 0.1f);
glm::vec3 pacman_lookat_size = glm::vec3(0.3f, 0.3f, 0.3f);
glm::vec3 pacman_size = pacman_lookat_size;

glm::vec4 pacman_movement = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
glm::vec4 pacman_offset = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);

// Base de um cubo unitário centrado na origem usado como base para as bounding boxes das paredes do labirinto:
glm::vec4 baseCorners[8] = {
    glm::vec4(-0.5f, -0.5f, -0.5f, 1.0f),
    glm::vec4(-0.5f, -0.5f, 0.5f, 1.0f),
    glm::vec4(-0.5f, 0.5f, -0.5f, 1.0f),
    glm::vec4(-0.5f, 0.5f, 0.5f, 1.0f),
    glm::vec4(0.5f, -0.5f, -0.5f, 1.0f),
    glm::vec4(0.5f, -0.5f, 0.5f, 1.0f),
    glm::vec4(0.5f, 0.5f, -0.5f, 1.0f),
    glm::vec4(0.5f, 0.5f, 0.5f, 1.0f)};

const float PACMAN_SPEED = 3.0f;

bool movePacmanForward = false;
bool movePacmanBackward = false;
bool movePacmanRight = false;
bool movePacmanLeft = false;

float pacman_initial_rotation = 3.14159f / 2;
float pacman_rotation = pacman_initial_rotation;

float g_PacmanTheta = 0.0f;
float g_PacmanPhi = 0.0f;
float g_PacmanDistance = -1.0f;

float r_initial_pacman = g_PacmanDistance;
float x_initial_pacman = r_initial_pacman * cos(g_PacmanPhi) * sin(g_PacmanTheta);
float y_initial_pacman = r_initial_pacman;
float z_initial_pacman = r_initial_pacman * cos(g_PacmanPhi) * cos(g_PacmanTheta);

glm::vec4 pacman_position_initial = glm::vec4(x_initial_pacman, y_initial_pacman, z_initial_pacman, 1.0f);
glm::vec4 pacman_position_c = pacman_position_initial;