#include <sb7.h>
#include <vmath.h>

#include <iostream>
#include <fstream>

#include <stdlib.h>
#include <vector>

using namespace vmath;
using namespace std;

void MessageCallback( GLenum source,
                      GLenum type,
                      GLuint id,
                      GLenum severity,
                      GLsizei length,
                      const GLchar* message,
                      const void* userParam )
{
    fprintf( stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
             ( type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "" ),
             type, severity, message );
    exit(0);
}

struct Face
{
    int v[3];
    int vt[3];
    int vn[3];
};

class Mesh
{
public:
    std::vector<Face> faces;

};

class Model //Clase que se encarga de cargar un modelo WavefrontOBJ y guardar sus datos
{
private:

public:
    std::vector< unsigned int > indicesVertices, indicesNormales;
    std::vector < vec3 > vertices_temporales,normales_temporales;
    std::vector < vec3 > objVertices, objNormales;

    bool load(const char* path)
    {
        FILE * f = fopen(path, "r");
        if(f==NULL)
        {
            std::cout<<("Error al abrir el archivo\n");
            return false;
        }

        //Interpretación del archivo
        while(true)
        {
            char head[100]; //Primera linea del documento

            int resultado = fscanf(f, "%s", head);
            if (resultado == EOF)
                break;

            if (strcmp(head, "v" ) == 0 ) //Vectores del modelo
            {
                vec3 vertex;
                fscanf(f, "%f %f %f\n", &vertex[0], &vertex[1], &vertex[2] );
                vertices_temporales.push_back(vertex);
            }
            else if(strcmp(head, "vt")==0) //Texturas de vectores del modelo, se ignoran
            {
                std::cerr<<"Linea de textura detectada... ignorando\n";
            }
            else if (strcmp(head, "vn" ) == 0 ) //Normales del modelo
            {
                vec3 normal;
                fscanf(f, "%f %f %f\n", &normal[0], &normal[1], &normal[2] );
                normales_temporales.push_back(normal);
            }
            else if (strcmp(head, "f" ) == 0 ) //Caras de modelo, si no son triangulare se ignoran
            {
                char cara[100];

                unsigned int vertexIndex[3], normalIndex[3], textureIndex[3];
                fscanf(f, "%[^\n]\n", cara);
                if (strstr(cara, "//")) // v//vn
                {
                    int verticesCara = sscanf(cara,"%d//%d %d//%d %d//%d\n",&vertexIndex[0], &normalIndex[0], &vertexIndex[1],
                                              &normalIndex[1], &vertexIndex[2], &normalIndex[2] );
                }
                else
                {
                    int verticesCara = sscanf(cara,"%d/%d/%d %d/%d/%d %d/%d/%d\n",
                                              &vertexIndex[0],&textureIndex[0], &normalIndex[0],
                                              &vertexIndex[1],&textureIndex[1], &normalIndex[1],
                                              &vertexIndex[2],&textureIndex[2], &normalIndex[2] );
                }

                indicesVertices.push_back(vertexIndex[0]);
                indicesVertices.push_back(vertexIndex[1]);
                indicesVertices.push_back(vertexIndex[2]);
                indicesNormales.push_back(normalIndex[0]);
                indicesNormales.push_back(normalIndex[1]);
                indicesNormales.push_back(normalIndex[2]);

            }
            else if ( strcmp( head, "s" ) == 0 )
            {
                std::cerr<<"Linea con formato no compatible... ignorando\n";
            }

        }
        //Asignando los vertices al std::vector de vertices
        for(int i=0; i<indicesVertices.size(); i++ )
        {
            int indiceVertice = indicesVertices[i];
            vec3 vertice = vertices_temporales[ indiceVertice-1 ];
            objVertices.push_back(vertice);
        }
        //Asignando las normales al std::vector de normales
        for(int i=0; i<indicesNormales.size(); i++ )
        {
            int indiceNormal = indicesNormales[i];
            vec3 normal = normales_temporales[ indiceNormal-1 ];
            objNormales.push_back(normal);
        }
        return true; //Exito
    }

};

