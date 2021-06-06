
//! \file tuto_rayons.cpp

#include <vector>
#include <cfloat>
#include <chrono>
#include <cstdlib>
#include <time.h>
#include <fstream>

#include "vec.h"
#include "mat.h"
#include "color.h"
#include "image.h"
#include "image_io.h"
#include "image_hdr.h"
#include "orbiter.h"
#include "mesh.h"
#include "wavefront.h"
#include <omp.h>
#include <limits>


 int nbRebond=8;
 int nbSample=16;

struct Ray
{
    Point o;            // origine
    Vector d;           // direction
    
    Ray( const Point& _o, const Point& _e ) :  o(_o), d(Vector(_o, _e)) {}
};


struct Hit
{
    float t;            // p(t)= o + td, position du point d'intersection sur le rayon
    float u, v;         // p(u, v), position du point d'intersection sur le triangle
    int triangle_id;    // indice du triangle dans le mesh
    
    Hit( ) : t(FLT_MAX), u(), v(), triangle_id(-1) {}
    Hit( const float _t, const float _u, const float _v, const int _id ) : t(_t), u(_u), v(_v), triangle_id(_id) {}
    operator bool ( ) { return (triangle_id != -1); }
};

struct Triangle
{
    Point p;            // sommet a du triangle
    Vector e1, e2;      // aretes ab, ac du triangle
    int id;
    
    Triangle( const TriangleData& data, const int _id ) : p(data.a), e1(Vector(data.a, data.b)), e2(Vector(data.a, data.c)), id(_id) {}
    
    /* calcule l'intersection ray/triangle
        cf "fast, minimum storage ray-triangle intersection" 
        
        renvoie faux s'il n'y a pas d'intersection valide (une intersection peut exister mais peut ne pas se trouver dans l'intervalle [0 tmax] du rayon.)
        renvoie vrai + les coordonnees barycentriques (u, v) du point d'intersection + sa position le long du rayon (t).
        convention barycentrique : p(u, v)= (1 - u - v) * a + u * b + v * c
    */
    Hit intersect( const Ray &ray, const float tmax ) const
    {
        Vector pvec= cross(ray.d, e2);
        float det= dot(e1, pvec);
        
        float inv_det= 1 / det;
        Vector tvec(p, ray.o);
        
        float u= dot(tvec, pvec) * inv_det;
        if(u < 0 || u > 1) return Hit();
        
        Vector qvec= cross(tvec, e1);
        float v= dot(ray.d, qvec) * inv_det;
        if(v < 0 || u + v > 1) return Hit();
        
        float t= dot(e2, qvec) * inv_det;
        if(t > tmax || t < 0) return Hit();
        
        return Hit(t, u, v, id);           // p(u, v)= (1 - u - v) * a + u * b + v * c
    }
};

struct World
 {
     World( const Vector& _n ) : n(_n) 
     {
         if(n.z < -0.9999999f)
         {
             t= Vector(0, -1, 0);
             b= Vector(-1, 0, 0);
         }
         else
         {
             float a= 1.f / (1.f + n.z);
             float d= -n.x * n.y * a;
             t= Vector(1.f - n.x * n.x * a, d, -n.x);
             b= Vector(d, 1.f - n.y * n.y * a, -n.y);
         }
     }
     
     Vector operator( ) ( const Vector& local )  const
     {
         return local.x * t + local.y * b + local.z * n;
     }

     Vector inverse( const Vector& global ) const { return Vector(dot(global, t), dot(global, b), dot(global, n)); }
     
     Vector t;
     Vector b;
     Vector n;
 };

Vector normal( const Mesh& mesh, const Hit& hit )
{
    // recuperer le triangle complet dans le mesh
    const TriangleData& data= mesh.triangle(hit.triangle_id);
    // interpoler la normale avec les coordonn�es barycentriques du point d'intersection
    float w= 1 - hit.u - hit.v;
    Vector n= w * Vector(data.na) + hit.u * Vector(data.nb) + hit.v * Vector(data.nc);
    return normalize(n);
}

