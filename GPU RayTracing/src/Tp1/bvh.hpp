#pragma once

#include <vector>
#include <iostream>
#include <algorithm>

#include <vec.h>

#include "node.hpp"
#include "Triangle_less.hpp"

struct BVH
{
    std::vector<Node> nodes;
    std::vector<Triangle> triangles;
    int root;
    
    // construit un bvh pour l'ensemble de triangles
    int build( const std::vector<Triangle>& _triangles, const BBox& _bounds )
    {
        triangles= _triangles;  // copie les triangles pour les trier
        nodes.clear();          // efface les noeuds
        
        // construit l'arbre... 
        root= build(_bounds, 0, triangles.size());
        // et renvoie la racine
        return root;
    }
    
protected:
    // construction d'un noeud
    int build( const BBox& bounds, const int begin, const int end )
    {
        if(end - begin <= 2)
        {
            // inserer une feuille et renvoyer son indice
            int index= nodes.size();
            nodes.push_back(make_leaf(bounds, begin, end));
            return index;
        }
        
        // axe le plus etire de l'englobant
        Vector d= Vector(bounds.pmin.returnPoint(), bounds.pmax.returnPoint());
        int axis;
        if(d.x > d.y && d.x > d.z)  // x plus grand que y et z ?
            axis= 0;
        else if(d.y > d.z)          // y plus grand que z ? (et que x implicitement)
            axis= 1;
        else                        // x et y ne sont pas les plus grands...
            axis= 2;
        
        // coupe l'englobant au milieu
        float cut= bounds.centroid(axis);
        
        // repartit les triangles 
        Triangle *pm= std::partition(triangles.data() + begin, triangles.data() + end, triangle_less1(axis, cut));
        int m= std::distance(triangles.data(), pm);
        
        // la repartition des triangles peut echouer, et tous les triangles sont dans la meme partie... 
        // forcer quand meme un decoupage en 2 ensembles 
        if(m == begin || m == end)
            m= (begin + end) / 2;
        assert(m != begin);
        assert(m != end);
        
        // construire le fils gauche
        // les triangles se trouvent dans [begin .. m)
        BBox bounds_left= triangle_bounds(begin, m);    // englobant des triangles
        int left= build(bounds_left, begin, m);         // construit le fils gauche 
        
        // on recommence pour le fils droit
        // les triangles se trouvent dans [m .. end)
        BBox bounds_right= triangle_bounds(m, end);     // englobant des triangles
        int right= build(bounds_right, m, end);         // construit le fils droit
        
        // inserer un noeud interne et renvoyer son indice
        int index= nodes.size();
        nodes.push_back(make_node(bounds, left, right));
        return index;
    }
    
    BBox triangle_bounds( const int begin, const int end )
    {
        BBox bbox= triangles[begin].bounds();
        for(int i= begin +1; i < end; i++)
            bbox.insert(triangles[i].bounds());
        
        return bbox;
    }
    
};
