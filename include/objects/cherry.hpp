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

void checkCherriesCollision(std::vector<Cherry> &cherries, Sphere pacman_sphere)
{
    std::vector<int> remove_cherry_indexes = {};

    int cherry_index = 0;
    for (Cherry &cherry : cherries)
    {
        bool ate = checkSphereToSphereCollision(pacman_sphere, cherry.cherry_sphere);
        if (ate)
        {
            remove_cherry_indexes.push_back(cherry_index);
            shouldBoostSpeed = true;
            shouldStopGhost = true;
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
}
