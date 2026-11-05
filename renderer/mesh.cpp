
#include "mesh.h"




Mesh::Mesh( std::vector<Vertex>& vertices  )
{
    static int meshIds = 0;
    _meshId = ++meshIds;
    //
    std::swap( vertices, _vertices );
}

std::span<const Vertex> Mesh::vertices() const{
    return _vertices;
}

int Mesh::meshId() const{
    return _meshId;
}

//
Color Mesh::color() const{
    return _color;
}

//
void Mesh::setColor( const Color& color ){
    _color = color;
}
