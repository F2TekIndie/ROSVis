#include <QGuiApplication>
#include <QtQuick/QQuickView>
#include <QQmlApplicationEngine>



int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQuickWindow::setGraphicsApi( QSGRendererInterface::Vulkan );
    //
    QQuickView view;
    view.setResizeMode( QQuickView::SizeRootObjectToView );
    view.setSource( QUrl("qrc:///ros2/ui/Main.qml") );
    view.show();
    //
    return app.exec();
}
