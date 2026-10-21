
#include "rosadapter.h"
#include "client.h"
#include "data.h"
#include <iostream>
#include <algorithm>
#include <QVariant>



//
namespace{
    // NEEDS SOME RETHINKING => PREFER one place to setup names, indices, function pointers to construct
    // the whole send/receive and parse logic
    const std::vector<std::string> cCommandString = {"Invalid","TopicsQuery","Subscribe","Unsubscribe","Data"};
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
    //
    std::shared_ptr<Data> extractData( int nCmd, std::string_view sData ){
        std::shared_ptr<Data> newDataContainer;
        if ( cCommandString[nCmd] == "TopicsQuery" ){
            DataTopics* pData = new DataTopics();
            // Get starting point for topic search
            auto nIdx = sData.find( "||" );
            // Search topic
            do{
                auto nTopic = sData.find( ":", nIdx+2);
                if ( nTopic != std::string::npos ){
                    std::string_view sub = sData.substr( nIdx+2, nTopic-nIdx-2 );
                    pData->_topics.push_back( std::string(sub) );
                    //
                    nIdx = sData.find( "||", nTopic+1 );
                }
                else{
                    nIdx = std::string::npos;
                }
            }while( nIdx != std::string::npos );
            //
            newDataContainer.reset( pData );
        }
        return newDataContainer;
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
    _dataContainer[ nCmd ] = extractData( nCmd, sData );
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

std::shared_ptr<Data> rosadapter::GetData( int nCmd ){
    return  _dataContainer[nCmd];
}
