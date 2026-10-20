
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
    UIAdapter();

public slots:
    void getTopics();
    QVariant getData( const QString& sTopic );

signals:
    void newTopic( const QString& sTopic );
    void newData( const QString& sTopic );

private:
    std::shared_ptr<rosadapter> _rosadapter;
};

#endif // UIADAPTER_H
