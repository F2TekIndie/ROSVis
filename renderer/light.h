#ifndef LIGHT_H
#define LIGHT_H

#include "mesh.h"
#include <QVector3D>



//
struct Light{
    // Direction in which the light rays travel, in world coordinates.
    QVector3D _dir{ -0.5f, -0.7f, -1.f };
    Color _col{ 1.f, 0.97f, 0.9f };
    float _strength{ 0.85f };
    float _ambient{ 0.2f };
};

#endif // LIGHT_H
