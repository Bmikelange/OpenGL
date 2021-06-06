#include "shader.h"

#include "draw.h"

#include "program.h"
#include "uniforms.h"
#include <set>
#include <iostream>
#include <vector>


void shader::init()
{
    widgets= create_widgets();
    program[0]=0;
    program[1]=0;
    program[2]=0;
    program_filename= Filename("src/Tp1/shader/defered.glsl");
    reloade_program(program[0]);
    program_filename= Filename("src/Tp1/shader/defered2.glsl");
    reloade_program(program[1]);
    program_filename= Filename("src/Tp1/shader/shader6.glsl");
    reloade_program(program[2]);
    program_filename= Filename("src/Tp1/shader/defered-instancied.glsl");
    reloade_program(program[3]);
    glUseProgram(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glClearColor(0.2f, 0.2f, 0.2f, 1.f);
    glClearDepth(1.f);
    
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glDisable(GL_CULL_FACE);
    
    glDepthFunc(GL_LESS);
    glEnable(GL_DEPTH_TEST);
}

void shader::reloade_program(GLuint & Program )
{
    if(Program == 0)
        Program= read_program(program_filename);
    else
        reload_program(Program, program_filename);
    
    // recupere les erreurs, si necessaire
    program_area= program_format_errors(Program, program_log);
    
    if(program_log.size() > 0)
        printf("[boom]\n%s\n", program_log.c_str());
    
    program_failed= (program_log.size() > 0);
}

void shader::quit()
{
    release_program(program[0]);
    release_program(program[1]);
    release_program(program[2]);
    glDeleteVertexArrays(1, &Quadvao);
    glDeleteVertexArrays(1, &Quadvao2);
}

GLuint & shader::getProgram(int i)
{
    return program[i];
}

static 
 int location( const GLuint program, const char *uniform )
 {
     if(program == 0) 
         return -1;
     
     // recuperer l'identifiant de l'uniform dans le program
     GLint location= glGetUniformLocation(program, uniform);
     if(location < 0)
     {
         char error[1024]= { 0 };
     #ifdef GL_VERSION_4_3
         {
             char label[1024];
             glGetObjectLabel(GL_PROGRAM, program, sizeof(label), NULL, label);
             
             sprintf(error, "uniform( %s %u, '%s' ): not found.", label, program, uniform); 
         }
     #else
         sprintf(error, "uniform( program %u, '%s'): not found.", program, uniform); 
     #endif
         
         static std::set<std::string> log;
         if(log.insert(error).second == true) 
             // pas la peine d'afficher le message 60 fois par seconde...
             printf("%s\n", error); 
         
         return -1; 
     }
     
 #ifndef GK_RELEASE
     // verifier que le program est bien en cours d'utilisation, ou utiliser glProgramUniform, mais c'est gl 4
     GLuint current;
     glGetIntegerv(GL_CURRENT_PROGRAM, (GLint *) &current);
     if(current != program)
     {
         char error[1024]= { 0 };
     #ifdef GL_VERSION_4_3
         {
             char label[1024];
             glGetObjectLabel(GL_PROGRAM, program, sizeof(label), NULL, label);
             char labelc[1024];
             glGetObjectLabel(GL_PROGRAM, current, sizeof(labelc), NULL, labelc);
             
             sprintf(error, "uniform( %s %u, '%s' ): invalid shader program( %s %u )", label, program, uniform, labelc, current); 
         }
     #else
         sprintf(error, "uniform( program %u, '%s' ): invalid shader program( %u )...", program, uniform, current); 
     #endif
         
         printf("%s\n", error);
         glUseProgram(program);
     }
 #endif
     
     return location;
 }

void shader::edraw(Mesh & mesh,Transform  T,Orbiter  & camera,GLuint  & texture,Point luxPosition,Point Direction,GLuint & Program,Materials m)
{
    Transform model= T;
    Transform view= camera.view();
    Transform projection= camera.projection(window_width(), window_height(), 45);
    
    Transform mvp= projection * view * model;
    GLuint vao= mesh.create_buffers(mesh.has_texcoord(), mesh.has_normal(), mesh.has_color(),1);
        
    glBindVertexArray(vao);
    glUseProgram(Program);
    int mvalue=m.materials.size();

    float ms[3*mvalue];
    float md[3*mvalue];
    float mg[3*mvalue];
    for(int i=0;i<3*mvalue;i+=3)
    {
        Material m2=m.material(i/3);
        ms[i]=m2.diffuse.r;
        ms[i+1]=m2.diffuse.g;
        ms[i+2]=m2.diffuse.b;
        md[i]=m2.specular.r;
        md[i+1]=m2.specular.g;
        md[i+2]=m2.specular.b;
        mg[i]=m2.emission.r;
        mg[i+1]=m2.emission.g;
        mg[i+2]=m2.emission.b;
    }
    
    int tab[mesh.triangle_count ()];
    for(int i=0;i<mesh.triangle_count ();i++)
    {
        tab[i]=mesh.triangle_material_index(i);
    }
    GLuint index_buffer;
    program_uniform(Program, "mvpMatrix", mvp);
    glUniform3fv(location(Program,"diffuse"),mvalue,ms);
    glUniform3fv(location(Program,"specular"),mvalue,md);
    glUniform3fv(location(Program,"emission"),mvalue,mg);
    glUniform1iv(location(Program,"valuemat"),mesh.triangle_count (),tab);
    glDrawArrays(GL_TRIANGLES, 0,mesh.vertex_count());
}

void shader::edraw(Mesh &mesh,Transform T,Orbiter & camera,GLuint & texture,Point luxPosition,Point Direction,GLuint & Program)
{
    // recupere les transformations
    Transform model= T;
    Transform view= camera.view();
    Transform projection= camera.projection(window_width(), window_height(), 45);
    
    Transform mvp= projection * view * model;
    
    GLuint vao= mesh.create_buffers(mesh.has_texcoord(), mesh.has_normal(), mesh.has_color(),1);
        
    // configuration minimale du pipeline
    glBindVertexArray(vao);
    glUseProgram(Program);

    // affecte une valeur aux uniforms
    // transformations standards
    program_uniform(Program, "modelMatrix", model);
    program_uniform(Program, "viewInvMatrix", view.inverse());

    
    program_uniform(Program, "mvpMatrix", mvp);
    
    program_uniform(Program,"direction",Point(0,0,0)-Direction);
    program_uniform(Program,"source",luxPosition);

    char uniform[1024];
    sprintf(uniform, "diffuse_color");
    program_use_texture(Program, uniform, 1, texture);
    // go
    glDrawArrays(GL_TRIANGLES, 0,mesh.vertex_count());
}

void shader::genBuffer()
{
    glGenFramebuffers(1, &gBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
}

void shader::tel_opengl(int & w,int & h)
{
    glGenTextures(1, &gPosition);
    glBindTexture(GL_TEXTURE_2D, gPosition);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, w, h, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);
    
    // - normal color buffer
    glGenTextures(1, &gNormal);
    glBindTexture(GL_TEXTURE_2D, gNormal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, w, h, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);

    glGenTextures(1, &gAlbedoSpec);
    glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedoSpec, 0);

    glGenTextures(1, &gDepth);
    glBindTexture(GL_TEXTURE_2D, gDepth);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, w, h, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, gDepth, 0);
    
    unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1,GL_COLOR_ATTACHMENT2};
    glDrawBuffers(3, attachments);
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cerr<<"erreur sur le framesbuffer"<<std::endl;
        exit(1);
    }
    // - tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
}

