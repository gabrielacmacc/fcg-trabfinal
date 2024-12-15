#include <glad/glad.h>
#include <GLFW/glfw3.h>

extern GLuint g_NumLoadedTextures;

// Variáveis que definem um programa de GPU (shaders). Veja função LoadShadersFromFiles().
extern GLuint g_GpuProgramID;
extern GLint g_model_uniform;
extern GLint g_view_uniform;
extern GLint g_projection_uniform;
extern GLint g_object_id_uniform;
extern GLint g_bbox_min_uniform;
extern GLint g_bbox_max_uniform;