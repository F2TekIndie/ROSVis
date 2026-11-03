#include <QGuiApplication>
#include <QtQuick/QQuickView>
#include <QQmlApplicationEngine>
#include "ros2/uiadapter.h"
#include "ros2/commands.h"
#include "ros2/topicdatamodel.h"



//
int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    //
    QQuickWindow::setGraphicsApi( QSGRendererInterface::Vulkan );
    //
    qmlRegisterUncreatableMetaObject( Commands::staticMetaObject, "Robotics.Commands", 1, 0, "Commands", "Provides enum values only" );
    //
    QQmlApplicationEngine engine;
    UIAdapter uiAdapter;
    TopicDataModel data;
    QObject::connect( &uiAdapter, &UIAdapter::latestReceived, &data, &TopicDataModel::onLatestReceived );
    //
    engine.setInitialProperties({
        {"rossy", QVariant::fromValue( &uiAdapter )},
        {"dataModel", QVariant::fromValue( &data )}
    });
    engine.loadFromModule("ROSVis", "Main");
    //
    return app.exec();
}
