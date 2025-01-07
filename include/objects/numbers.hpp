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

#include "objects/objects.hpp"
#include "globals/globals.hpp"
#include "matrices.h"

using namespace std;

void renderCount(int current_count, char *count_first_digit, char *count_second_digit, char *count_third_digit)
{
    std::ostringstream oss;
    oss << std::setfill('0') << std::setw(3) << current_count;
    std::string numStr = oss.str();

    snprintf(count_first_digit, 4, "N_%c", numStr[2]);
    snprintf(count_second_digit, 4, "N_%c", numStr[1]);
    snprintf(count_third_digit, 4, "N_%c", numStr[0]);
}
