
//! \file tuto7_camera.cpp reprise de tuto7.cpp mais en derivant AppCamera, avec gestion automatique d'une camera.


#include "wavefront.h"
#include "texture.h"
#include "image_io.h"

#include "orbiter.h"
#include "draw.h"        
#include "app_camera.h"        // classe Application a deriver
#include "shader.h"
#include "camera.h"
#include <string>
#include <iostream>




class TP : public AppCamera
{
public:
    // constructeur : donner les dimensions de l'image, et eventuellement la version d'openGL.
    TP( ) : AppCamera(1024, 640) {}
    

     GLuint read_cubemap( const int unit, const char *filename,  const GLenum texel_type = GL_RGBA )
    {
        ImageData image= read_image_data(filename);
        if(image.pixels.empty()) 
            return 0;
        
        int w= image.width / 4;
        int h= image.height / 3;
        assert(w == h);
        
        GLenum data_format;
        GLenum data_type= GL_UNSIGNED_BYTE;
        if(image.channels == 3)
            data_format= GL_RGB;
        else 
            data_format= GL_RGBA;
        
        GLuint texture= 0;
        glGenTextures(1, &texture);
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
    
        struct { int x, y; } faces[]= {
            {0, 1}, 
            {2, 1},
            {1, 2}, 
            {1, 0},  
            {1, 1}, 
            {3, 1}, 
        };
        
        for(int i= 0; i < 6; i++)
        {
            ImageData face= flipX(flipY(copy(image, faces[i].x*w, faces[i].y*h, w, h)));
            
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X +i, 0,
                texel_type, w, h, 0,
                data_format, data_type, face.data());
        }
        
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);    
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);    
        
        glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

        glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
        
        return texture;
    }
    
    // creation des objets de l'application
    int init( )
    {
        // objet dynamique, commence par un element
        for(int i=1;i<=1;i++)
        {
            std::string s="src/Tp1/obj/OBJ/";
            std::string s2="src/Tp1/texture/Textures/";
            std::string t=".obj";
            std::string n=".png";
            std::string m1=s+std::to_string(i)+t;
            std::string m2=s2+std::to_string(i)+n;
            m_objet[i-1]= read_mesh(m1.c_str());
            text[i-1]=read_texture(5, m2.c_str());
            te[i-1]=Translation(Vector(20-2*i,0,2*i));
        }

        m_objet[1]= read_mesh("src/Tp1/obj/OBJ/volcano.obj");
        text[1]=read_texture(0,"src/Tp1/texture/Textures/sol.jpeg");
        te[1]=Translation(300,-450,300)*Scale(800,800,800);

        m_objet[2]= read_mesh("src/Tp1/obj/OBJ/sorciere.obj");
        text[2]=read_texture(0,"src/Tp1/texture/Textures/image.jpeg");
        te[2]=Translation(180,5,90)*Scale(0.2,0.2,0.2);
        cam.lookAt(Point(200,5,110),20,Point(0,0,-90));
        m_view=cam;

        m_objet[3]= read_mesh("src/Tp1/obj/OBJ/manor.obj");
        text[3]=read_texture(0,"src/Tp1/texture/Textures/manor.png");
        te[3]=Translation(0,35,0)*Scale(20,20,20)*RotationY(180);

        m_objet[4]= read_mesh("src/Tp1/obj/OBJ/tombe4.obj");
        text[4]=read_texture(0,"src/Tp1/texture/Textures/tombe2.jpg");
        te[4]=Identity();
        
        m_objet[5]= read_mesh("src/Tp1/obj/OBJ/tombe3.obj");
        text[5]=read_texture(0,"src/Tp1/texture/Textures/tombe2.jpg");
        te[5]=RotationX(-90);

        m_objet[6]= read_mesh("src/Tp1/obj/OBJ/tombe2.obj");
        text[6]=read_texture(0,"src/Tp1/texture/Textures/tombe2.jpg");
        te[6]=RotationX(-90);

        m_objet[7]= read_mesh("src/Tp1/obj/OBJ/tombe1.obj");
        text[7]=read_texture(0,"src/Tp1/texture/Textures/tombe2.jpg");
        te[7]=RotationX(-90);
        
        m_objet[8]= read_mesh("src/Tp1/obj/OBJ/single_pumpkin.obj");
        text[8]=read_texture(0,"src/Tp1/texture/Textures/pumpkin.png");
        te[8]=RotationX(90);

        m_objet[9]= read_mesh("src/Tp1/obj/OBJ/terrain.obj");
        text[9]=read_texture(0,"src/Tp1/texture/Textures/sol.jpeg");
        te[9]=Scale(200,200,200);

        init_hauteur();

        // si l'objet est "gros", il faut regler la camera pour l'observer entierement :
        // recuperer les points extremes de l'objet (son englobant)
        // parametrer la camera de l'application, renvoyee par la fonction camera()
        camera().lookat(Point(0,0,0), 2000);
        
        // etat openGL par defaut
        glClearColor(0.2f, 0.2f, 0.2f, 1.f);        // couleur par defaut de la fenetre
        
        glClearDepth(1.f);                          // profondeur par defaut
        glDepthFunc(GL_LEQUAL);                       // ztest, conserver l'intersection la plus proche de la camera
        glEnable(GL_DEPTH_TEST);                    // activer le ztest
        shad.init();
        shad.genBuffer();
        int h,w;
        SDL_GetWindowSize(m_window,&w,&h);
        shad.tel_opengl(w,h);
        shad.init_vao();
        

        shad.lux(0,0,500,0,0,0,1);
        int j=100;
        for(int i=3;i<153;i+=3)
        {
            Vector p=te[9]((Vector)((m_objet[9].positions())[position[j]]));
            shad.lux(i,p.x,p.y,p.z+1,0.09,0.09,0.85);
            j++;
        }
        shad.lux(153,0,70,0,0,1,0.5);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        cubemapText=read_cubemap( 3, "data/cubemap/testNight2.jpeg");
        //cubemapText=read_cubemap( 3, "tutos/cubemap_debug_cross.png");
        return 0;   // ras, pas d'erreur
    }

    void init_hauteur()
    {
        std::vector<int> v;
        for(int i=0;i<m_objet[9].vertex_count();i++)
        {
            v.push_back(i);
        }
        std::random_shuffle(v.begin(),v.end());
        for(int j=0;j<150;j++)
        {
           if(j<100)
           {
                Transform Rotations=Rotation(Vector(0,1,0),(Vector)(m_objet[9].normals())[v[j]]);
                Transform Translations=Translation(te[9]((Vector)((m_objet[9].positions())[v[j]]))- Rotations(Vector(0,1,0)));
                Tp[j]=Translations*Rotations;
                if(j<10)
                    Tp[j]=Tp[j]*te[4];
                if (j>=10 && j<20)
                    Tp[j]=Tp[j]*te[5];
                if (j>=20 && j<50)
                    Tp[j]=Tp[j]*te[6];
                if (j>=50 && j<100)
                    Tp[j]=Tp[j]*te[7];
           }
           else
           {
               Transform Rotations=Rotation(Vector(0,0,1),(Vector)(m_objet[9].normals())[v[j]]);
               Transform Translations=Translation(te[9]((Vector)((m_objet[9].positions())[v[j]])));
               Tp[j]=Translations*Rotations;
               Tp[j]=Tp[j]*te[8];
           }
           
           position[j]=v[j];
        }
        for(int i=0;i<150;i++)
        {
            for(int k=0;k<4;k++)
            for(int j=0;j<4;j++)
            {
                if (i<10)
                    modelTinFloat.push_back(Tp[i].m[k][j]);
                if (i>=10 && i<20)
                    modelT1inFloat.push_back(Tp[i].m[k][j]);
                if (i>=20 && i<50)
                    modelT2inFloat.push_back(Tp[i].m[k][j]);
                if (i>=50 && i<100)
                    modelT3inFloat.push_back(Tp[i].m[k][j]);
                if (i>=100)
                    modelCitrinFloat.push_back(Tp[i].m[k][j]);
            }
        }
    }
    
    // destruction des objets de l'application
    int quit( )
    {
        for(int i=0;i<10;i++)
        {
            m_objet[i].release();
            glDeleteTextures(1, &text[i]);
        }
        shad.quit();
        return 0;
    }
    
    

    int render( )
    {
        glBindFramebuffer(GL_FRAMEBUFFER, shad.gBuffer);
        glClearColor(0.0, 0.0, 0.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        for(int i=0;i<10;i++)
        {
            if(i==4)
            {
                shad.edrawInstancied(m_objet[i],m_view,text[i],shad.getProgram(3), 10,modelTinFloat.data());
            }
            else if(i==5)
            {
                shad.edrawInstancied(m_objet[i],m_view,text[i],shad.getProgram(3), 10,modelT1inFloat.data());
            }
            else if(i==6)
            {
                shad.edrawInstancied(m_objet[i],m_view,text[i],shad.getProgram(3), 30,modelT2inFloat.data());
            }
            else if(i==7)
            {
                shad.edrawInstancied(m_objet[i],m_view,text[i],shad.getProgram(3), 50,modelT3inFloat.data());
            }
            else if(i==8)
            {
                shad.edrawInstancied(m_objet[i],m_view,text[i],shad.getProgram(3), 50,modelCitrinFloat.data());
            }
            else if(i==2)
            {

            }
            else
            {
                shad.edrawdef(m_objet[i], te[i], m_view,text[i],Point(0,1,0),Point(1,0,0),shad.getProgram(0));
            }
            
        }
        
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        shad.edrawdef2( Identity(), m_view,text[0],cubemapText,shad.getProgram(1));
        
        return 1;
    }

    int update(const float time, const float delta)
    {
        Point dir(0,0,0);
        Point dir2(0,0,0);
        int deltas=delta;
        double vit=delta/75;
        static int times=0;
        if (times<=5000)
        {
            te[2]=Translation(-0.01*deltas,0,0.01*deltas)*te[2];
            dir =Point(0,0,-135);
            dir2 =Point(-20,5,20);
            if(times>4100)
            {
                dir.z=dir.z+(int)((times-4100)/deltas)*3.75*vit;
                dir2.z=dir2.z-(int)((times-4100)/deltas)*(1.66)*vit;
            }
        }
        else if (times>5000 && times<=19000 )
        {
            te[2]=Translation(-0.01*deltas,0,0)*te[2];
            dir =Point(0,0,-90);
            dir2 =Point(-20,5,0);
            if(times>18100)
            {
                dir.y=dir.y-(int)((times-18100)/deltas)*3.75*vit;
                dir2.y=dir2.y+(int)((times-18100)/deltas)*(1.25)*vit;
            }
        }
        else if (times>19000 && times<=21500 )
        {
            te[2]=Translation(-0.01*deltas,0.01*deltas,0)*te[2];
            dir =Point(0,-45,-90);
            dir2 =Point(-20,20,0);
            if(times>20600)
            {
                dir.y=dir.y+(int)((times-20600)/deltas)*3.75*vit;
                dir2.y=dir2.y-(int)((times-20600)/deltas)*(1.25)*vit;
            }
        }
        else if (times>21500 && times<=29000 )
        {
            te[2]=Translation(-0.01*deltas,0,0)*te[2];
            dir =Point(0,0,-90);
            dir2 =Point(-20,5,0);
            if(times>28100)
            {
                dir.y=dir.y+(int)((times-28100)/deltas)*3.75*vit;
                dir2.y=dir2.y-(int)((times-28100)/deltas)*(2.08)*vit;
            }
        }
        else if (times>29000 && times<=31500 )
        {
            te[2]=Translation(-0.01*deltas,-0.01*deltas,0)*te[2];
            dir =Point(0,45,-90);
            dir2 =Point(-20,-20,0);
            if(times>30600)
            {
                dir.y=dir.y-(int)((times-30600)/deltas)*3.75*vit;
                dir.z=dir.z+(int)((times-30600)/deltas)*3.75*vit;
                dir2.x=dir2.x+(int)((times-30600)/deltas)*(0.83)*vit;
                dir2.y=dir2.y+(int)((times-30600)/deltas)*(2.08)*vit;
                dir2.z=dir2.z-(int)((times-30600)/deltas)*(1.66)*vit;
            }
        }
        else if (times>31500 && times<=40000)
        {
            te[2]=Translation(-0.005*deltas,0,-0.01*deltas)*te[2];
            dir =Point(0,0,-45);
            dir2 =Point(-10,5,-20);
            if(times>39100)
            { 
                dir.z=dir.z+(int)((times-39100)/deltas)*2.25*vit;
            }
        }
        else if (times>40000 && times<=47200)
        {
            te[2]=Translation(-0.002*deltas,0,-0.01*deltas)*te[2];
            dir =Point(0,0,-18);
            dir2 =Point(-10,5,-20);
            if(times>46300)
            { 
                dir.z=dir.z+(int)((times-46300)/deltas)*1.5*vit;
                dir2.x=dir2.x+(int)((times-46300)/deltas)*(0.41)*vit;
            }
        }
        else if (times>47200 && times<=49000)
        {
            te[2]=Translation(0,0,-0.01*deltas)*te[2];
            dir =Point(0,0,0);
            dir2 =Point(-5,5,-20);
            if(times>48100)
            {
                dir.y=dir.y-(int)((times-48100)/deltas)*2.08*vit;
                dir.z=dir.z+(int)((times-48100)/deltas)*3.75*vit;
                dir2.x=dir2.x+(int)((times-48100)/deltas)*(2.5)*vit;
                dir2.y=dir2.y+(int)((times-48100)/deltas)*(0.41)*vit;
                dir2.z=dir2.z+(int)((times-48100)/deltas)*(0.83)*vit;
            }
        }
        else if (times>49000 && times<=53000)
        {
            te[2]=Translation(0.01*deltas,0.005*deltas,-0.005*deltas)*te[2];
            dir =Point(0,-25,45);
            dir2 =Point(20,10,-10);
            if(times>52100)
            {
                dir.y=dir.y+(int)((times-52100)/deltas)*2.08*vit;
                dir2.y=dir2.y-(int)((times-52100)/deltas)*(0.41)*vit;
            }
        }
        else if (times>53000 && times<=69000)
        {
            te[2]=Translation(0.01*deltas,0,-0.005*deltas)*te[2];
            dir =Point(0,0,45);
            dir2 =Point(20,5,-10);
            if(times>68100)
            {
                dir.z=dir.z+(int)((times-68100)/deltas)*7.5*vit;
                dir2.z=dir2.z+(int)((times-68100)/deltas)*(1.66)*vit;
            }
        }
        else if (times>69000 && times<=75000)
        {
            te[2]=Translation(0.01*deltas,0,0.005*deltas)*te[2];
            dir =Point(0,0,135);
            dir2 =Point(20,5,10);
            if(times>74100)
            {
                dir.z=dir.z+(int)((times-74100)/deltas)*7.5*vit;
                dir.y=dir.y-(int)((times-74100)/deltas)*3.75*vit;
                dir2.z=dir2.z+(int)((times-74100)/deltas)*(0.83)*vit;
                dir2.x=dir2.x-(int)((times-74100)/deltas)*(3.33)*vit;
                dir2.y=dir2.y+(int)((times-74100)/deltas)*(1.25)*vit;
            }
        }
        else if (times>75000 && times<=77000)
        {
            te[2]=Translation(-0.01*deltas,0.01*deltas,0.01*deltas)*te[2];
            dir =Point(0,-45,225);
            dir2 =Point(-20,20,20);
            if(times>76100)
            {
                dir.y=dir.y+(int)((times-76100)/deltas)*3.75*vit;
                dir2.y=dir2.y-(int)((times-76100)/deltas)*(1.25)*vit;
            }
        }
        else if (times>77000 && times<=82000)
        {
            te[2]=Translation(-0.01*deltas,0,0.01*deltas)*te[2];
            dir =Point(0,0,225);
            dir2 =Point(-20,5,20);
            if(times>81100)
            {
                dir.z=dir.z-(int)((times-81100)/deltas)*3.75*vit;
                dir2.x=dir2.x+(int)((times-81100)/deltas)*(1.66)*vit;
            }
        }
        else
        {
            dir =Point(0,0,180);
            dir2 =Point(0,5,20);
        }

        Point p=te[2](Point(0, 0, 0));
        cam.lookAt(Point(p.x+dir2.x,p.y+dir2.y,p.z+dir2.z),20,dir);
        m_view=cam;
        times+=deltas;
        return 1;
    }

protected:
    Mesh m_objet[10];
    GLuint text[10];
    shader shad;
    Mesh m_cube;
    Transform te[10];
    Transform Tp[150];
    int position[150];
    Camera cam;	
    Orbiter m_view;
    GLuint cubemapText;
    std::vector<float> modelTinFloat;
    std::vector<float> modelT1inFloat;
    std::vector<float> modelT2inFloat;
    std::vector<float> modelT3inFloat;
    std::vector<float> modelCitrinFloat;
};


int main( int argc, char **argv )
{
    // il ne reste plus qu'a creer un objet application et la lancer 
    TP tp;
    tp.run();
    
    return 0;
}
