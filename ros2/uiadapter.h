
#ifndef UIADAPTER_H
#define UIADAPTER_H

#include <QObject>



// ROS2 Data Adapter
// DDS
class UIAdapter : public QObject
{
    Q_OBJECT

public:
    UIAdapter();

public slots:
    void GetTopics();
    QVariant GetData( const QString& sTopic );

signals:
    void NewTopic( const QString& sTopic );
    void NewData( const QString& sTopic );

private:
};

#endif // UIADAPTER_H
