
#ifndef UIADAPTER_H
#define UIADAPTER_H

#include <QObject>
#include <QQuickItem>
#include <QtQml/qqmlregistration.h>
#include "commands.h"


//
class rosadapter;

// ROS2 Data Adapter
// DDS
class UIAdapter : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QString currentTopic READ currentTopic WRITE setCurrentTopic NOTIFY currentTopicChanged FINAL)

public:
    //
    UIAdapter(QObject* parent = nullptr);
    QString currentTopic() const;

public slots:
    void sendCommand( Commands::ROS_CMD nCmd );
    void setCurrentTopic( const QString& sTopic );

private slots:
    void handleNewTopic( const QString& sTopic );


signals:
    void newTopic( const QString& sTopic );
    void currentTopicChanged( const QString& sTopic );

private:
    std::shared_ptr<rosadapter> _rosadapter;
    QString _currTopic{""};
};

#endif // UIADAPTER_H
