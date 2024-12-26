#pragma once

// Based on http://hamelot.io/visualization/opengl-text-without-any-external-libraries/
//   and on https://github.com/rougier/freetype-gl

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

#include "globals/globals.hpp"
#include "utils/utils.h"
#include "external/dejavufont.h"

GLuint CreateGpuProgram(GLuint vertex_shader_id, GLuint fragment_shader_id); // Função definida em main.cpp

const GLchar* const textvertexshader_source = ""
"#version 330\n"
"layout (location = 0) in vec4 position;\n"
"out vec2 texCoords;\n"
"void main()\n"
"{\n"
    "gl_Position = vec4(position.xy, 0, 1);\n"
    "texCoords = position.zw;\n"
"}\n"
"\0";

const GLchar* const textfragmentshader_source = ""
"#version 330\n"
"uniform sampler2D tex;\n"
"in vec2 texCoords;\n"
"out vec4 fragColor;\n"
"void main()\n"
"{\n"
    "fragColor = vec4(0, 0, 0, texture(tex, texCoords).r);\n"
"}\n"
"\0";

GLuint textVAO;
GLuint textVBO;
GLuint textprogram_id;
GLuint texttexture_id;

void TextRendering_LoadShader(const GLchar* const shader_string, GLuint shader_id)
{
    // Define o código do shader, contido na string "shader_string"
    glShaderSource(shader_id, 1, &shader_string, NULL);

    // Compila o código do shader (em tempo de execução)
    glCompileShader(shader_id);

    // Verificamos se ocorreu algum erro ou "warning" durante a compilação
    GLint compiled_ok;
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &compiled_ok);

    GLint log_length = 0;
    glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &log_length);

    // Alocamos memória para guardar o log de compilação.
    // A chamada "new" em C++ é equivalente ao "malloc()" do C.
    GLchar* log = new GLchar[log_length];
    glGetShaderInfoLog(shader_id, log_length, &log_length, log);

    // Imprime no terminal qualquer erro ou "warning" de compilação
    if ( log_length != 0 )
    {
        std::string  output;

        if ( !compiled_ok )
        {
            output += "ERROR: OpenGL compilation failed.\n";
            output += "== Start of compilation log\n";
            output += log;
            output += "== End of compilation log\n";
        }
        else
        {
            output += "ERROR: OpenGL compilation failed.\n";
            output += "== Start of compilation log\n";
            output += log;
            output += "== End of compilation log\n";
        }

        fprintf(stderr, "%s", output.c_str());
    }

    // A chamada "delete" em C++ é equivalente ao "free()" do C
    delete [] log;
}

