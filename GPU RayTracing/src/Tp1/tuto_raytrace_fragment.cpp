
#include <cfloat>
#include <cmath>
#include <algorithm>

#include "app_time.h"

#include "vec.h"
#include "color.h"
#include "mat.h"

#include "mesh.h"
#include "wavefront.h"

#include "program.h"
#include "uniforms.h"

#include "orbiter.h"

#include "bvh.hpp"
#include "BVh-Cousu.hpp"

void displayTriangles(const BVHC &bvh) {
    std::cout << "Root : " << bvh.root << std::endl;
    for (int i = 0; i < (int)bvh.nodes.size(); ++i) {
        const auto &node = bvh.nodes[i];
        std::cout << (node.left >= 0 ? "Node " : "Leaf ") << i << " : ";
        std::cout << (node.left >= 0 ? "Left " : "Triangle ")
                  << (node.left >= 0 ? node.left : (node.left + 1) * -1);
        std::cout << " Next " << node.next;
        std::cout << " pad " << node.pad << std::endl;
        ;
    }
}
// cf tuto_storage

struct RT : public AppTime
{
    // constructeur : donner les dimensions de l'image, et eventuellement la version d'openGL.
    RT( const char *filename ) : AppTime(1024, 640) 
    {
        m_mesh= read_mesh(filename);
    }

    void bindVao()
    {
        float Quad[24]={-1.0f,1.0f,0.0f,1.0f,-1.0f,-1.0f,0.0f,0.0f,1.0f,-1.0f,1.0f,0.0f,
            -1.0f,1.0f,0.0f,1.0f,1.0f,-1.0f,1.0f,0.0f,1.0f,1.0f,1.0f,1.0f};
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
    }

    void  square2triangle(float & x, float & y)
    {
        if ( y > x ) 
        {
            x *= 0.5f;
            y  -= x;
        } else {
            y *= 0.5f;
            x  -= y;
        }
    }
    
    int init( )
    {
        if(m_mesh == Mesh::error())
            return -1;
        
        Point pmin, pmax;
        m_mesh.bounds(pmin, pmax);
        m_camera.lookat(pmin, pmax);
        
        // 

        struct normale
        {
            glsl::vec3 na;
            glsl::vec3 nb;
            glsl::vec3 nc;	
        };

        struct Sources
        {
            glsl::vec3 Emission;
            glsl::vec3 Information;
        };

        struct Colors
        {
            glsl::vec3 c;
            glsl::vec3 e;
        };
        
        std::vector<Colors> datac;
        datac.reserve(m_mesh.triangle_count());
        std::vector<normale> datan;
        datan.reserve(m_mesh.triangle_count());
        std::vector<Sources> datas;


        /*######### BVH BEGIN ##########*/
        std::vector<Triangle> databvh;
        databvh.reserve(m_mesh.triangle_count());
        for(int i= 0; i < m_mesh.triangle_count(); i++)
        {
            TriangleData t= m_mesh.triangle(i);
            databvh.push_back( Triangle(t,i));
        }
        BBox box=BBox(pmin,pmax);
        bvh.build(databvh,box);
        /*######### BVH END ##########*/
        //displayTriangles(bvh);
        for(unsigned int i= 0; i < bvh.triangles.size(); i++)
        {
            int index=bvh.triangles[i].id;
            const Material& material= m_mesh.triangle_material(index);
            Color c1=material.emission;
            if(c1.r != 0 || c1.g !=0 || c1.b !=0)
            {
                datas.push_back({Point(c1.r,c1.g,c1.b),Point(i,0,0)});
            }
            Color c=material.diffuse;
            datac.push_back({Point(c.r,c.g,c.b),Point(c1.r,c1.g,c1.b)});
            const TriangleData& dat= m_mesh.triangle(index);
            datan.push_back({dat.na,dat.nb,dat.nc});
        }
        sizeS=datas.size();

        glGenBuffers(1, &m_buffer);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_buffer);
        glBufferData(GL_SHADER_STORAGE_BUFFER, bvh.triangles.size() * sizeof(Triangle), bvh.triangles.data(), GL_STATIC_READ);

