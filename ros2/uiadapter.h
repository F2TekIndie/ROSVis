
#ifndef UIADAPTER_H
#define UIADAPTER_H

#include <QObject>
#include <QQuickItem>
#include "commands.h"


//
class rosadapter;

// ROS2 Data Adapter
// DDS
class UIAdapter : public QObject
{
    Q_OBJECT
    QML_ELEMENT

public:
    //
    UIAdapter(QObject* parent = nullptr);

public slots:
    void sendCommand( Commands::ROS_CMD nCmd );


private slots:
    void handleNewTopic( const QString& sTopic );


signals:
    void newTopic( const QString& sTopic );

private:
    std::shared_ptr<rosadapter> _rosadapter;
};

#endif // UIADAPTER_H
