
#ifndef MESHFACTORY_H
#define MESHFACTORY_H

#include "mesh.h"



//
struct MeshFactory
{
    static MeshPtr CreateGroundPlane();
    static MeshPtr CreateBaseRobot();
};

#endif // MESHFACTORY_H
