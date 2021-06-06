#version 430

#ifdef COMPUTE_SHADER

#define M_PI 3.1415926535897932384626433832795

struct Triangle
{
    vec3 a;		// sommet
    vec3 ab;	// arete 1
    vec3 ac;	// arete 2
    int id;
    int pad1;
};

struct normale
{
    vec3 na;
    vec3 nb;	
    vec3 nc;
};

struct Source
{
    vec3 Emission;
    vec3 Information;
};

struct Color
{
    vec3 c;
    vec3 e;
};

struct Node
{
    vec3 pmin;
    int pad1;
    vec3 pmax;
    int pad2;
    int left;   
    int next;  
    int end;    
    int pad;
};

struct Hit
{
    float t;
    float u, v;        
    int id;
};

struct Ray{
    vec3 o,d;
};

struct World
 {   
    vec3 t;
    vec3 b;
    vec3 n;
 };

layout(std430, binding= 0) readonly buffer triangleData
{
    Triangle triangles[];
};

layout(std430, binding= 1) readonly buffer triangleColor
{
    Color col[];
};

layout(std430, binding= 2) readonly buffer triangleNormale
{
    normale n[];
};

layout(std430, binding= 3) readonly buffer triangleSources
{
    Source src[];
};

layout(std430, binding= 4) readonly buffer Nodes
{
    Node nodes[];
};

void initWorld( inout World t, vec3 _n ) 
{
    t.n=_n;
    if(t.n.z < -0.9999999f)
    {
        t.t= vec3(0, -1, 0);
        t.b= vec3(-1, 0, 0);
    }
    else
    {
        float a= 1.f / (1.f + t.n.z);
        float d= -t.n.x * t.n.y * a;
        t.t= vec3(1.f - t.n.x * t.n.x * a, d, -t.n.x);
        t.b= vec3(d, 1.f - t.n.y * t.n.y * a, -t.n.y);
    }
}

vec3  computeWorld( vec3 local,World t ) 
{
    return local.x * t.t + local.y * t.b + local.z * t.n;
}

vec3 inverseWorld( vec3 global , World t ) { return vec3(dot(global, t.t), dot(global, t.b), dot(global, t.n)); }

bool intersect( const Triangle triangle, Ray ray, inout Hit hit, float tmax )
{
    vec3 pvec= cross(ray.d, triangle.ac);
    float det= dot(triangle.ab, pvec);
    float inv_det= 1.0f / det;
    
    vec3 tvec= ray.o - triangle.a;
    float u= dot(tvec, pvec) * inv_det;
    vec3 qvec= cross(tvec, triangle.ab);
    float v= dot(ray.d, qvec) * inv_det;
    
    hit.t= dot(triangle.ac, qvec) * inv_det;
    hit.u= u;
    hit.v= v;
    
    if(any(greaterThan(vec3(u, v, u+v), vec3(1, 1, 1))) || any(lessThan(vec2(u, v), vec2(0, 0))))
        return false;

    return (hit.t < tmax && hit.t > 0);
}

void swap(inout  float xmin,inout float xmax)
{
    float temp=xmax;
    xmax=xmin;
    xmin=temp;
}

bool intersect(Ray ray, float tmax,vec3 pmin,vec3 pmax)
{
    vec3 invd= vec3(1 / ray.d.x, 1 / ray.d.y, 1 / ray.d.z);
    vec3 rmin= pmin;
    vec3 rmax= pmax;
    if(ray.d.x < 0) swap(rmin.x, rmax.x);
    if(ray.d.y < 0) swap(rmin.y, rmax.y);
    if(ray.d.z < 0) swap(rmin.z, rmax.z);
    vec3 dmin= (rmin - ray.o) * invd;
    vec3 dmax= (rmax - ray.o) * invd;

    float tmin= max(dmin.z, max(dmin.y, max(dmin.x, 0.f)));
    float tmax2= min(dmax.z, min(dmax.y, min(dmax.x, tmax)));

    return tmin <= tmax2;
}

bool leaf(Node n)
{
    return n.left < 0;
}


void intersect_bvh(int root,inout Hit hit,inout Ray ray)
{
    float tmax= hit.t;
    int index= root;
    int count=0;
    int indexprec=-100;
    while(index != -1)
    {
        Node node= nodes[index];
        indexprec=index;
        if(leaf(node))
        {
            Hit h;
            if(intersect(triangles[node.pad],ray, h, tmax ))
            {
                tmax= h.t; 
                hit= h; 
                hit.id=node.pad;
            }
            index= node.next;
        }
        else
        {
            if(intersect(ray, tmax,node.pmin,node.pmax))
            {
                index = node.left;
   
            }
            else
                index = node.next;  
        }
        count++;
    }
}

vec3 normals( Hit hit)
{
    float w= 1 - hit.u - hit.v;
    vec3 nu= w * n[hit.id].na + hit.u * n[hit.id].nb + hit.v * n[hit.id].nc;
    return normalize(nu);
}

float length2(vec3 l)
{
    return l.x*l.x+l.y*l.y+l.z*l.z;
}

float random(vec2 uv)
{
    return fract(sin(dot(uv,vec2(12.9898,78.233)))*43758.5453123);
}