void shader::init_vao()
{
    float Quad[24]={-1.0f,1.0f,0.0f,1.0f,-1.0f,-1.0f,0.0f,0.0f,1.0f,-1.0f,1.0f,0.0f,
            -1.0f,1.0f,0.0f,1.0f,1.0f,-1.0f,1.0f,0.0f,1.0f,1.0f,1.0f,1.0f
    };
    glGenVertexArrays(1, &Quadvao);
    glBindVertexArray(Quadvao);
    glGenBuffers(1, &Quadvbo);
    glBindBuffer(GL_ARRAY_BUFFER, Quadvbo);
    glBufferData(GL_ARRAY_BUFFER,sizeof(Quad), &Quad, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), 0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)(2*sizeof(float)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    glGenVertexArrays(1, &Quadvao2);
}


void shader::edrawdef(Mesh &mesh,Transform T,Orbiter & camera,GLuint & texture,Point luxPosition,Point Direction,GLuint & Program)
{
    // recupere les transformations
    
    Transform model= T;
    Transform view= camera.view();
    Transform projection= camera.projection(window_width(), window_height(), 45);
    
    Transform mvp= projection * view * model;
    
    GLuint vao= mesh.create_buffers(mesh.has_texcoord(), mesh.has_normal(), mesh.has_color(),1);
        
    // configuration minimale du pipeline
    glBindVertexArray(vao);
    glUseProgram(Program);

    program_use_texture(Program, "texture_diffuse1", 5, texture);
    program_uniform(Program, "modelMatrix", model);
    program_uniform(Program, "mvpMatrix", mvp);

    glDrawArrays(GL_TRIANGLES, 0,mesh.vertex_count());
}