void TextRendering_Init()
{
    GLuint sampler;

    glGenBuffers(1, &textVBO);
    glGenVertexArrays(1, &textVAO);
    glGenTextures(1, &texttexture_id);
    glGenSamplers(1, &sampler);
    glSamplerParameteri(sampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glSamplerParameteri(sampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glCheckError();

    GLuint textvertexshader_id = glCreateShader(GL_VERTEX_SHADER);
    TextRendering_LoadShader(textvertexshader_source, textvertexshader_id);
    glCheckError();

    GLuint textfragmentshader_id = glCreateShader(GL_FRAGMENT_SHADER);
    TextRendering_LoadShader(textfragmentshader_source, textfragmentshader_id);
    glCheckError();

    textprogram_id = CreateGpuProgram(textvertexshader_id, textfragmentshader_id);
    glLinkProgram(textprogram_id);
    glCheckError();

    GLuint texttex_uniform;
    texttex_uniform = glGetUniformLocation(textprogram_id, "tex");
    glCheckError();

    GLuint textureunit = 31;
    glActiveTexture(GL_TEXTURE0 + textureunit);
    glBindTexture(GL_TEXTURE_2D, texttexture_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, dejavufont.tex_width, dejavufont.tex_height, 0, GL_RED, GL_UNSIGNED_BYTE, dejavufont.tex_data);
    glBindSampler(textureunit, sampler);
    glCheckError();

    glBindVertexArray(textVAO);

    glBindBuffer(GL_ARRAY_BUFFER, textVBO);
    glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(float), NULL, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);
    glCheckError();

    glUseProgram(textprogram_id);
    glUniform1i(texttex_uniform, textureunit);
    glUseProgram(0);
    glCheckError();

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glCheckError();
}

float textscale = 1.5f;

void TextRendering_PrintString(GLFWwindow* window, const std::string &str, float x, float y, float scale = 1.0f)
{
    scale *= textscale;
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    float sx = scale / width;
    float sy = scale / height;

    for (size_t i = 0; i < str.size(); i++)
    {
        // Find the glyph for the character we are looking for
        texture_glyph_t *glyph = 0;
        for (size_t j = 0; j < dejavufont.glyphs_count; ++j)
        {
            if (dejavufont.glyphs[j].codepoint == (uint32_t)str[i])
            {
                glyph = &dejavufont.glyphs[j];
                break;
            }
        }
        if (!glyph) {
            continue;
        }
        x += glyph->kerning[0].kerning;
        float x0 = (float) (x + glyph->offset_x * sx);
        float y0 = (float) (y + glyph->offset_y * sy);
        float x1 = (float) (x0 + glyph->width * sx);
        float y1 = (float) (y0 - glyph->height * sy);

        float s0 = glyph->s0 - 0.5f/dejavufont.tex_width;
        float t0 = glyph->t0 - 0.5f/dejavufont.tex_height;
        float s1 = glyph->s1 - 0.5f/dejavufont.tex_width;
        float t1 = glyph->t1 - 0.5f/dejavufont.tex_height;

        struct {float x, y, s, t;} data[6] = {
            { x0, y0, s0, t0 },
            { x0, y1, s0, t1 },
            { x1, y1, s1, t1 },
            { x0, y0, s0, t0 },
            { x1, y1, s1, t1 },
            { x1, y0, s1, t0 }
        };

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glDepthFunc(GL_ALWAYS);
        glBindBuffer(GL_ARRAY_BUFFER, textVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, 24 * sizeof(float), data);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glUseProgram(textprogram_id);
        glBindVertexArray(textVAO);

        glDrawArrays(GL_TRIANGLES, 0, 6);

        glBindVertexArray(0);
        glUseProgram(0);
        glDepthFunc(GL_LESS);

        glDisable(GL_BLEND);

        x += (glyph->advance_x * sx);
    }
}

float TextRendering_LineHeight(GLFWwindow* window)
{
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    return dejavufont.height / height * textscale;
}

float TextRendering_CharWidth(GLFWwindow* window)
{
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    return dejavufont.glyphs[32].advance_x / width * textscale;
}

void TextRendering_PrintMatrix(GLFWwindow* window, glm::mat4 M, float x, float y, float scale = 1.0f)
{
    char buffer[40];
    float lineheight = TextRendering_LineHeight(window) * scale;

    snprintf(buffer, 40, "[%+0.2f %+0.2f %+0.2f %+0.2f]", M[0][0], M[1][0], M[2][0], M[3][0]);
    TextRendering_PrintString(window, buffer, x, y, scale);
    snprintf(buffer, 40, "[%+0.2f %+0.2f %+0.2f %+0.2f]", M[0][1], M[1][1], M[2][1], M[3][1]);
    TextRendering_PrintString(window, buffer, x, y - lineheight, scale);
    snprintf(buffer, 40, "[%+0.2f %+0.2f %+0.2f %+0.2f]", M[0][2], M[1][2], M[2][2], M[3][2]);
    TextRendering_PrintString(window, buffer, x, y - 2*lineheight, scale);
    snprintf(buffer, 40, "[%+0.2f %+0.2f %+0.2f %+0.2f]", M[0][3], M[1][3], M[2][3], M[3][3]);
    TextRendering_PrintString(window, buffer, x, y - 3*lineheight, scale);
}

void TextRendering_PrintVector(GLFWwindow* window, glm::vec4 v, float x, float y, float scale = 1.0f)
{
    char buffer[10];
    float lineheight = TextRendering_LineHeight(window) * scale;

    snprintf(buffer, 10, "[%+0.2f]", v.x);
    TextRendering_PrintString(window, buffer, x, y, scale);
    snprintf(buffer, 10, "[%+0.2f]", v.y);
    TextRendering_PrintString(window, buffer, x, y - lineheight, scale);
    snprintf(buffer, 10, "[%+0.2f]", v.z);
    TextRendering_PrintString(window, buffer, x, y - 2*lineheight, scale);
    snprintf(buffer, 10, "[%+0.2f]", v.w);
    TextRendering_PrintString(window, buffer, x, y - 3*lineheight, scale);
}

void TextRendering_PrintMatrixVectorProduct(GLFWwindow* window, glm::mat4 M, glm::vec4 v, float x, float y, float scale = 1.0f)
{
    char buffer[70];
    float lineheight = TextRendering_LineHeight(window) * scale;

    auto r = M*v;
    snprintf(buffer, 70, "[%+0.2f %+0.2f %+0.2f %+0.2f][%+0.2f]     [%+0.2f]\n", M[0][0], M[1][0], M[2][0], M[3][0], v[0], r[0]);
    TextRendering_PrintString(window, buffer, x, y, scale);
    snprintf(buffer, 70, "[%+0.2f %+0.2f %+0.2f %+0.2f][%+0.2f]     [%+0.2f]\n", M[0][1], M[1][1], M[2][1], M[3][1], v[1], r[1]);
    TextRendering_PrintString(window, buffer, x, y - lineheight, scale);
    snprintf(buffer, 70, "[%+0.2f %+0.2f %+0.2f %+0.2f][%+0.2f] --> [%+0.2f]\n", M[0][2], M[1][2], M[2][2], M[3][2], v[2], r[2]);
    TextRendering_PrintString(window, buffer, x, y - 2*lineheight, scale);
    snprintf(buffer, 70, "[%+0.2f %+0.2f %+0.2f %+0.2f][%+0.2f]     [%+0.2f]\n", M[0][3], M[1][3], M[2][3], M[3][3], v[3], r[3]);
    TextRendering_PrintString(window, buffer, x, y - 3*lineheight, scale);
}

void TextRendering_PrintMatrixVectorProductMoreDigits(GLFWwindow* window, glm::mat4 M, glm::vec4 v, float x, float y, float scale = 1.0f)
{
    char buffer[70];
    float lineheight = TextRendering_LineHeight(window) * scale;

    auto r = M*v;
    snprintf(buffer, 70, "[%5.1f %5.1f %5.1f %5.1f][%5.2f]     [%+6.1f]\n", M[0][0], M[1][0], M[2][0], M[3][0], v[0], r[0]);
    TextRendering_PrintString(window, buffer, x, y, scale);
    snprintf(buffer, 70, "[%5.1f %5.1f %5.1f %5.1f][%5.2f]     [%+6.1f]\n", M[0][1], M[1][1], M[2][1], M[3][1], v[1], r[1]);
    TextRendering_PrintString(window, buffer, x, y - lineheight, scale);
    snprintf(buffer, 70, "[%5.1f %5.1f %5.1f %5.1f][%5.2f] --> [%+6.1f]\n", M[0][2], M[1][2], M[2][2], M[3][2], v[2], r[2]);
    TextRendering_PrintString(window, buffer, x, y - 2*lineheight, scale);
    snprintf(buffer, 70, "[%5.1f %5.1f %5.1f %5.1f][%5.2f]     [%+6.1f]\n", M[0][3], M[1][3], M[2][3], M[3][3], v[3], r[3]);
    TextRendering_PrintString(window, buffer, x, y - 3*lineheight, scale);
}

void TextRendering_PrintMatrixVectorProductDivW(GLFWwindow* window, glm::mat4 M, glm::vec4 v, float x, float y, float scale = 1.0f)
{
    auto r = M*v;
    auto w = r[3];

    char buffer[90];
    float lineheight = TextRendering_LineHeight(window) * scale;

    snprintf(buffer, 90, "[%+0.2f %+0.2f %+0.2f %+0.2f][%+0.2f]     [%+0.2f]        [%+0.2f]\n", M[0][0], M[1][0], M[2][0], M[3][0], v[0], r[0], r[0]/w);
    TextRendering_PrintString(window, buffer, x, y, scale);
    snprintf(buffer, 90, "[%+0.2f %+0.2f %+0.2f %+0.2f][%+0.2f]     [%+0.2f] div. w [%+0.2f]\n", M[0][1], M[1][1], M[2][1], M[3][1], v[1], r[1], r[1]/w);
    TextRendering_PrintString(window, buffer, x, y - lineheight, scale);
    snprintf(buffer, 90, "[%+0.2f %+0.2f %+0.2f %+0.2f][%+0.2f] --> [%+0.2f] -----> [%+0.2f]\n", M[0][2], M[1][2], M[2][2], M[3][2], v[2], r[2], r[2]/w);
    TextRendering_PrintString(window, buffer, x, y - 2*lineheight, scale);
    snprintf(buffer, 90, "[%+0.2f %+0.2f %+0.2f %+0.2f][%+0.2f]     [%+0.2f]        [%+0.2f]\n", M[0][3], M[1][3], M[2][3], M[3][3], v[3], r[3], r[3]/w);
    TextRendering_PrintString(window, buffer, x, y - 3*lineheight, scale);
}

// Escrevemos na tela os ângulos de Euler definidos nas variáveis globais
// g_AngleX, g_AngleY, e g_AngleZ.
void TextRendering_ShowEulerAngles(GLFWwindow *window)
{
    if (!g_ShowInfoText)
        return;

    float pad = TextRendering_LineHeight(window);

    char buffer[80];
    snprintf(buffer, 80, "Euler Angles rotation matrix = Z(%.2f)*Y(%.2f)*X(%.2f)\n", g_AngleZ, g_AngleY, g_AngleX);

    TextRendering_PrintString(window, buffer, -1.0f + pad / 10, -1.0f + 2 * pad / 10, 1.0f);
}

// Escrevemos na tela qual matriz de projeção está sendo utilizada.
void TextRendering_ShowProjection(GLFWwindow *window)
{
    if (!g_ShowInfoText)
        return;

    float lineheight = TextRendering_LineHeight(window);
    float charwidth = TextRendering_CharWidth(window);

    if (g_UsePerspectiveProjection)
        TextRendering_PrintString(window, "Perspective", 1.0f - 13 * charwidth, -1.0f + 2 * lineheight / 10, 1.0f);
    else
        TextRendering_PrintString(window, "Orthographic", 1.0f - 13 * charwidth, -1.0f + 2 * lineheight / 10, 1.0f);
}

// Escrevemos na tela o número de quadros renderizados por segundo (frames per
// second).
void TextRendering_ShowFramesPerSecond(GLFWwindow *window)
{
    if (!g_ShowInfoText)
        return;

    // Variáveis estáticas (static) mantém seus valores entre chamadas
    // subsequentes da função!
    static float old_seconds = (float)glfwGetTime();
    static int ellapsed_frames = 0;
    static char buffer[20] = "?? fps";
    static int numchars = 7;

    ellapsed_frames += 1;

    // Recuperamos o número de segundos que passou desde a execução do programa
    float seconds = (float)glfwGetTime();

    // Número de segundos desde o último cálculo do fps
    float ellapsed_seconds = seconds - old_seconds;

    if (ellapsed_seconds > 1.0f)
    {
        numchars = snprintf(buffer, 20, "%.2f fps", ellapsed_frames / ellapsed_seconds);

        old_seconds = seconds;
        ellapsed_frames = 0;
    }

    float lineheight = TextRendering_LineHeight(window);
    float charwidth = TextRendering_CharWidth(window);

    TextRendering_PrintString(window, buffer, 1.0f - (numchars + 1) * charwidth, 1.0f - lineheight, 1.0f);
}
