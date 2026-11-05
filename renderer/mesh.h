
#ifndef MESH_H
#define MESH_H

#include <vector>
#include <span>
#include <memory>



//
struct Vertex{
    float x{0.0f};
    float y{0.0f};
    float z{0.0f};
};
struct Color{
    float r{1.f};
    float g{1.f};
    float b{1.f};
};

//
class Mesh
{
public:
    explicit Mesh( std::vector<Vertex>& vertices );
    std::span<const Vertex> vertices() const;
    int meshId() const;
    Color color() const;
    void setColor( const Color& color );

private:
    Color _color;
    int _meshId{0};
    std::vector<Vertex> _vertices{};
};

//
typedef std::shared_ptr<Mesh> MeshPtr;

#endif // MESH_H