Vector normal( const Mesh& mesh, int i ,int u, int v)
{
    // recuperer le triangle complet dans le mesh
    const TriangleData& data= mesh.triangle(i);
    // interpoler la normale avec les coordonn�es barycentriques du point d'intersection
    float w= 1 - u - v;
    Vector n= w * Vector(data.na) + u * Vector(data.nb) + v * Vector(data.nc);
    return normalize(n);
}

std::vector<Point> Fibonnaci(int N)
{
    std::vector<Point> p;
    int pi=3.14159265359;
    for(int i=0;i<N;i++)
    {
        double cost=1-(2*i+1)/(2*N);
        double sint=sqrt(1-cost*cost);
        double phiM=(sqrt(5)+1)/2;
        double phim=2*pi*int(i/phiM);
        p.push_back(Point(cos(phim)*sint,sin(phim)*sint,cost));
    }
    return p;
}

Color diffuse_color( const Mesh& mesh, const Hit& hit )
{
    const Material& material= mesh.triangle_material(hit.triangle_id);
    return material.diffuse;
}


struct Source
{
    Point s;
    Color emission;
    Vector normal;
};

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

Color Color_direct(std::vector<Source> sources,Hit hit,Ray ray,Mesh mesh,std::vector<Triangle> triangles)
{
    Color color(0,0,0);
    for(unsigned int i=0;i<sources.size();i++)
    {
        Point s= sources[i].s;
        Color emission= sources[i].emission;
        
        Point p= ray.o + hit.t * ray.d;
        Vector pn= normal(mesh, hit);
        Vector l= Vector(p, s);
        
        
        float v= 1;
        Ray shadow_ray(p + 0.001f * pn, s);
        for(int i= 0; i < int(triangles.size()); i++)
        {
            if(triangles[i].intersect(shadow_ray, 1 - .001f))
            {
                v= 0;
                break; 
            }
        }
        
        Vector pn2=sources[i].normal;
        Vector l2= Vector(s, p);
        float cos_theta= std::max(0.f,dot(pn, normalize(l)));
        float cos_thetas= std::max(0.f,dot(pn2, normalize(l2)));
        Color fr= diffuse_color(mesh, hit) / M_PI;

        color=color +  v * emission * fr * ((cos_theta*cos_thetas) / length2(l));
    }
    return color/(float)sources.size();
}

Color Color_direct2(std::vector<Source> sources,Hit hit,Ray ray,Mesh mesh,std::vector<Triangle> triangles)
{
    Color color(0,0,0);
    for(unsigned int i=0;i<sources.size();i++)
    {
        Point s= sources[i].s;
        Color emission= sources[i].emission;
        
        Point p= ray.o + hit.t * ray.d;
        Vector pn= normal(mesh, hit);
        Vector l= Vector(p, s);
        
        
        float v= 1;
        Ray shadow_ray(p + 0.001f * pn, s);
        for(int i= 0; i < int(triangles.size()); i++)
        {
            if(triangles[i].intersect(shadow_ray, 1 - .001f))
            {
                v= 0;
                break; 
            }
        }
        Color fr= diffuse_color(mesh, hit) / M_PI;

        color=color +  v * emission * fr;
    }
    return color/(float)sources.size();
}

Color Color_indirect(Hit hit,Ray ray,Mesh mesh,std::vector<Triangle> triangles)
{
    Color color(0,0,0);

    Point p= ray.o + hit.t*ray.d;
    float pdf = 2*M_PI;
    Vector pn=normal(mesh,hit);
    std::vector<Point> direct;
    for(int j=0;j<nbRebond;j++)
    {
        Point p1=aleatory();
        direct.push_back(p1);
    }

    World transform(pn);

    Point origine=p+std::max(std::max(p.x,p.y),p.z)*std::numeric_limits<float>::epsilon()*(pn);

    for(int j=0;j<nbRebond;j++)
    {
        Point extremite= (Point)((Vector)p+transform((Vector)direct[j]));
        Ray newRay(origine,extremite);

        Hit newHit;
        for(int i= 0; i < int(triangles.size()); i++)
        {
            if(Hit h= triangles[i].intersect(newRay, newHit.t))
                if(i != hit.triangle_id)
                    newHit= h;
        }
        if(newHit)
        {
            Point q=newRay.o+newHit.t*newRay.d;
            Vector pn2= normal(mesh, newHit);
            std::vector<Source> sources;
            Source so;
            so.s=q;
            so.emission= diffuse_color(mesh,newHit);
            so.normal=pn2;
            Vector l= Vector(p, q);
            float cos_theta= std::max(0.f,dot(pn, normalize(l)));
            Color fr= diffuse_color(mesh, newHit);
            sources.push_back(so);
            color=color+pdf*Color_direct2(sources,hit,ray,mesh,triangles)*fr*cos_theta;
        }
    }
    return color/(float)nbRebond;   
}