class WaveFrontOBJ:public sb7::application
{
private:
    GLuint program, vao;
    GLuint position_buffer;
    GLuint normal_buffer;
    GLuint index_buffer;
    GLuint colorLoc;
    GLuint mvpLoc[3];
    vec3 modelPos, cameraPos;
    float pitch,yaw;
    float modelPitch, modelYaw,modelRoll;
    std::vector<vec3> objVertices;
    std::vector<vec3> objNormales;

public:
    void init()
    {
        static const char
        title[] = "WavefrontOBJ Loader v2.0 by Jaime Arancibia";
        sb7::application::init();
        memcpy(info.title, title, sizeof(title));
    }


    virtual void onKey(int key, int action)
    {
        if (key == GLFW_KEY_A && (action == GLFW_PRESS || action == GLFW_REPEAT))
        {
            modelPitch-=2.0; //Rotación en Y
        }
        if (key == GLFW_KEY_D && (action == GLFW_PRESS || action == GLFW_REPEAT))
        {
            modelPitch+=2.0;
        }
        if (key == GLFW_KEY_W && (action == GLFW_PRESS || action == GLFW_REPEAT))
        {
            modelYaw-=2.0; //Rotación en X
        }
        if (key == GLFW_KEY_S && (action == GLFW_PRESS || action == GLFW_REPEAT))
        {
            modelYaw+=2.0;
        }
        if (key == GLFW_KEY_E && (action == GLFW_PRESS || action == GLFW_REPEAT))
        {
            modelRoll-=2.0; //Rotación en Z
        }
        if (key == GLFW_KEY_Q && (action == GLFW_PRESS || action == GLFW_REPEAT))
        {
            modelRoll+=2.0;
        }
        if (key == GLFW_KEY_R && (action == GLFW_PRESS || action == GLFW_REPEAT))
        {
            pitch-=2.0; //Rotación de la camara
        }
        if (key == GLFW_KEY_T && (action == GLFW_PRESS || action == GLFW_REPEAT))
        {
            pitch+=2.0;
        }
        if (key == GLFW_KEY_UP && (action == GLFW_PRESS || action == GLFW_REPEAT))
        {
            modelPos[2]-=0.2; //Movimiento en eje Z modelo
        }
        if (key == GLFW_KEY_DOWN && (action == GLFW_PRESS || action == GLFW_REPEAT))
        {
            modelPos[2]+=0.2; //Movimiento en eje Z modelo
        }
    }

    virtual void
    startup()
    {
        Model obj;

        std::cout<<"Ingrese la ruta del archivo:\n";
        char * path = new char[256];
        std::cin>>(path);

        if(obj.load(path))//en la carpeta build
        {
            objNormales = obj.objNormales;
            objVertices = obj.objVertices;
        }

        // During init, enable debug output
        glEnable              ( GL_DEBUG_OUTPUT );
        glDebugMessageCallback( (GLDEBUGPROC) MessageCallback, 0 );

        static const char *vs_source[] =
        {
            "#version 420 core                                    \n"
            "uniform mat4 model;                                  \n"
            "uniform mat4 view;                                   \n"
            "uniform mat4 projection;                             \n"
            "in vec3 position;				      \n"
            "in vec3 normal;                                      \n"
            "in vec4 color;                                       \n"
            "out vec3 vs_normal;                                  \n"
            "out vec3 vl_diff;                                    \n"
            "out vec3 vs_position;                                \n"
            "out vec4 vs_color;                                   \n"
            "uniform vec3 light_pos = vec3(25);                   \n"
            "void main(void)                                      \n"
            "{                                                    \n"
            "	vec4 pos = model * vec4(position,1.0);                  \n"
            "       vs_normal = mat3(model) * normal;             \n"
            "	vl_diff = light_pos - pos.xyz;                \n"
            "	vs_color = color;                             \n"
            "                                                     \n"
            "       vs_position = pos.xyz * -1;                   \n"
            "       gl_Position = projection * view * pos;        \n"
            "}                                                    \n"
        };

        static const char *fs_source[] =
        {
            "#version 420 core                                    \n"
            "out vec4 fs_color;                                   \n"
            "in vec3 vs_normal;                                   \n"
            "in vec3 vl_diff;                                     \n"
            "in vec3 vs_position;                                 \n"
            "in vec4 vs_color;                                    \n"
            "uniform vec3 diffuse_albedo = vec3(0.9);             \n"
            "uniform vec3 specular_albedo = vec3(0.6);            \n"
            "vec3 ambient = vec3(0.3);            \n"

            "void main(void)                                      \n"
            "{                                                    \n"
            "	vec3 vs_normal = normalize(vs_normal);                                       \n"
            "	vec3 vl_diff = normalize(vl_diff);                                           \n"
            "	vec3 vs_position = normalize(vs_position);                                   \n"
            "   vec3 H = normalize(vl_diff + vs_position);                                   \n"
            "   vec3 diffuse = max(dot(vs_normal, vl_diff), 0.0) * diffuse_albedo;           \n"
            "   vec3 specular = pow(max(dot(vs_normal, H), 0.0), 150) * specular_albedo;     \n"
            "   fs_color = vs_color * vec4(specular + diffuse + ambient, 1.0);                         \n"
            "}\n"
        };


        program = glCreateProgram();
        GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fs, 1, fs_source, NULL);
        glCompileShader(fs);

