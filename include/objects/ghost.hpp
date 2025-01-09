#pragma once

// Headers das bibliotecas OpenGL
#include <external/glad/glad.h>  // Criação de contexto OpenGL 3.3
#include <external/GLFW/glfw3.h> // Criação de janelas do sistema operacional

// Headers da biblioteca GLM: criação de matrizes e vetores.
#include <external/glm/mat4x4.hpp>
#include <external/glm/vec4.hpp>
#include <external/glm/gtc/type_ptr.hpp>

#include "objects/objects.hpp"
#include "globals/globals.hpp"
#include "matrices.h"

enum class Direction
{
    FORWARD = 0,
    BACKWARD = 1,
    RIGHT = 2,
    LEFT = 3,
    NONE = 4
};

enum GhostType
{
    FIRST = 0,
    SECOND = 1
};

class Ghost
{
public:
    // Atributos:
    float radius;
    glm::mat4 modelMatrix;
    int objectType;
    std::string objectName;
    Direction direction;
    glm::vec4 current_position;
    glm::vec4 initial_position;
    glm::vec4 final_position;
    float rotation;
    float radius_bbox;

    // Métodos:

    // Construtor
    Ghost(int objectType, std::string objectName, glm::vec4 initial_position, glm::vec4 final_position, float radius)
        : objectType(objectType), objectName(objectName), initial_position(initial_position), final_position(final_position), radius(radius)
    {
        this->direction = Direction::NONE;
        this->current_position = initial_position;
        this->rotation = -INITIAL_ROTATION;
        this->radius_bbox = radius;
    }

    // inicializador vazio apenas para o início
    Ghost() {}

    void render()
    {
        modelMatrix = Matrix_Translate(current_position.x, current_position.y, current_position.z) * Matrix_Rotate_Y(rotation) * Matrix_Scale(radius, radius, radius);
        glUniformMatrix4fv(g_model_uniform, 1, GL_FALSE, glm::value_ptr(modelMatrix));
        glUniform1i(g_object_id_uniform, objectType);
        DrawVirtualObject(objectName.c_str());
    }

    void move(float elapsedTime)
    {
        freeze_ghosts_countdown = std::max(0.0f, freeze_ghosts_countdown - 0.01f);
        if (game_over)
            return;

        if (current_position.x == initial_position.x && current_position.z != initial_position.z)
        {
            direction = Direction::BACKWARD;
        }
        else if (current_position.x == final_position.x && current_position.z != final_position.z)
        {
            direction = Direction::FORWARD;
        }
        else if (current_position.x == initial_position.x && current_position.z == initial_position.z)
        {
            direction = Direction::LEFT;
        }
        else if (current_position.x == final_position.x && current_position.z == final_position.z)
        {
            direction = Direction::RIGHT;
        }

        if (freeze_ghosts_countdown > 0.0f)
        {
            return;
        }

        switch (direction)
        {
        case Direction::FORWARD:
            current_position.z = std::max(current_position.z - GHOST_SPEED * elapsedTime, final_position.z);
            rotation = 3.14159f;
            break;
        case Direction::BACKWARD:
            current_position.z = std::min(current_position.z + GHOST_SPEED * elapsedTime, initial_position.z);
            rotation = 0.0f;
            break;
        case Direction::RIGHT:
            current_position.x = std::min(current_position.x + GHOST_SPEED * elapsedTime, initial_position.x);
            rotation = 3.14159f / 2;
            break;
        case Direction::LEFT:
            current_position.x = std::max(current_position.x - GHOST_SPEED * elapsedTime, final_position.x);
            rotation = -3.14159f / 2;
            break;
        case Direction::NONE:
            break;
        }
    }

    bool collided(Sphere pacman)
    {
        if (freeze_ghosts_countdown > 0.0f) // Se os fantasmas estão parados, a colisão não conta
        {
            return false;
        }
        else
        {
            if (isFreeCamOn)
            {
                radius_bbox = 0.55;
            }
            Sphere ghost_b_sphere = Sphere{current_position, radius_bbox};
            return checkSphereToSphereCollision(ghost_b_sphere, pacman);
        }
    }
};

Ghost instanciateGhost(GhostType type)
{
    switch (type)
    {
    case FIRST:
        return {GHOST, "ghost", glm::vec4(8.5f, -1.4f, 8.5f, 0.0f), glm::vec4(-8.5f, -1.4f, -8.5f, 0.0f), ghost_size};

    case SECOND:
        return {GHOST2, "ghost", glm::vec4(3.2, -1.4f, 2.0f, 0.0f), glm::vec4(-3.35, -1.4f, -2.0f, 0.0f), ghost_size};

    default:
        return {GHOST, "ghost", glm::vec4(8.5f, -1.4f, 8.5f, 0.0f), glm::vec4(-8.5f, -1.4f, -8.5f, 0.0f), ghost_size};
    }
}
