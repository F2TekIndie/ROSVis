
#include "rosadapter.h"
#include "client.h"
#include <iostream>
#include <QVariant>
#include "message.pb.h"
#include "topicsRequest.pb.h"



//
namespace{
}

//
rosadapter::rosadapter()
    :_ioCtx()
    ,_cmdChannel( _ioCtx.get_executor() )
    ,_dataChannel( _ioCtx.get_executor() )
{
    // get configuration for ip and port
    Client::loadConfiguration( this );
    // start communication channels
    Client::start( this );
}

rosadapter::~rosadapter()
{
    Client::stop( this );
}

/*
 */
std::string rosadapter::MakeCommand( Commands::ROS_CMD nCmd ){
    static uint64_t _nRequestId{0};
    robotics::Message msg;
    msg.set_request_id( ++_nRequestId );
    //
    switch( nCmd ){
        case Commands::RC_GET_TOPICS:
            msg.mutable_topics_request();
            break;
        case Commands::RC_SUBSCRIBE:
            msg.mutable_sub_request();
            break;
        case Commands::RC_UNSUBSCRIBE:
            msg.mutable_unsub_request();
            break;
        case Commands::RC_EXIT:
            msg.mutable_exit_request();
            break;
        default:
            break;
    }
    std::string sCmd;
    msg.SerializeToString( &sCmd );
    return sCmd;
}
void rosadapter::SendCommand( std::string sCommand ){
    _cmdChannel.async_send( boost::system::error_code{}, std::move(sCommand), [](boost::system::error_code ec){
        if ( ec ){
            std::cout << "Error while sending message: " << ec << std::endl;
        }
    });
}
void rosadapter::HandleData( std::string const& sData ){
    // Get command
    robotics::Message msg;
    if ( !msg.ParseFromString( sData ) ){
        return;
    }
    //
    switch( msg.payload_case() ){
        case robotics::Message::kTopicsResponse:
        {
            auto data = msg.topics_response();
            for( auto& topic : data.topics() ){
                QString sTopic = QString::fromStdString( topic.name() );
                emit newTopic( sTopic );
            }
            break;
        }
        case robotics::Message::kSubResponse:
        {
            auto data = msg.sub_response();
            break;
        }
        case robotics::Message::kUnsubResponse:
        {
            auto data = msg.unsub_response();
            break;
        }
        default:
            break;
    }
}

/*
 */
boost::asio::io_context& rosadapter::ioCtx(){
    return _ioCtx;
}
MyChannelType& rosadapter::cmdChannel(){
    return _cmdChannel;
}
MyChannelType& rosadapter::dataChannel(){
    return _dataChannel;
}

rosadapterconfig& rosadapter::config(){
    return _config;
}
