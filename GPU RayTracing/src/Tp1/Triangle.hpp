#pragma once

#include <mesh.h>
#include <GPU.hpp>

struct Triangle
 {
     glsl::vec3 p;            // sommet a du triangle
     glsl::vec3 e1, e2;      // aretes ab, ac du triangle
     int id;
     int pad1;
     
     Triangle( const TriangleData& data, const int _id ) : p(data.a), e1(Vector(data.a, data.b)), e2(Vector(data.a, data.c)), id(_id) {}

     
     BBox bounds( ) const 
     {
         BBox box(p.returnPoint());
         return box.insert(p.returnPoint()+e1.returnVector()).insert(p.returnPoint()+e2.returnVector());
     }
 };