void shader::lux(int indice,int x, int y,int z, int r, int g, int b)
{
    lpos[indice]=x;
    lpos[indice+1]=y;
    lpos[indice+2]=z;
    lcol[indice]=r;
    lcol[indice+1]=g;
    lcol[indice+2]=b;
}

void shader::edrawdef2(Transform T,Orbiter & camera,GLuint & texture,GLuint & textureCM,GLuint & Program2)
{
    
    Transform model= T;
    Transform view= camera.view();
    Transform projection= camera.projection(window_width(), window_height(), 45);

    Transform viewport= camera.viewport();
    Transform viewInv= Inverse(view);

    Transform INV=Inverse(viewport * projection * view);
    
    Transform mvp= projection * view * model;
    Point camera_position= viewInv(Point(0, 0, 0));
        
    // configuration minimale du pipeline
    glBindVertexArray(Quadvao);
    glUseProgram(Program2);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gPosition);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, gNormal);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_CUBE_MAP,  textureCM);
    // - position color buffer

    char uniform3[1024];
    sprintf(uniform3, "gPosition");
    program_use_texture(Program2, uniform3, 0, gPosition);
    char uniform4[1024];
    sprintf(uniform4, "gNormal");
    program_use_texture(Program2, uniform4, 1, gNormal);
    char uniform5[1024];
    sprintf(uniform5, "gAlbedoSpec");
    program_use_texture(Program2, uniform5, 2, gAlbedoSpec);
    program_uniform(Program2, "texture0", int(3));
    glUniform3fv(location(Program2,"lightspos"),52,lpos);
    glUniform3fv(location(Program2,"lightscol"),52,lcol);
    program_uniform(Program2, "viewInvMatrix", view.inverse());

    program_uniform(Program2, "invMatrix", INV);
    program_uniform(Program2, "camera_position", camera_position);
    
    // go
    glDrawArrays(GL_TRIANGLES, 0,6);
}


void shader::edrawCubeMap(Orbiter & camera,GLuint & texture,GLuint & Program, Transform dir)
{

    // recupere les transformations
    Transform model= Identity();
    Transform view= camera.view();
    Transform projection= camera.projection(window_width(), window_height(), 45);
    Transform viewport= camera.viewport();
    Transform viewInv= Inverse(view);

    Transform INV=Inverse(viewport * projection * view);
    Transform mvp= projection * view * model;

    Point camera_position= viewInv(Point(0, 0, 0));
        
    // configuration minimale du pipeline
    glBindVertexArray(Quadvao2);
    glUseProgram(Program);

    // affecte une valeur aux uniforms
    // transformations standards
    program_uniform(Program, "invMatrix", INV);
    program_uniform(Program, "camera_position", camera_position);

    
    glBindTexture(GL_TEXTURE_CUBE_MAP,  texture);
    program_uniform(Program, "texture0", int(0));
    // go
    glDrawArrays(GL_TRIANGLES, 0, 3); 
} 


void shader::edrawInstancied(Mesh &mesh,Orbiter & camera,GLuint & texture,GLuint & Program, int indicefin,float * T)
{
    Transform view= camera.view();
    Transform projection= camera.projection(window_width(), window_height(), 45);
    
    Transform vp= projection * view;

    GLuint vao= mesh.create_buffers(mesh.has_texcoord(), mesh.has_normal(), mesh.has_color(),1);
        
    // configuration minimale du pipeline
    glBindVertexArray(vao);
    glUseProgram(Program);

    program_use_texture(Program, "texture_diffuse1", 5, texture);
    program_uniform(Program, "vpMatrix", vp);

    glUniformMatrix4fv(location(Program,"models"),indicefin,true,T);

    
    // go
    glDrawArraysInstanced(GL_TRIANGLES,0,mesh.vertex_count(),indicefin);
}
