
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
std::string rosadapter::MakeCommand( int nCmd ){
    robotics::Message msg;
    msg.set_request_id( 1 );
    //
    switch( nCmd ){
        case 1:
            msg.mutable_topics_request();
            break;
        case 2:
            msg.mutable_sub_request();
            break;
        case 3:
            msg.mutable_unsub_request();
            break;
        case 4:
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
    int nCmd{0};
    switch( msg.payload_case() ){
        case robotics::Message::kTopicsResponse:
        {
            auto data = msg.topics_response();
            // Notify UI of new data
            emit newDataAvailable( 1 );
            nCmd = 1;
            break;
        }
        case robotics::Message::kSubResponse:
        {
            auto data = msg.sub_response();
            // Notify UI of new data
            emit newDataAvailable( 2 );
            nCmd = 2;
            break;
        }
        case robotics::Message::kUnsubResponse:
        {
            auto data = msg.unsub_response();
            nCmd = 3;
            break;
        }
        default:
            break;
    }
    // Notify UI of new data
    emit newDataAvailable( nCmd );
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
