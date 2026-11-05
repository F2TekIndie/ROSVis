#include "meshfactory.h"



//
MeshPtr MeshFactory::CreateGroundPlane(){
    // Two triangles facing +Z, centered at the world origin.
    std::vector<Vertex> vertices{
        { -500.f, -500.f, 0.f }, { 500.f, -500.f, 0.f }, { 500.f, 500.f, 0.f },
        { -500.f, -500.f, 0.f }, { 500.f, 500.f, 0.f }, { -500.f, 500.f, 0.f }
    };
    auto mesh = std::make_shared<Mesh>( vertices );
    mesh->setColor( { 0.28f, 0.34f, 0.4f } );
    return mesh;
}

//
MeshPtr MeshFactory::CreateBaseRobot(){
    // Triangle-list faces wind counterclockwise when viewed from outside.
    std::vector<Vertex> vertices{
        // Bottom (-Z)
        { -1.f, -1.f, 0.f }, { -1.f, 1.f, 0.f }, { 1.f, 1.f, 0.f },
        { -1.f, -1.f, 0.f }, { 1.f, 1.f, 0.f }, { 1.f, -1.f, 0.f },
        // Top (+Z)
        { -1.f, -1.f, 2.f }, { 1.f, -1.f, 2.f }, { 1.f, 1.f, 2.f },
        { -1.f, -1.f, 2.f }, { 1.f, 1.f, 2.f }, { -1.f, 1.f, 2.f },
        // Front (-Y)
        { -1.f, -1.f, 0.f }, { 1.f, -1.f, 0.f }, { 1.f, -1.f, 2.f },
        { -1.f, -1.f, 0.f }, { 1.f, -1.f, 2.f }, { -1.f, -1.f, 2.f },
        // Back (+Y)
        { -1.f, 1.f, 0.f }, { -1.f, 1.f, 2.f }, { 1.f, 1.f, 2.f },
        { -1.f, 1.f, 0.f }, { 1.f, 1.f, 2.f }, { 1.f, 1.f, 0.f },
        // Left (-X)
        { -1.f, -1.f, 0.f }, { -1.f, -1.f, 2.f }, { -1.f, 1.f, 2.f },
        { -1.f, -1.f, 0.f }, { -1.f, 1.f, 2.f }, { -1.f, 1.f, 0.f },
        // Right (+X)
        { 1.f, -1.f, 0.f }, { 1.f, 1.f, 0.f }, { 1.f, 1.f, 2.f },
        { 1.f, -1.f, 0.f }, { 1.f, 1.f, 2.f }, { 1.f, -1.f, 2.f }
    };
    auto mesh = std::make_shared<Mesh>( vertices );
    mesh->setColor( { 0.95f, 0.55f, 0.15f } );
    return mesh;
}
