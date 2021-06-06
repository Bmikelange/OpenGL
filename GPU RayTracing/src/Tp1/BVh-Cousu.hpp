#pragma once
#include <Node_cousu.hpp>
#include <vector>
#include <iostream>
#include "Triangle_less.hpp"
#include <vec.h>
#include <algorithm>

struct BVHC
{
    std::vector<NodeC> nodes;
    std::vector<Triangle> triangles;
    int root;

    int build(const std::vector<Triangle>& _triangles,const BBox& _bounds )
    {
        triangles = _triangles;  
        nodes.clear();          
        nodes.reserve(triangles.size());
         
        root= build(_bounds, 0, triangles.size(), -1);
        return root;
    }
    
protected:
    int build( const BBox& bounds, const int begin, const int end, const int next)
    {
        if(end - begin < 2)
        {
            int index= nodes.size();
            nodes.push_back(make_leafC(bounds, begin, next, end));
            return index;
        }
        
        Vector d= Vector(bounds.pmin.returnPoint(), bounds.pmax.returnPoint());
        int axis;
        if(d.x > d.y && d.x > d.z)  
            axis= 0;
        else if(d.y > d.z)          
            axis= 1;
        else                        
            axis= 2;

        float cut= bounds.centroid(axis);
        
        Triangle *pm= std::partition(triangles.data() + begin, triangles.data() + end, triangle_less1(axis, cut));
        int m= std::distance(triangles.data(), pm);
        

        if(m == begin || m == end)
            m= (begin + end) / 2;
        assert(m != begin);
        assert(m != end);
        

        BBox bounds_right= triangles[m].bounds();
        for(int i= m+1; i < end; i++)
            bounds_right.insert(triangles[i].bounds());
        
        int right= build(bounds_right, m, end, next);

        BBox bounds_left= triangles[begin].bounds();
        for(int i= begin+1; i < m; i++)
            bounds_left.insert(triangles[i].bounds());
        int left= build(bounds_left, begin, m, right);
        
        int index= nodes.size();
        nodes.push_back(make_nodeC(bounds, left,next, right));
        return index;
    }
    };