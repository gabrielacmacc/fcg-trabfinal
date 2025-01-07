// Headers das bibliotecas OpenGL
#include <external/glad/glad.h>  // Criação de contexto OpenGL 3.3
#include <external/GLFW/glfw3.h> // Criação de janelas do sistema operacional

// Headers da biblioteca GLM: criação de matrizes e vetores.
#include <external/glm/mat4x4.hpp>
#include <external/glm/vec4.hpp>
#include <external/glm/gtc/type_ptr.hpp>

#include "collisions/collisions.hpp"
#include "globals/globals.hpp"
#include "matrices.h"

glm::vec4 calculateBezierPosition(glm::vec4 p1, glm::vec4 p2, glm::vec4 p3, glm::vec4 p4, float t)
{
    float b03 = pow((1 - t), 3);
    float b13 = 3 * t * pow((1 - t), 2);
    float b23 = 3 * pow(t, 2) * (1 - t);
    float b33 = pow(t, 3);

    return b03 * p1 + b13 * p2 + b23 * p3 + b33 * p4;
}

void BoostPacmanSpeed(float& previousTime) {
    static bool speedChanged = false;
    static float elapsedTime = 0.0f;

    float currentTime = (float)glfwGetTime();
    float frameElapsedTime = currentTime - previousTime;
    previousTime = currentTime;

    if (!speedChanged) {
        PACMAN_SPEED = PACMAN_BOOST;
        speedChanged = true;
        elapsedTime = 0.0f;
    }

    elapsedTime += frameElapsedTime;

    if (elapsedTime >= 1.5f) {
        PACMAN_SPEED = PACMAN_ORIGINAL_SPEED;
        speedChanged = false;
        shouldBoostSpeed = false;
    }
}

void MovePacman(glm::vec4 camera_view_unit, glm::vec4 camera_side_view_unit, float elapsedTime, std::vector<glm::vec4> collision_directions)
{
    if (movePacmanBackward)
    {
        pacman_movement = -camera_view_unit * PACMAN_SPEED * elapsedTime;
        pacman_movement = cancelCollisionMovement(pacman_movement, collision_directions);
        pacman_position_c += pacman_movement;

        if (!isFreeCamOn) 
            pacman_rotation = -3.14159f / 2;
    }

    if (movePacmanForward)
    {
        pacman_movement = camera_view_unit * PACMAN_SPEED * elapsedTime;
        pacman_movement = cancelCollisionMovement(pacman_movement, collision_directions);
        pacman_position_c += pacman_movement;

        if (!isFreeCamOn) 
            pacman_rotation = 3.14159f / 2;
    }

    if (movePacmanRight)
    {
        pacman_movement = -camera_side_view_unit * PACMAN_SPEED * elapsedTime;
        pacman_movement = cancelCollisionMovement(pacman_movement, collision_directions);
        pacman_position_c += pacman_movement;

        if (!isFreeCamOn) 
            pacman_rotation = 0.0f;
    }

    if (movePacmanLeft)
    {
        pacman_movement = camera_side_view_unit * PACMAN_SPEED * elapsedTime;
        pacman_movement = cancelCollisionMovement(pacman_movement, collision_directions);
        pacman_position_c += pacman_movement;

        if (!isFreeCamOn) 
            pacman_rotation = 3.14159f;
    }
}
