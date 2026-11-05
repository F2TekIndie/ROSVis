#ifndef CAMERA_H
#define CAMERA_H

#include <QMatrix4x4>
#include <QVector3D>



//
struct Camera{
    QVector3D _pos{ 8.f, -20.f, 18.f };
    QVector3D _target{ 0.f, 0.f, 1.f };
    QVector3D _up{ 0.f, 0.f, 1.f };
    float _verticalFov{ 85.f };
    float _nearPlane{ 0.1f };
    float _farPlane{ 2000.f };

    QMatrix4x4 viewProjection( float aspectRatio ) const;
};

#endif // CAMERA_H
