#pragma once

 #include "Triangle.hpp"

 
 struct triangle_less1
 {
     int axis;
     float cut;
     
     triangle_less1( const int _axis, const float _cut ) : axis(_axis), cut(_cut) {}
     
     bool operator() ( const Triangle& triangle ) const
     {
         // re-construit l'englobant du triangle
         BBox bounds= triangle.bounds();
         return bounds.centroid(axis) < cut;
     }
 };