#pragma once

#include <QObject>
#include <QVariantList>
#include <QThread>
#include <QStringList>
#include "adapter.grpc.pb.h"
#include "rosdata/schema.h"
#include "commands.h"
#include "rosadapterconfig.h"

//
class rosadapter : public QObject{
    Q_OBJECT
public:
    explicit rosadapter( const rosadapterconfig& config = {} );
    ~rosadapter() override;
    //
    void sendCommand( Commands::ROS_CMD command, const QString& topic = {}, const QString& type = {} );
    void getLatest( const QString& topic );
    void getSchemas();

signals:
    void topicsReceived( const QVariantList& topics );
    void subscriptionChanged( const QString& topic, bool subscribed );
    void latestReceived( const QString& topic, const QVariantMap& data );
    void schemas( const QVariantList& schemas );
    void errorOccurred( const QString& message );

private:
    void report( const grpc::Status& status );
    bool loadSchema( const std::string& type ); // Called on the worker thread.
    //
    rosdata::SchemaRegistry schemas_;
    QThread worker_;
    QObject* workerContext_;
    std::unique_ptr<robotics::ROSAdapter::Stub> stub_;
};
