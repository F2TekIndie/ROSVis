#include <QGuiApplication>
#include <QtQuick/QQuickView>
#include <QQmlApplicationEngine>



int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQuickWindow::setGraphicsApi( QSGRendererInterface::Vulkan );
    //
    QQmlApplicationEngine engine;
    engine.loadFromModule("ROSVis", "Main");
    //
    return app.exec();
}
