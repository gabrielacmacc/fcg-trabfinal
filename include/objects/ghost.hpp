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

enum class GhostDirection
{
    FORWARD = 0,
    BACKWARD = 1,
    RIGHT = 2,
    LEFT = 3,
    NONE = 4
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

void MoveGhost(float elapsedTime)
{
    static GhostDirection currentDirection = GhostDirection::NONE;

    if (ghost_position_c.x == ghost_position_initial.x && ghost_position_c.z != ghost_position_initial.z)
    {
        currentDirection = GhostDirection::BACKWARD;
    }
    else if (ghost_position_c.x == ghost_position_final.x && ghost_position_c.z != ghost_position_final.z)
    {
        currentDirection = GhostDirection::FORWARD;
    }
    else if (ghost_position_c.x == ghost_position_initial.x && ghost_position_c.z == ghost_position_initial.z)
    {
        currentDirection = GhostDirection::LEFT;
    }
    else if (ghost_position_c.x == ghost_position_final.x && ghost_position_c.z == ghost_position_final.z)
    {
        currentDirection = GhostDirection::RIGHT;
    }

    if (shouldStopGhost)
    {
        return;
    }

    switch (currentDirection)
    {
    case GhostDirection::FORWARD:
        ghost_position_c.z = std::max(ghost_position_c.z - GHOST_SPEED * elapsedTime, ghost_position_final.z);
        break;
    case GhostDirection::BACKWARD:
        ghost_position_c.z = std::min(ghost_position_c.z + GHOST_SPEED * elapsedTime, ghost_position_initial.z);
        break;
    case GhostDirection::RIGHT:
        ghost_position_c.x = std::min(ghost_position_c.x + GHOST_SPEED * elapsedTime, ghost_position_initial.x);
        break;
    case GhostDirection::LEFT:
        ghost_position_c.x = std::max(ghost_position_c.x - GHOST_SPEED * elapsedTime, ghost_position_final.x);
        break;
    case GhostDirection::NONE:
        break;
    }
}
