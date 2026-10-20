
#include "rosadapter.h"
#include "client.h"
#include <iostream>



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
std::string rosadapter::MakeCommand(){
    std::string sCmd = {"TopicsQuery\n"};
    return sCmd;
}
void rosadapter::SendCommand( std::string sCommand ){
    _cmdChannel.async_send( boost::system::error_code{}, std::move(sCommand), [](boost::system::error_code ec){
        if ( ec ){
            std::cout << "Error while sending message: " << ec << std::endl;
        }
    });
}
void rosadapter::HandleData( std::string sData ){
    ;
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