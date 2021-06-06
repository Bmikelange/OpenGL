#ifndef _SHADER_H
#define _SHADER_H
#include <cstdio>
#include <cstring>

#include "glcore.h"
#include "window.h"

#include "program.h"
#include "uniforms.h"

#include "texture.h"
#include "mesh.h"
#include "wavefront.h"

#include "vec.h"
#include "mat.h"
#include "orbiter.h"

#include "text.h"
#include "widgets.h"
#include "glcore.h"

#include "draw.h"


struct Filename
{
    char path[1024];
    
    Filename( ) { path[0]= 0; }
    Filename( const char *_filename ) { strcpy(path, _filename); }
    operator const char *( ) { return path; }
};

class shader
{
public:
    void init();
    void reloade_program(GLuint & program);
    void edraw(Mesh & mesh,Transform  T,Orbiter  & camera,GLuint  & texture,Point luxPosition,Point Direction,GLuint & Program);
    void edrawdef(Mesh & mesh,Transform  T,Orbiter  & camera,GLuint  & texture,Point luxPosition,Point Direction,GLuint & Program);
    void edraw(Mesh & mesh,Transform  T,Orbiter  & camera,GLuint  & texture,Point luxPosition,Point Direction,GLuint & Program,Materials m);
    void edrawdef2(Transform T,Orbiter & camera,GLuint & texture,GLuint & texturCM,GLuint & Program2);
    void edrawCubeMap(Orbiter & camera,GLuint & texture,GLuint & Program, Transform dir);
    void edrawInstancied(Mesh &mesh,Orbiter & camera,GLuint & texture,GLuint & Program, int indicefin,float * T);
    void quit();
    void lux(int indice,int x, int y,int z, int r, int g, int b);
    GLuint & getProgram(int i);
    int foudreControle;
    int nbindice=0;
    float time=0.0;
    void genBuffer();
    void tel_opengl(int & w,int & h);
    void init_vao();
    unsigned int gBuffer;
    float lpos[156];
    float lcol[156];
protected :
    Filename program_filename;
    GLuint program[5];
    Widgets widgets;

    // affichage des erreurs
    std::string program_log;
    int program_area;
    bool program_failed;
    Filename mesh_filename;
    Mesh mesh;
    DrawParam gl;
    GLuint gPosition, gNormal,gAlbedoSpec,gDepth;
    GLuint Quadvao,Quadvbo;
    GLuint Quadvao2;
    GLuint m_vao;


    std::vector<Filename> texture_filenames;
    std::vector<GLuint> textures;
};
#endif