int main( const int argc, const char **argv )
{
    srand( time( NULL ) );
    const char *mesh_filename= "data/cornell2.obj";
    if(argc > 1)
        mesh_filename= argv[1];
        
    const char *orbiter_filename= "data/cornell_orbiter.txt";
    if(argc > 2)
        orbiter_filename= argv[2];
    
    Orbiter camera;
    if(camera.read_orbiter(orbiter_filename) < 0)
        return 1;

    Mesh mesh= read_mesh(mesh_filename);
    
    // recupere les triangles
    std::vector<Triangle> triangles;
    {
        int n= mesh.triangle_count();
        for(int i= 0; i < n; i++)
            triangles.emplace_back(mesh.triangle(i), i);
    }
    
    // recupere les sources
    std::vector<Source> sources;
    {
        int n= mesh.triangle_count();
        for(int i= 0; i < n; i++)
        {
            const Material& material= mesh.triangle_material(i);
            if(material.emission.r + material.emission.g + material.emission.b > 0)
            {
                //utiliser le centre du triangle comme source de lumi�re
                const TriangleData& data= mesh.triangle(i);
                for(int j=0;j<nbSample;j++)
                {
                    float x=rand()%100/100.f;
                    float y=rand()%100/100.f;
                    square2triangle(x,y);
                    Point V=x*Point(data.a)+y*Point(data.b)+ (1-x-y)*Point(data.c);
                    Vector n=normal(mesh,i,x,y);
                    sources.push_back( { V, material.emission,n } );
                } 
            }
        }
        
        printf("%d sources\n", int(sources.size()));
        assert(sources.size() > 0);
    }
    
    Image image(1024, 768);

    // recupere les transformations
    camera.projection(image.width(), image.height(), 45);
    Transform model= Identity();
    Transform view= camera.view();
    Transform projection= camera.projection();
    Transform viewport= camera.viewport();
    Transform inv= Inverse(viewport * projection * view * model);
    
auto start= std::chrono::high_resolution_clock::now();
    
    // c'est parti, parcours tous les pixels de l'image
    #pragma omp parallel for
    for(int y= 0; y < image.height(); y++)
    for(int x= 0; x < image.width(); x++)
    {
        // generer le rayon
        Point origine= inv(Point(x + .5f, y + .5f, 0));
        Point extremite= inv(Point(x + .5f, y + .5f, 1));
        Ray ray(origine, extremite);
        
        // calculer les intersections avec tous les triangles
        Hit hit;
        for(int i= 0; i < int(triangles.size()); i++)
        {
            if(Hit h= triangles[i].intersect(ray, hit.t))
                // ne conserve que l'intersection la plus proche de l'origine du rayon
                hit= h;
        }
        if(hit)
        {
            // position et emission de la source de lumiere
            Color color=Color_direct(sources,hit,ray,mesh,triangles);
            color = color + Color_indirect(hit,ray,mesh,triangles);
            image(x, y)= Color(pow(color.r,1.0/2.2),pow(color.g,1.0/2.2),pow(color.b,1.0/2.2), 1);
        }
    }
auto stop= std::chrono::high_resolution_clock::now();
int cpu= std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();
printf("%dms\n", cpu);
    
    write_image(image, "render.png");
    write_image_hdr(image, "shadow.hdr");
    return 0;
}
