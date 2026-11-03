#include "rosadapter.h"
#include <grpcpp/grpcpp.h>
#include <chrono>
#include <algorithm>
#include <stdexcept>
#include <google/protobuf/util/json_util.h>
#include <QJsonDocument>
#include <QJsonObject>



//
namespace{
    void deadline( grpc::ClientContext& context ){
        context.set_deadline( std::chrono::system_clock::now() + std::chrono::seconds( 2 ) );
    }

    void collectBinaryFields( const google::protobuf::Descriptor& descriptor, const QString& prefix, QStringList& paths, int depth = 0 ){
        if ( depth > 64 || paths.size() >= 4096 ){
            return;
        }
        for ( int i = 0; i < descriptor.field_count(); ++i ){
            const auto* field = descriptor.field( i );
            auto path = prefix + QString::fromStdString( field->name() );
            if ( field->is_repeated() ){
                path += "[]";
            }
            if ( field->type() == google::protobuf::FieldDescriptor::TYPE_BYTES ){
                paths.append( path );
            }
            else if ( field->cpp_type() == google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE ){
                collectBinaryFields( *field->message_type(), path + ".", paths, depth + 1 );
            }
        }
    }

} // namespace

//
rosadapter::rosadapter( const rosadapterconfig& config )
    :workerContext_( new QObject )
{
    grpc::ChannelArguments channelArguments;
    channelArguments.SetMaxReceiveMessageSize( 72 * 1024 * 1024 );
    channelArguments.SetMaxSendMessageSize( 72 * 1024 * 1024 );
    stub_ = robotics::ROSAdapter::NewStub( grpc::CreateCustomChannel(
        config.sIp + ":" + std::to_string( config.nPort ), grpc::InsecureChannelCredentials(), channelArguments ) );
    workerContext_->moveToThread( &worker_ );
    connect( &worker_, &QThread::finished, workerContext_, &QObject::deleteLater );
    worker_.start();
}

//
rosadapter::~rosadapter(){
    worker_.requestInterruption();
    worker_.quit();
    worker_.wait();
}

//
void rosadapter::report( const grpc::Status& status ){
    if ( !status.ok() ){
        emit errorOccurred( QString::fromStdString( status.error_message() ) );
    }
}

//
bool rosadapter::loadSchema( const std::string& type ){
    const auto knownTypes = schemas_.supportedTypes();
    if ( std::find( knownTypes.begin(), knownTypes.end(), type ) != knownTypes.end() ){
        return true;
    }
    grpc::ClientContext context;
    deadline( context );
    robotics::SchemaRequest request;
    robotics::SchemaResponse response;
    request.set_type( type );
    auto status = stub_->GetSchema( &context, request, &response );
    if ( !status.ok() ){
        report( status );
        return false;
    }
    try{
        if ( response.type() != type ){
            throw std::invalid_argument( "Schema response does not match the requested ROS type" );
        }
        schemas_.loadSchema( response.descriptors() );
        if ( schemas_.descriptor( type )->full_name() != response.protobuf_type() ){
            throw std::invalid_argument( "Schema response has the wrong protobuf type" );
        }
    }
    catch ( const std::exception& error ){
        emit errorOccurred( QString::fromStdString( error.what() ) );
        return false;
    }
    return true;
}

