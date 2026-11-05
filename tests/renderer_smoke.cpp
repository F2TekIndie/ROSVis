#include "ui/vulkanview.h"
#include <QGuiApplication>
#include <QQuickWindow>
#include <QTimer>
#include <QImage>
#include <iostream>



//
int main( int argc, char** argv ){
    QQuickWindow::setGraphicsApi( QSGRendererInterface::Vulkan );
    QGuiApplication app( argc, argv );
    QQuickWindow window;
    window.resize( 640, 480 );
    VulkanView view;
    view.setParentItem( window.contentItem() );
    view.setSize( QSizeF( 640, 480 ) );
    window.show();
    QTimer::singleShot( 1500, &app, [&]{
        const auto image = window.grabWindow();
        if ( image.isNull() ){
            std::cerr << "Cannot capture Vulkan scene" << std::endl;
            app.exit( 1 );
            return;
        }
        image.save( "/tmp/rosvis-camera-light.png" );
        int robotPixels = 0;
        int groundPixels = 0;
        for ( int y = 0; y < image.height(); ++y ){
            for ( int x = 0; x < image.width(); ++x ){
                const auto pixel = image.pixelColor( x, y );
                if ( pixel.red() > pixel.blue() * 2 && pixel.red() > 40 ){
                    ++robotPixels;
                }
                if ( pixel.blue() > pixel.red() && pixel.blue() > 20 ){
                    ++groundPixels;
                }
            }
        }
        std::cout << "Robot pixels: " << robotPixels << ", ground pixels: " << groundPixels << std::endl;
        app.exit( robotPixels > 100 && groundPixels > 1000 ? 0 : 2 );
    } );
    return app.exec();
}
