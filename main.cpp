#include <QGuiApplication>
#include <QtQuick/QQuickView>
#include <QQmlApplicationEngine>
#include "ros2/uiadapter.h"



int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQuickWindow::setGraphicsApi( QSGRendererInterface::Vulkan );
    //
    QQmlApplicationEngine engine;
    UIAdapter uiAdapter;
    //
    engine.setInitialProperties({
        {"rossy", QVariant::fromValue( &uiAdapter )}
    });
    engine.loadFromModule("ROSVis", "Main");
    //
    return app.exec();
}