        glGenBuffers(1, &m_node_buffer);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_node_buffer);
        glBufferData(GL_SHADER_STORAGE_BUFFER, bvh.nodes.size() * sizeof(NodeC), bvh.nodes.data(), GL_STATIC_READ);

        glGenBuffers(1, &m_color_buffer);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_color_buffer);
        glBufferData(GL_SHADER_STORAGE_BUFFER, datac.size() * sizeof(Colors), datac.data(), GL_STATIC_READ);

        glGenBuffers(1, &m_normale_buffer);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_normale_buffer);
        glBufferData(GL_SHADER_STORAGE_BUFFER, datan.size() * sizeof(normale), datan.data(), GL_STATIC_READ);

        glGenBuffers(1, &m_source_buffer);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_source_buffer);
        glBufferData(GL_SHADER_STORAGE_BUFFER, datas.size() *sizeof(Sources), datas.data(), GL_STATIC_READ);

        m_texture=gentext();

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);

        //
        m_program= read_program("src/Tp1/my_raytrace.glsl");
        program_print_errors(m_program);
         m_program2= read_program("src/Tp1/defered2.glsl");
        program_print_errors(m_program2);


        // associe l'uniform buffer a l'entree 0
        GLint index= glGetUniformBlockIndex(m_program, "triangleData");
        glUniformBlockBinding(m_program, index, 0);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glBindTexture(GL_TEXTURE_2D, 0);

        bindVao();
        
        return 0;
    }

    GLuint gentext()
    {
        GLuint text;
        glGenTextures(1, &text);
        glBindTexture(GL_TEXTURE_2D, text);
        glTexImage2D(GL_TEXTURE_2D, 0,
            GL_RGBA32F, window_width(), window_height(), 0,
            GL_RGBA, GL_FLOAT, nullptr);
        return text;
    }

    Point aleatory()
    {
        float r1=(rand()%10000/10000.f);
        float r2=(rand()%10000/10000.f);
        float phi=2*M_PI*r1;
        float teta=std::acos(r2);
        float x=std::cos(2*M_PI*r1)*std::sqrt(1-r2*r2);
        float y=std::sin(2*M_PI*r1)*std::sqrt(1-r2*r2);
        float z=r2;
        return Point(x,y,z);

    }
    
    int quit( )
    {
        release_program(m_program);
        glDeleteTextures(1, &m_texture);
        glDeleteBuffers(1, &m_buffer);
        return 0;
    }
    
    int render( )
    {
        static int r=1;
        a.x=(float)(rand()%10000)/10000;
        a.y=(float)(rand()%10000)/10000;
        b.x=(float)(rand()%10000)/10000;
        b.y=(float)(rand()%10000)/10000;
        int randS=rand()%sizeS;
        Point ps=aleatory();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        if(key_state('f'))
        {
            clear_key_state('f');
            Point pmin, pmax;
            m_mesh.bounds(pmin, pmax);
            m_camera.lookat(pmin, pmax);        
        }
        
        // deplace la camera
        int mx, my;
        unsigned int mb= SDL_GetRelativeMouseState(&mx, &my);
        if(mb & SDL_BUTTON(1))              // le bouton gauche est enfonce
            m_camera.rotation(mx, my);
        else if(mb & SDL_BUTTON(3))         // le bouton droit est enfonce
            m_camera.move(mx);
        else if(mb & SDL_BUTTON(2))         // le bouton du milieu est enfonce
            m_camera.translation((float) mx / (float) window_width(), (float) my / (float) window_height());
        if(mb & SDL_BUTTON(1) || mb & SDL_BUTTON(3) || mb & SDL_BUTTON(2) )
        {
            Color c(0,0,0,0);
            glClearTexImage(m_texture, 0, GL_RGBA, GL_FLOAT, &c);
            r=1;
        }
            
        
        Transform m=Identity();
        Transform v= m_camera.view();
        Transform p= m_camera.projection(window_width(), window_height(), 45);
        Transform im= m_camera.viewport();//window_width(), window_height());
        Transform Images= im * p * v * m;
        
        glUseProgram(m_program);
        
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_buffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_color_buffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_normale_buffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, m_source_buffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, m_node_buffer);
        glBindImageTexture(0, m_texture, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA32F);
        

        program_uniform(m_program, "image", 0);

        
        program_uniform(m_program, "invMatrix", Images.inverse());
        program_uniform(m_program, "R", (float)r);
        program_uniform(m_program, "A", a);
        program_uniform(m_program, "B", b);
        program_uniform(m_program, "dir", ps);
        program_uniform(m_program, "aleaLux", randS);
        program_uniform(m_program, "bvhroot", bvh.root);
        int nx= window_width() / 8;
        int ny= window_height() / 8;
        glDispatchCompute(nx, ny, 1);
        
        glMemoryBarrier(GL_ALL_BARRIER_BITS);

        
        glBindVertexArray(Quadvao);
        glUseProgram(m_program2);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, m_texture);

        program_use_texture(m_program2, "image", 0, m_texture);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        r++;

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glBindTexture(GL_TEXTURE_2D, 0);
        
        return 1;
    }
    
protected:
    Mesh m_mesh;
    Orbiter m_camera;

    GLuint m_program;
    GLuint m_program2;
    GLuint Quadvao,Quadvbo;
    GLuint m_buffer;
    GLuint m_color_buffer;
    GLuint m_normale_buffer;
    GLuint m_source_buffer;
    GLuint m_node_buffer;
    GLuint m_texture;
    vec2 a;
    vec2 b;
    BVHC bvh;
    unsigned int sizeS;
};

    
int main( int argc, char **argv )
{
    const char *filename= "data/cornell.obj";
    if(argc > 1)
        filename= argv[1];
    
    RT app(filename);
    app.run();
    
    return 0;
}
