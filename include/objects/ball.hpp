// Headers das bibliotecas OpenGL
#include <external/glad/glad.h>  // Criação de contexto OpenGL 3.3
#include <external/GLFW/glfw3.h> // Criação de janelas do sistema operacional

// Headers da biblioteca GLM: criação de matrizes e vetores.
#include <external/glm/mat4x4.hpp>
#include <external/glm/vec4.hpp>
#include <external/glm/gtc/type_ptr.hpp>

#include "objects/objects.hpp"
#include "globals/globals.hpp"
#include "collisions/collisions.hpp"
#include "matrices.h"

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

void checkLittleBallsCollision(std::vector<Ball> &balls, Sphere pacman_sphere, int &eaten_ball_count)
{
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
}