        GLuint vs = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vs, 1, vs_source, NULL);
        glCompileShader(vs);

        glAttachShader(program, vs);
        glAttachShader(program, fs);

        glLinkProgram(program);
        colorLoc = glGetAttribLocation(program, "color");
        GLuint posLoc = glGetAttribLocation(program, "position");
        GLuint normalLoc = glGetAttribLocation(program, "normal");

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        glGenBuffers(1, &position_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, position_buffer);

        glBufferData(GL_ARRAY_BUFFER, objVertices.size() * sizeof(vec3), &objVertices[0], GL_STATIC_DRAW);
        glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(posLoc);

        glGenBuffers(1, &normal_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, normal_buffer);
        glBufferData(GL_ARRAY_BUFFER, objNormales.size() * sizeof(vec3), &objNormales[0], GL_STATIC_DRAW);
        glVertexAttribPointer(normalLoc, 3, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(normalLoc);

        mvpLoc[0] = glGetUniformLocation(program, "model");
        mvpLoc[1] = glGetUniformLocation(program, "view");
        mvpLoc[2] = glGetUniformLocation(program, "projection");

        modelPos=vec3(0,0,0);
        cameraPos=vec3(0,0,6);
        pitch=0;
        yaw=0;
        modelPitch=0;
        modelYaw=0;
        modelRoll=0;

        glEnable(GL_DEPTH_TEST);
    }

    virtual void
    render(double currentTime)
    {
        static const GLfloat black[] = {0.0f,0.0f,0.0f, 1.0f};
        GLfloat red[] = { 1.0f,0.0f,0.0f, 1.0f };
        GLfloat green[] = { 0.0f,1.0f,0.0f, 1.0f };
        GLfloat blue[] = { 0.0f,0.0f,1.0f, 1.0f };
        GLfloat white[] = { 1.0f,1.0f,1.0f, 1.0f };

        glClearColor(0,0.4f,1.0f,1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(program);
        // Diffuse
        glVertexAttrib4fv(colorLoc,white);

        vec3 cameraFront=vec3(
                             cos(radians(pitch))*sin(radians(yaw)),
                             sin(radians(pitch)),
                             -cos(radians(pitch))*cos(radians(yaw))
                         );
        mat4 view=lookat(cameraPos,
                         cameraPos+cameraFront,
                         vec3(0,1,0));




        //Posición y rotación del modelo
        mat4 model=translate(modelPos)*rotate(modelPitch, 0.0f, 1.0f, 0.0f)
                   *rotate(modelYaw, 1.0f, 0.0f, 0.0f)*rotate(modelRoll, 0.0f, 0.0f, 1.0f);

        mat4 projection=perspective(60,800.0/600.0,1,-1);


        glUniformMatrix4fv(mvpLoc[0], 1, GL_FALSE, model);
        glUniformMatrix4fv(mvpLoc[1], 1, GL_FALSE, view);
        glUniformMatrix4fv(mvpLoc[2], 1, GL_FALSE, projection);
        glUniform3fv(glGetUniformLocation(program, "cameraPos"), 1,cameraPos);
        glDrawArrays(GL_TRIANGLES, 0, objVertices.size());
    }

    virtual void
    shutdown()
    {
        glDeleteVertexArrays(1, &vao);
        glDeleteProgram(program);
        glDeleteBuffers(1, &position_buffer);
        glDeleteBuffers(1, &index_buffer);

    }



};

DECLARE_MAIN(WaveFrontOBJ)
