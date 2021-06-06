#pragma once

#include "BBox.hpp"
#include "assert.h"

struct NodeC
{
    BBox bounds;
    int left;   
    int next;  
    int end;    
    int pad;    

    bool internal( ) const { return left >= 0; }                           
    bool leaf( ) const { return left < 0; }                            

};

NodeC make_nodeC( const BBox& bounds, const int left, const int next, const int end )
{
    NodeC node { bounds, left, next, end , -1};
    assert(node.internal());    // verifie que c'est bien un noeud...
    return node;
}

// creation d'une feuille
NodeC make_leafC( const BBox& bounds, const int begin, const int next, const int end )
{
    NodeC node { bounds, -begin - 1, next , -end ,begin}; // -1 pour éviter le cas ou begin == 0
    assert(node.leaf());        // verifie que c'est bien une feuille...
    return node;
}