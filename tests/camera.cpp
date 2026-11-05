#include "scene.h"
#include <QtTest>
#include <cmath>



//
class CameraTest : public QObject{
    Q_OBJECT

private slots:
    void targetAndAspect(){
        Scene scene;
        const auto& camera = scene.camera();
        const auto square = camera.viewProjection( 1.f );
        const auto target = square * QVector4D( camera._target, 1.f );
        QVERIFY( std::abs( target.x() / target.w() ) < 0.0001f );
        QVERIFY( std::abs( target.y() / target.w() ) < 0.0001f );
        QVERIFY( target.z() / target.w() > 0.f && target.z() / target.w() < 1.f );
        const auto wide = camera.viewProjection( 2.f );
        QVERIFY( std::abs( wide( 0, 0 ) * 2.f - square( 0, 0 ) ) < 0.0001f );
    }

    void vulkanClipSpace(){
        Camera camera;
        camera._pos = { 0.f, -10.f, 0.f };
        camera._target = { 0.f, 0.f, 0.f };
        const auto matrix = camera.viewProjection( 1.f );
        const auto nearPoint = matrix * QVector4D( 0.f, -10.f + camera._nearPlane, 0.f, 1.f );
        const auto farPoint = matrix * QVector4D( 0.f, -10.f + camera._farPlane, 0.f, 1.f );
        QVERIFY( std::abs( nearPoint.z() / nearPoint.w() ) < 0.0001f );
        QVERIFY( std::abs( farPoint.z() / farPoint.w() - 1.f ) < 0.0001f );
        const auto elevated = matrix * QVector4D( 0.f, 0.f, 1.f, 1.f );
        QVERIFY( elevated.y() / elevated.w() < 0.f );
    }

    void overheadView(){
        Camera camera;
        camera._pos = { 0.f, 0.f, 10.f };
        camera._target = { 0.f, 0.f, 0.f };
        const auto matrix = camera.viewProjection( 1.f );
        QVERIFY( matrix.determinant() != 0.f );
        for ( int i = 0; i < 16; ++i ){
            QVERIFY( std::isfinite( matrix.constData()[i] ) );
        }
    }
};

QTEST_GUILESS_MAIN( CameraTest )
#include "camera.moc"
