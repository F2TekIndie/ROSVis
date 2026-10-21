
#ifndef UIADAPTER_H
#define UIADAPTER_H

#include <QObject>
#include <QQuickItem>



//
class rosadapter;

// ROS2 Data Adapter
// DDS
class UIAdapter : public QObject
{
    Q_OBJECT
    QML_ELEMENT

public:
    enum ROS_CMD{
        RC_INVALID = 0,
        RC_GET_TOPICS = 1,
        RC_SUBSCRIBE = 2,
        RC_UNSUBSCRIBE = 3
    };
    Q_ENUM( ROS_CMD )

    //
    UIAdapter(QObject* parent = nullptr);

public slots:
    void sendCommand( UIAdapter::ROS_CMD nCmd );


private slots:
    void getData( int nCmd );


signals:
    void newTopic( const QString& sTopic );
    void newData( const QString& sTopic );


private:
    std::shared_ptr<rosadapter> _rosadapter;
};

#endif // UIADAPTER_H
