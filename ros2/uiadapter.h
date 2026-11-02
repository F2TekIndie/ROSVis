
#ifndef UIADAPTER_H
#define UIADAPTER_H

#include <QObject>
#include <QVariantList>
#include <QMap>
#include <QStringList>
#include <memory>
#include <QQuickItem>
#include <QtQml/qqmlregistration.h>
#include "commands.h"



//
class rosadapter;

// ROS2 Data Adapter
// DDS
class UIAdapter : public QObject{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY( QString currentTopic READ currentTopic WRITE setCurrentTopic NOTIFY currentTopicChanged FINAL )

public:
    //
    UIAdapter( QObject* parent = nullptr );
    QString currentTopic() const;
    //
    Q_INVOKABLE void sendCommand( Commands::ROS_CMD nCmd );
    Q_INVOKABLE void getLatest();
    Q_INVOKABLE void getSchemas();

public slots:
    void setCurrentTopic( const QString& sTopic );

signals:
    void topicsReceived( const QVariantList& topics );
    void currentTopicChanged( const QString& sTopic );
    //
    void subscriptionChanged( const QString& topic, bool subscribed );
    void latestReceived( const QString& topic, const QVariantMap& data );
    //
    void schemas( const QVariantList& schemas );
    //
    void errorOccurred( const QString& message );

private:
    std::shared_ptr<rosadapter> _rosadapter;
    QMap<QString, QStringList> _types;
    QString _currTopic{ "" };
};

#endif // UIADAPTER_H
