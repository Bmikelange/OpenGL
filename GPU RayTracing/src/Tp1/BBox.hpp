#pragma once

#include "GPU.hpp"

 struct BBox
 {
     glsl::vec3 pmin, pmax;
     
     BBox( ) : pmin(), pmax() {}
     
     BBox( const Point& p ) : pmin(p), pmax(p) {}
     BBox( const Point& _pmin, const Point& _pmax) : pmin(_pmin), pmax(_pmax) {}
     BBox( const BBox& box ) : pmin(box.pmin), pmax(box.pmax) {}
     
     BBox& insert( const Point& p ) { pmin= min(pmin.returnPoint(), p); pmax= max(pmax.returnPoint(), p); return *this; }
     BBox& insert( const BBox& box ) { pmin= min(pmin.returnPoint(), box.pmin.returnPoint()); pmax= max(pmax.returnPoint(), box.pmax.returnPoint()); return *this; }
     
     float centroid( const int axis ) const { return (pmin.returnPoint()(axis) + pmax.returnPoint()(axis)) / 2; }
 };