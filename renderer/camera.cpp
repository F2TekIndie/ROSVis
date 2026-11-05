#include "camera.h"
#include <cmath>



//
QMatrix4x4 Camera::viewProjection( float aspectRatio ) const{
    auto forward = _target - _pos;
    if ( forward.lengthSquared() < 0.000001f ){
        forward = QVector3D( 0.f, 0.f, -1.f );
    }
    auto up = _up.isNull() ? QVector3D( 0.f, 0.f, 1.f ) : _up.normalized();
    if ( QVector3D::crossProduct( forward.normalized(), up ).lengthSquared() < 0.000001f ){
        // A directly overhead camera needs an image-up axis perpendicular to Z.
        up = std::abs( forward.normalized().y() ) < 0.99f ? QVector3D( 0.f, 1.f, 0.f ) : QVector3D( 1.f, 0.f, 0.f );
    }
    QMatrix4x4 view;
    view.lookAt( _pos, _pos + forward, up );
    QMatrix4x4 projection;
    projection.perspective( _verticalFov, aspectRatio > 0.f ? aspectRatio : 1.f, _nearPlane, _farPlane );
    // Qt matrices use OpenGL clip space. Vulkan uses downward Y and depth [0, 1].
    QMatrix4x4 correction;
    correction( 1, 1 ) = -1.f;
    correction( 2, 2 ) = 0.5f;
    correction( 2, 3 ) = 0.5f;
    return correction * projection * view;
}
