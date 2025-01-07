// Headers das bibliotecas OpenGL
#include <external/glad/glad.h>  // Criação de contexto OpenGL 3.3
#include <external/GLFW/glfw3.h> // Criação de janelas do sistema operacional

// Headers da biblioteca GLM: criação de matrizes e vetores.
#include <external/glm/mat4x4.hpp>
#include <external/glm/vec4.hpp>
#include <external/glm/gtc/type_ptr.hpp>

#include "objects/graphics.hpp"
#include "objects/objects.hpp"
#include "globals/globals.hpp"
#include "matrices.h"

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

std::vector<Wall> instanciateWalls() {
    std::vector<Wall> walls;
    int objectIdCounter = 0;
    const float FLOOR_Y = -1.0f;
    const float MAX_X = 10.0f, MIN_X = -10.0f;
    const float MAX_Z = 10.0f, MIN_Z = -10.0f;

    auto createWall = [&objectIdCounter](float tx, float ty, float tz, float sx, float sy, float sz, 
                                         const std::string& texture, int labyrinth) -> Wall {
        return {Matrix_Translate(tx, ty, tz) * Matrix_Scale(sx, sy, sz), 
                objectIdCounter++, labyrinth, texture, g_VirtualScene};
    };

    std::vector<std::tuple<float, float, float, float, float, float, std::string, int>> baseWalls = {
        {3.5f, FLOOR_Y, 2.9f, 0.6f, 0.5f, 0.2f, "p22", LABYRINTH_2},
        {3.0f, FLOOR_Y, 4.3f, 0.4f, 0.5f, 0.2f, "p22", LABYRINTH_2},
        {5.7f, FLOOR_Y, 6.5f, 0.2f, 0.5f, 0.4f, "p2", LABYRINTH_2},
        {3.3f, FLOOR_Y, 6.3f, 0.2f, 0.5f, 0.25f, "p2", LABYRINTH_2},
        {4.5f, FLOOR_Y, 5.0f, 0.2f, 0.5f, 0.25f, "p2", LABYRINTH_2},
        {2.2f, FLOOR_Y, 7.0f, 0.3f, 0.5f, 0.2f, "p22", LABYRINTH_2},
        {7.0f, FLOOR_Y, 4.0f, 0.2f, 0.5f, 0.3f, "p2", LABYRINTH_2},
        {6.5f, FLOOR_Y, 6.8f, 0.2f, 0.5f, 0.2f, "p22", LABYRINTH_2},
        {6.4f, FLOOR_Y, 4.0f, 0.1f, 0.5f, 0.2f, "p22", LABYRINTH_2},
        {4.5f, FLOOR_Y, 2.0f, 0.2f, 0.5f, 0.2f, "p2", LABYRINTH_2},

        {0.0f, FLOOR_Y, 7.0f, 0.2f, 0.5f, 0.1f, "p2", LABYRINTH_2},
        {0.0f, FLOOR_Y, 5.5f, 0.6f, 0.5f, 0.2f, "p22", LABYRINTH_2},
        {0.0f, FLOOR_Y, 4.0f, 0.2f, 0.5f, 0.4f, "p2", LABYRINTH_2},
        {0.0f, FLOOR_Y, 5.5f, 0.6f, 0.5f, 0.2f, "p22", LABYRINTH_2},
        {0.0f, FLOOR_Y, 7.0f, 0.2f, 0.5f, 0.1f, "p2", LABYRINTH_2},

        {6.4f, FLOOR_Y, 0.0f, 0.2f, 0.5f, 0.5f, "p2", LABYRINTH_2},
        {7.0f, FLOOR_Y, 0.0f, 0.2f, 0.5f, 0.2f, "p22", LABYRINTH_2}
    };

    std::vector<std::tuple<float, float, float, float, float, float, std::string, int>> centerWalls = {
        {0.0f, FLOOR_Y, 1.0f, 0.4f, 0.5f, 0.3f, "p3", LABYRINTH_3},
        {2.1f, FLOOR_Y, -0.2f, 0.3f, 0.5f, 0.2f, "p33", LABYRINTH_3},
        {-2.1f, FLOOR_Y, -0.2f, 0.3f, 0.5f, 0.2f, "p33", LABYRINTH_3},
        {1.5f, FLOOR_Y, -1.1f, 0.125f, 0.5f, 0.3f, "p3", LABYRINTH_3},
        {-1.5f, FLOOR_Y, -1.1f, 0.125f, 0.5f, 0.3f, "p3", LABYRINTH_3}
    };

    auto addMirroredWalls = [&](float tx, float ty, float tz, float sx, float sy, float sz, 
                                 const std::string& texture, int labyrinth) {
        
        walls.push_back(createWall(tx, ty, tz, sx, sy, sz, texture, labyrinth));
        
        if (tx != 0) walls.push_back(createWall(-tx, ty, tz, sx, sy, sz, texture, labyrinth));
        if (tz != 0) walls.push_back(createWall(tx, ty, -tz, sx, sy, sz, texture, labyrinth));
        if (tx != 0 && tz != 0) walls.push_back(createWall(-tx, ty, -tz, sx, sy, sz, texture, labyrinth));
    };

    auto addCenterWalls = [&](float tx, float ty, float tz, float sx, float sy, float sz, 
                                 const std::string& texture, int labyrinth) {
        
        walls.push_back(createWall(tx, ty, tz, sx, sy, sz, texture, labyrinth));
    };

    for (const auto& [tx, ty, tz, sx, sy, sz, texture, labyrinth] : baseWalls) {
        if (tx <= MAX_X && tx >= MIN_X && tz <= MAX_Z && tz >= MIN_Z) {
            addMirroredWalls(tx, ty, tz, sx, sy, sz, texture, labyrinth);
        }
    }

    for (const auto& [tx, ty, tz, sx, sy, sz, texture, labyrinth] : centerWalls) {
        if (tx <= MAX_X && tx >= MIN_X && tz <= MAX_Z && tz >= MIN_Z) {
            addCenterWalls(tx, ty, tz, sx, sy, sz, texture, labyrinth);
        }
    }

    return walls;
}

void checkWallsCollision(std::vector<Wall>& walls, Sphere pacman_sphere, std::vector<glm::vec4>& all_collision_directions)
{
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
}
