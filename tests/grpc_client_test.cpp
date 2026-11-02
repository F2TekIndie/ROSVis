#include "rosadapter.h"
#include "ros_messages.pb.h"
#include "custom_schema.h"
#include <QCoreApplication>
#include <QTimer>
#include <grpcpp/grpcpp.h>
#include <iostream>
#include <atomic>



//
class Service : public robotics::ROSAdapter::Service{
public:
    explicit Service( bool failSchema, bool existingSubscription )
        :failSchema_( failSchema )
        ,existingSubscription_( existingSubscription )
    {
        google::protobuf::FileDescriptorSet descriptors;
        *descriptors.add_file() = TestSchema::descriptor();
        schemas_.loadSchema( descriptors );
    }

    std::atomic<int> schemaRequests{ 0 };
    std::atomic<int> subscriptions{ 0 };

private:
    bool failSchema_;
    bool existingSubscription_;
    rosdata::SchemaRegistry schemas_;

    grpc::Status GetSchema( grpc::ServerContext*, const robotics::SchemaRequest* request, robotics::SchemaResponse* response ) override{
        ++schemaRequests;
        if ( failSchema_ ){
            return { grpc::StatusCode::UNAVAILABLE, "Schema unavailable" };
        }
        response->set_type( request->type() );
        response->set_protobuf_type( schemas_.descriptor( request->type() )->full_name() );
        *response->mutable_descriptors() = schemas_.schema( request->type() );
        return grpc::Status::OK;
    }

    grpc::Status ListTopics( grpc::ServerContext*, const robotics::ListTopicsRequest*, robotics::ListTopicsResponse* out ) override{
        auto* topic = out->add_topics();
        topic->set_name( "/test" );
        topic->add_types( "std_msgs/msg/Int32" );
        if ( existingSubscription_ ){
            topic = out->add_topics();
            topic->set_name( "/custom" );
            topic->add_types( "custom_vehicle/msg/Telemetry" );
            topic->set_subscribed( true );
            topic->set_subscribed_type( "custom_vehicle/msg/Telemetry" );
        }
        return grpc::Status::OK;
    }

    grpc::Status Subscribe( grpc::ServerContext*, const robotics::SubscribeRequest* in, robotics::Topic* out ) override{
        ++subscriptions;
        if ( in->type() != "std_msgs/msg/Int32" && in->type() != "custom_vehicle/msg/Telemetry" ){
            return { grpc::StatusCode::INVALID_ARGUMENT, "wrong type" };
        }
        out->set_name( in->topic() );
        out->set_subscribed( true );
        return grpc::Status::OK;
    }

    grpc::Status GetLatest( grpc::ServerContext*, const robotics::TopicRequest* request, robotics::LatestData* out ) override{
        if ( request->topic() == "/custom" ){
            out->set_type( "custom_vehicle/msg/Telemetry" );
            out->set_sequence( 1 );
            auto value = schemas_.create( out->type() );
            const auto* field = value->GetDescriptor()->FindFieldByName( "counter" );
            value->GetReflection()->SetUInt64( value.get(), field, UINT64_MAX );
            out->mutable_payload()->PackFrom( *value );
            return grpc::Status::OK;
        }
        out->set_type( "std_msgs/msg/Int32" );
        out->set_sequence( 1 );
        robotics::ros::std_msgs__msg__Int32 value;
        value.set_data( -42 );
        out->mutable_payload()->PackFrom( value );
        return grpc::Status::OK;
    }
};

//
int main( int argc, char** argv ){
    QCoreApplication app( argc, argv );
    const bool failSchema = app.arguments().contains( "--schema-failure" );
    const bool existingSubscription = app.arguments().contains( "--existing-subscription" );
    Service service( failSchema, existingSubscription );
    grpc::ServerBuilder builder;
    int port = 0;
    builder.AddListeningPort( "127.0.0.1:0", grpc::InsecureServerCredentials(), &port );
    builder.RegisterService( &service );
    auto server = builder.BuildAndStart();
    if ( !server ){
        return 2;
    }
    rosadapter client( rosadapterconfig{ "127.0.0.1", port } );
    QObject::connect( &client, &rosadapter::errorOccurred, &app, [&]( const QString& e ){
        std::cerr << e.toStdString();
        app.exit( failSchema && e == "Schema unavailable" ? 0 : 3 );
    } );
    QObject::connect( &client, &rosadapter::topicsReceived, &app, [&]( const QVariantList& t ){
        if ( t.size() != ( existingSubscription ? 2 : 1 ) || t[0].toMap()["topicName"] != "/test" ){
            app.exit( 4 );
            return;
        }
        client.sendCommand( Commands::RC_SUBSCRIBE, "/test", "std_msgs/msg/Int32" );
    } );
    QObject::connect( &client, &rosadapter::subscriptionChanged, &app, [&]( const QString& t, bool s ){
        if ( !s ){
            app.exit( 5 );
        }
        else{
            client.getLatest( t );
        }
    } );
    int customSamples = 0;
    QObject::connect( &client, &rosadapter::latestReceived, &app, [&]( const QString& topic, const QVariantMap& d ){
        if ( topic == "/test" ){
            if ( d["value"].toInt() != -42 || !d["hasData"].toBool() ){
                app.exit( 6 );
                return;
            }
            client.sendCommand( Commands::RC_SUBSCRIBE, "/custom", "custom_vehicle/msg/Telemetry" );
        }
        else{
            const auto fields = d["fields"].toMap();
            if ( fields.value( "counter" ).toString() != "18446744073709551615" ){
                app.exit( 8 );
                return;
            }
            if ( ++customSamples == 1 ){
                client.sendCommand( Commands::RC_SUBSCRIBE, "/custom", "custom_vehicle/msg/Telemetry" );
            }
            else{
                app.exit( 0 );
            }
        }
    } );
    QTimer::singleShot( 10000, &app, [&] {
        app.exit( 7 );
    } );
    if ( failSchema ){
        client.sendCommand( Commands::RC_SUBSCRIBE, "/custom", "custom_vehicle/msg/Telemetry" );
    }
    else{
        client.sendCommand( Commands::RC_GET_TOPICS );
    }
    int result = app.exec();
    server->Shutdown();
    server->Wait();
    if ( result == 0 && ( service.schemaRequests != 1 || service.subscriptions != ( failSchema ? 0 : 3 ) ) ){
        return 9;
    }
    return result;
}
