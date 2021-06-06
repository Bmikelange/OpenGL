#pragma once

namespace glsl 
{
    template < typename T >
    struct alignas(8) gvec2
    {
        alignas(4) T x, y;
        
        gvec2( ) {}
        gvec2( const vec2& v ) : x(v.x), y(v.y) {}
        vec2 returnVec2() const{return vec2(x,y);}
    };
    
    typedef gvec2<float> vec2;
    typedef gvec2<int> ivec2;
    typedef gvec2<unsigned int> uvec2;
    typedef gvec2<int> bvec2;
    
    template < typename T >
    struct alignas(16) gvec3
    {
        alignas(4) T x, y, z;
        
        gvec3( ) {}
        gvec3( const vec3& v ) : x(v.x), y(v.y), z(v.z) {}
        gvec3( const Point& v ) : x(v.x), y(v.y), z(v.z) {}
        gvec3( const Vector& v ) : x(v.x), y(v.y), z(v.z) {}
        Point returnPoint() const{return Point(x,y,z);}
        Vector returnVector() const{return Vector(x,y,z);}
        vec3 returnVec3() const{return vec3(x,y,z);}
    };
    
    typedef gvec3<float> vec3;
    typedef gvec3<int> ivec3;
    typedef gvec3<unsigned int> uvec3;
    typedef gvec3<int> bvec3;
    
    template < typename T >
    struct alignas(16) gvec4
    {
        alignas(4) T x, y, z, w;
        
        gvec4( ) {}
        gvec4( const vec4& v ) : x(v.x), y(v.y), z(v.z), w(v.w) {}
        vec4 returnVec4() const{return vec4(x,y,z,w);}
    };
    
    typedef gvec4<float> vec4;
    typedef gvec4<int> ivec4;
    typedef gvec4<unsigned int> uvec4;
    typedef gvec4<int> bvec4;
}


struct RNG
{
    unsigned int x;
    unsigned int x0;

    RNG( const unsigned int seed ) : x(seed), x0(seed) {}

    // glibc
    static const unsigned int a= 1103515245;
    static const unsigned int b= 12345;
    static const unsigned int m= 1u << 31;
   
    float sample( )    				// renvoie un reel aleatoire dans [0 1]
    {
        x= (a*x + b) % m;
        return float(x) / float(m);
    }

    unsigned int index( const size_t i )    	// prepare la generation du terme i
    {
        unsigned int cur_mul= a;
        unsigned int cur_add= b;
        unsigned int acc_mul= 1u;
        unsigned int acc_add= 0u;
   
        size_t delta= i;
        while(delta)
        {
            if(delta & 1u)
            {
                acc_mul= acc_mul * cur_mul;
                acc_add= acc_add * cur_mul + cur_add;
            }
           
            cur_add= cur_mul * cur_add + cur_add;
            cur_mul= cur_mul * cur_mul;
            delta= delta >> 1u;
        }
       
        x= acc_mul * x0 + acc_add;
	return x;
    }
};