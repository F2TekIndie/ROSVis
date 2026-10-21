
#include "rosadapter.h"
#include "client.h"
#include <iostream>
#include <algorithm>



//
namespace{
    //
    const std::vector<std::string> cCommandString = {"Invalid","TopicsQuery","Subscribe","Unsubscribe"};
    //
    int getCommand( std::string_view sData ){
        // Extract command
        std::string sCommand = {""};
        {
            auto nIdx = sData.find( "||" );
            if ( nIdx != std::string::npos ){
                sCommand = sData.substr( 0, nIdx );
            }
        }
        // Find index
        auto findIt = std::find_if( cCommandString.begin(), cCommandString.end(), [&sCommand](std::string const& rhs){
            return rhs == sCommand;
        });
        //
        if ( findIt == cCommandString.end() ){
            return 0;
        }
        return std::distance( cCommandString.begin(), findIt );
    }
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
    Q_ASSERT( nCmd >= 0 && nCmd < cCommandString.size() );
    //
    std::string sCmd = cCommandString[ nCmd ] + "\n";
    return sCmd;
}
void rosadapter::SendCommand( std::string sCommand ){
    _cmdChannel.async_send( boost::system::error_code{}, std::move(sCommand), [](boost::system::error_code ec){
        if ( ec ){
            std::cout << "Error while sending message: " << ec << std::endl;
        }
    });
}
void rosadapter::HandleData( std::string_view sData ){
    // Get command
    int nCmd = getCommand( sData );
    // Store data for command
    ;
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