//
void rosadapter::sendCommand( Commands::ROS_CMD command, const QString& topic, const QString& type ){
    QMetaObject::invokeMethod( workerContext_, [this, command, topic, type] {
        if ( worker_.isInterruptionRequested() ){
            return;
        }
        if ( command == Commands::RC_SUBSCRIBE && !loadSchema( type.toStdString() ) ){
            return;
        }
        grpc::ClientContext context;
        deadline( context );
        if ( command == Commands::RC_GET_TOPICS ){
            robotics::ListTopicsRequest request;
            robotics::ListTopicsResponse response;
            auto status = stub_->ListTopics( &context, request, &response );
            if ( !status.ok() ){
                report( status );
                return;
            }
            //
            QVariantList topics;
            for ( const auto& entry : response.topics() ){
                if ( entry.subscribed() && !loadSchema( entry.subscribed_type() ) ){
                    return;
                }
                QStringList types;
                for ( const auto& value : entry.types() ){
                    types.append( QString::fromStdString( value ) );
                }
                topics.append( QVariantMap{ { "topicName", QString::fromStdString( entry.name() ) },
                    { "topicTypes", types },
                    { "subscribed", entry.subscribed() } } );
            }
            emit topicsReceived( topics );
        }
        else if ( command == Commands::RC_SUBSCRIBE || command == Commands::RC_UNSUBSCRIBE ){
            robotics::Topic response;
            grpc::Status status;
            if ( command == Commands::RC_SUBSCRIBE ){
                robotics::SubscribeRequest request;
                request.set_topic( topic.toStdString() );
                request.set_type( type.toStdString() );
                status = stub_->Subscribe( &context, request, &response );
            }
            else{
                robotics::TopicRequest request;
                request.set_topic( topic.toStdString() );
                status = stub_->Unsubscribe( &context, request, &response );
            }
            if ( !status.ok() ){
                report( status );
                return;
            }
            emit subscriptionChanged( topic, response.subscribed() );
        }
    }, Qt::QueuedConnection );
}

//
void rosadapter::getLatest( const QString& topic ){
    QMetaObject::invokeMethod( workerContext_, [this, topic] {
        if ( worker_.isInterruptionRequested() ){
            return;
        }
        grpc::ClientContext context;
        deadline( context );
        robotics::TopicRequest request;
        request.set_topic( topic.toStdString() );
        robotics::LatestData response;
        auto status = stub_->GetLatest( &context, request, &response );
        if ( !status.ok() ){
            report( status );
            return;
        }
        //
        QVariantMap data{ { "type", QString::fromStdString( response.type() ) },
            { "sequence", QVariant::fromValue<qulonglong>( response.sequence() ) },
            { "receivedUnixNanoseconds", QVariant::fromValue<qlonglong>( response.received_unix_nanoseconds() ) },
            { "hasData", response.has_payload() },
            { "conversionError", QString::fromStdString( response.conversion_error() ) } };
        //
        if ( response.has_payload() ){
            try{
                auto message = schemas_.unpack( response.payload() );
                if ( message->GetDescriptor() != schemas_.descriptor( response.type() ) ){
                    throw std::invalid_argument( "Payload type does not match the topic type" );
                }
                std::string json;
                google::protobuf::util::JsonPrintOptions options;
                options.preserve_proto_field_names = true;
                auto jsonStatus = google::protobuf::util::MessageToJsonString( *message, &json, options );
                if ( !jsonStatus.ok() ){
                    throw std::runtime_error( jsonStatus.ToString() );
                }
                const auto fields = QJsonDocument::fromJson( QByteArray::fromStdString( json ) ).object().toVariantMap();
                data.insert( "fields", fields );
                QStringList binaryFields;
                collectBinaryFields( *message->GetDescriptor(), {}, binaryFields );
                data.insert( "binaryFields", binaryFields );
                data.insert( "protobufType", QString::fromStdString( message->GetDescriptor()->full_name() ) );
                // Preserve the scalar value convenience used by existing consumers.
                if ( fields.size() == 1 && fields.contains( "data" ) ){
                    data.insert( "value", fields.value( "data" ) );
                }
            }
            catch ( const std::exception& error ){
                emit errorOccurred( QString::fromStdString( error.what() ) );
                return;
            }
        }
        emit latestReceived( topic, data );
    }, Qt::QueuedConnection );
}

//
void rosadapter::getSchemas(){
    QVariantList lTypes;
    auto availableTypes = schemas_.supportedTypes();
    for ( const auto& type : availableTypes ){
        lTypes.append( QString::fromStdString( type ) );
    }
    //
    emit schemas( lTypes );
}
