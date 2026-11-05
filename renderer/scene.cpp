
#include "scene.h"
#include "meshfactory.h"




void Scene::Init(){
    _groundPlane = MeshFactory::CreateGroundPlane();
    _robot = MeshFactory::CreateBaseRobot();
}

std::vector<MeshPtr> Scene::Meshes(){
    std::vector<MeshPtr> result;
    //
    result.push_back( _groundPlane );
    result.push_back( _robot );
    result.push_back( _environment );
    return result;
}


//
Camera& Scene::camera(){
    return _cam;
}

//
const Camera& Scene::camera() const{
    return _cam;
}

//
Light& Scene::light(){
    return _dir;
}

//
const Light& Scene::light() const{
    return _dir;
}
