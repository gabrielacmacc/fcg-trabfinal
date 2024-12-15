#include <glad/glad.h>
#include <GLFW/glfw3.h>

#ifndef _SHADER_UTILS_H
#define _SHADER_UTILS_H

GLuint LoadShader_Vertex(const char *filename); // Carrega um vertex shader
GLuint LoadShader_Fragment(const char *filename); // Carrega um fragment shader
void LoadShader(const char *filename, GLuint shader_id); // Função utilizada pelas duas acima
void LoadShadersFromFiles(); // Carrega os shaders de vértice e fragmento, criando um programa de GPU
GLuint CreateGpuProgram(GLuint vertex_shader_id, GLuint fragment_shader_id); // Cria um programa de GPU
#endif // _SHADER_UTILS_H