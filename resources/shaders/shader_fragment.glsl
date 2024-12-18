#version 330 core

// Atributos de fragmentos recebidos como entrada ("in") pelo Fragment Shader.
// Neste exemplo, este atributo foi gerado pelo rasterizador como a
// interpolação da posição global e a normal de cada vértice, definidas em
// "shader_vertex.glsl" e "main.cpp".
in vec4 position_world;
in vec4 normal;

// Posição do vértice atual no sistema de coordenadas local do modelo.
in vec4 position_model;

// Coordenadas de textura obtidas do arquivo OBJ (se existirem!)
in vec2 texcoords;

// Matrizes computadas no código C++ e enviadas para a GPU
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

// Identificador que define qual objeto está sendo desenhado no momento
#define SPHERE 0
#define LABYRINTH_1 1
#define LABYRINTH_2 2
#define LABYRINTH_3 3
#define PLANE  4
#define BACKGROUND 5
uniform int object_id;

// Parâmetros da axis-aligned bounding box (AABB) do modelo
uniform vec4 bbox_min;
uniform vec4 bbox_max;

// Variáveis para acesso das imagens de textura
uniform sampler2D FloorTexture;
uniform sampler2D LabTexture;
uniform sampler2D Test1;
uniform sampler2D Test2;

// O valor de saída ("out") de um Fragment Shader é a cor final do fragmento.
out vec4 color;

// Constantes
#define M_PI   3.14159265358979323846
#define M_PI_2 1.57079632679489661923

void main()
{
    // Obtemos a posição da câmera utilizando a inversa da matriz que define o
    // sistema de coordenadas da câmera.
    vec4 origin = vec4(0.0, 0.0, 0.0, 1.0);
    vec4 camera_position = inverse(view) * origin;

    // O fragmento atual é coberto por um ponto que percente à superfície de um
    // dos objetos virtuais da cena. Este ponto, p, possui uma posição no
    // sistema de coordenadas global (World coordinates). Esta posição é obtida
    // através da interpolação, feita pelo rasterizador, da posição de cada
    // vértice.
    vec4 p = position_world;

    // Normal do fragmento atual, interpolada pelo rasterizador a partir das
    // normais de cada vértice.
    vec4 n = normalize(normal);

    // Vetor que define o sentido da câmera em relação ao ponto atual.
    vec4 v = normalize(camera_position - p);

    // Vetor que define o sentido da fonte de luz em relação ao ponto atual.
    vec4 l = v;

    // Vetor que define o sentido da reflexão especular ideal.
    vec4 r = -l + 2 * n * (dot(n, l)); // vetor de reflexão especular ideal

    vec4 h = normalize(l + v);

    // Coordenadas de textura U e V
    float U = 0.0;
    float V = 0.0;

    // Parâmetros que definem as propriedades espectrais da superfície
    float q = 1.0; // Expoente especular para o modelo de iluminação de Phong

    // Refletância difusa
    vec3 Kd = vec3(0.5,0.5,0.5);

    // Refletância especular
    vec3 Ks = vec3(0.5,0.5,0.5);

    // Refletância ambiente
    vec3 Ka = vec3(0.05,0.05,0.05);

    // Espectro da fonte de iluminação
    vec3 I = vec3(1.0, 1.0, 1.0); // espectro da fonte de luz

    // Espectro da luz ambiente
    vec3 Ia = vec3(0.2, 0.2, 0.2); // espectro da luz ambiente

    // Termo difuso utilizando a lei dos cossenos de Lambert
    vec3 lambert_diffuse_term; // termo difuso de Lambert

    // Equação de Iluminação
    float lambert = max(0, dot(n, l));

    // Termo ambiente
    vec3 ambient_term = Ka * Ia; // termo ambiente

    // Termo especular utilizando o modelo de iluminação de Phong
    vec3 phong_specular_term  = Ks * I * max(0, pow(dot(r, v), q)); // termo especular de Phong 

    vec3 blinn_phong_specular_term  = Ks * I * (pow(dot(n, h), q)); // termo especular de Blinn-Phong

    if ( object_id == SPHERE )
    {
        Kd = vec3(0.8, 0.4, 0.08);
        Ks = vec3(0.0, 0.0, 0.0);
        Ka = vec3(0.4, 0.2, 0.04);
        q = 1.0;
        lambert_diffuse_term = Kd * I * lambert;
        color.rgb = lambert_diffuse_term + ambient_term + phong_specular_term;
    }
    else if ( object_id == PLANE )
    {
        U = texcoords.x + 10.0f;
        V = texcoords.y + 10.0f;
        Kd = texture(Test2, vec2(U,V)).rgb;
        color.rgb = Kd;
    }
    else if ( object_id == BACKGROUND )
    {
        vec4 bbox_center = (bbox_min + bbox_max) / 2.0;

        float radius = length(bbox_max - bbox_center);

       vec4 position_sphere = bbox_center + radius * normalize(position_model - bbox_center);
        //vec4 p_vector = position_model - bbox_center;

        //float px = p_vector.x;
        //float py = p_vector.y;
        //float pz = p_vector.z;

        //float theta = atan(px, pz);
        //float rho = length(p_vector);
        //float phi = asin(py/rho);

        float theta = atan(position_sphere.x, position_sphere.z);
        float phi = asin(position_sphere.y / radius);

        U = (theta + M_PI) / (2 * M_PI);
        V = (phi + M_PI_2) / M_PI;

        U *= 3.0f;
        V *= 3.0f;

        Kd = texture(Test2, vec2(U,V)).rgb;
        color.rgb = Kd;
    }
    else if ( object_id == LABYRINTH_1 || object_id == LABYRINTH_2 || object_id == LABYRINTH_3)
    {
        U = texcoords.x;
        V = texcoords.y;

        Kd = texture(LabTexture, vec2(U,V)).rgb;
        lambert_diffuse_term = Kd * I * lambert;
        color.rgb = lambert_diffuse_term;
    }
    else // Objeto desconhecido = preto
    {
        Kd = vec3(0.0, 0.0, 0.0);
        Ks = vec3(0.0, 0.0, 0.0);
        Ka = vec3(0.0, 0.0, 0.0);
        q = 1.0;
        color.rgb = lambert_diffuse_term + ambient_term + phong_specular_term;
    }

    // NOTE: Se você quiser fazer o rendering de objetos transparentes, é
    // necessário:
    // 1) Habilitar a operação de "blending" de OpenGL logo antes de realizar o
    //    desenho dos objetos transparentes, com os comandos abaixo no código C++:
    //      glEnable(GL_BLEND);
    //      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // 2) Realizar o desenho de todos objetos transparentes *após* ter desenhado
    //    todos os objetos opacos; e
    // 3) Realizar o desenho de objetos transparentes ordenados de acordo com
    //    suas distâncias para a câmera (desenhando primeiro objetos
    //    transparentes que estão mais longe da câmera).
    // Alpha default = 1 = 100% opaco = 0% transparente
    color.a = 1;

    // Cor final com correção gamma, considerando monitor sRGB.
    // Veja https://en.wikipedia.org/w/index.php?title=Gamma_correction&oldid=751281772#Windows.2C_Mac.2C_sRGB_and_TV.2Fvideo_standard_gammas
    color.rgb = pow(color.rgb, vec3(1.0,1.0,1.0)/2.2);
} 