vec2 cut_triangle(vec2 A, vec2 B)
{
    float x=random(A);
    float y=random(B);
    if ( y > x ) 
    {
        x *= 0.5f;
        y  -= x;
    } else {
        y *= 0.5f;
        x  -= y;
    }
    return vec2(x,y);
}

vec3 Color_direct(vec3 s,Hit hit,Ray ray,vec3 normal,Source srcs,vec3 ns,int root)
{
    vec3 color=col[hit.id].e;
    
    vec3 p= ray.o + (hit.t-0.001*hit.t) * ray.d;

    vec3 l= s-p;
    vec3 ls= p-s;
    
    Ray ray2; 
    ray2.o=p + 0.001f * normal;
    ray2.d=l;

    Hit hit2=Hit(10000,0,0,-1);
    intersect_bvh(root,hit2,ray2);
    if(hit2.id !=srcs.Information.x && hit2.id !=-1)
    {
        return vec3(0,0,0);
    }
    
    // calculer la lumiere reflechie vers la camera / l'origine du rayon
    float cos_theta= max(dot(normal, normalize(l)),0.0);
    float cos_thetas= max(dot(ns, normalize(ls)),0.0);
    vec3 fr= col[hit.id].c /M_PI ;

    color=color + srcs.Emission * fr * (cos_theta*cos_thetas) / length2(l);
    return color;
}

vec3 Color_direct2(vec3 s,Hit hit,Ray ray,vec3 normal,Source srcs,vec3 ns,int root)
{
    vec3 color=col[hit.id].e;
    
    vec3 p= ray.o + (hit.t-0.001*hit.t) * ray.d;

    vec3 l= s-p;
    vec3 ls= p-s;
    
    Ray ray2; 
    ray2.o=p + 0.001f * normal;
    ray2.d=l;

    Hit hit2=Hit(10000,0,0,-1);
    intersect_bvh(root,hit2,ray2);
    if(hit2.id !=srcs.Information.x && hit2.id !=-1)
    {
        return vec3(0,0,0);
    }
    
    // calculer la lumiere reflechie vers la camera / l'origine du rayon
    vec3 fr= col[hit.id].c /M_PI ;
    float cos_theta= max(dot(normal, normalize(l)),0.0);
    float cos_thetas= max(dot(ns, normalize(ls)),0.0);

    color=color + srcs.Emission* fr *(cos_theta*cos_thetas);

    return color;
}

vec3 Color_indirect(Hit hit,Ray ray,vec3 normal,vec3 dir,int root)
{
    vec3 color=vec3(0,0,0);

    vec3 p= ray.o + hit.t*ray.d;
    float pdf = 2*M_PI;

    World transform;
    initWorld( transform, normal );

    vec3 origine=p+0.1*(normal);

    vec3 extremite= p+1000*computeWorld( dir,transform );

    Ray newRay=Ray(origine,extremite);

    Hit newHit=Hit(100000,0,0,-1);
    intersect_bvh(root,newHit,newRay);
    if(newHit.id != -1)
    {
        vec3  s =newRay.o+newHit.t*newRay.d;
        vec3 pn2= normals(newHit);
        vec3 emission= col[newHit.id].c;
        Source srcs=Source(emission,vec3(newHit.id,0,0));
        vec3 l= s-p;
        float cos_theta= max(dot(normal, normalize(l)),0.0);
        vec3 fr= col[newHit.id].c;
        vec3 col=pdf*Color_direct2(s,hit,ray,normal,srcs,pn2,root)*10*fr*cos_theta;
        color=color +  col;
    }
    return color;  
}

uniform mat4 invMatrix;
uniform float R;
uniform vec2 A;
uniform vec2 B;
uniform vec3 dir;
uniform int n_source;
uniform int aleaLux;
uniform int bvhroot;

layout(binding= 0, rgba32f)  coherent uniform image2D image;

layout( local_size_x= 8, local_size_y= 8 ) in;
void main( )
{
    vec2 position= vec2(gl_GlobalInvocationID.xy);
    vec4 oh= invMatrix * vec4(position, 0, 1);
    vec4 eh= invMatrix * vec4(position, 1, 1);
    Ray ray;
    ray.o= oh.xyz / oh.w;                            
    ray.d= eh.xyz / eh.w; 

    Hit hit=Hit(100000,0,0,-1);
    intersect_bvh(bvhroot,hit,ray);
    if(hit.id==-1)
    {
        imageStore(image, ivec2(gl_GlobalInvocationID.xy), vec4(0,0,1, 1));
    }
    else
    {
        vec4 colorprec=imageLoad(image, ivec2(position));
        vec3 color=vec3(0);
        int id=int(src[aleaLux].Information.x);
        vec2 v= cut_triangle(A,B);
        vec3 b=triangles[id].ab+triangles[id].a;
        vec3 c=triangles[id].ac+triangles[id].a;
        vec3 s=v.x*triangles[id].a+v.y*b+ (1-v.x-v.y)*c;
        Hit h=Hit(1,v.x,v.y,id);
        vec3 ns=normals(h);
        color+=Color_direct(s,hit,ray,normals(hit),src[aleaLux],ns,bvhroot);
        color+=Color_indirect(hit,ray,normals(hit),dir,bvhroot);
        imageStore(image, ivec2(position), (colorprec*R+vec4(color, 1))/(R+1));
    }
}

#endif
