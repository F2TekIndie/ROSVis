
#ifndef SCENE_H
#define SCENE_H

#include "mesh.h"
#include "camera.h"
#include "light.h"



//
class Scene
{
public:
    Scene() = default;

    //
    void Init();
    std::vector<MeshPtr> Meshes();

    Camera& camera();
    const Camera& camera() const;
    Light& light();
    const Light& light() const;

private:
    // Base geometries
    MeshPtr    _groundPlane;
    MeshPtr    _robot;
    MeshPtr    _environment;
    // Sensor data

    // Z-up perspective camera
    Camera  _cam;
    // Base directional light
    Light   _dir;
};

#endif // SCENE_H
