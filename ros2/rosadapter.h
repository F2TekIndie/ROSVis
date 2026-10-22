
#ifndef ROSADAPTER_H
#define ROSADAPTER_H

#include <QObject>
#include <boost/asio.hpp>
#include <boost/asio/experimental/concurrent_channel.hpp>
#include <boost/asio/io_context.hpp>
#include "rosadapterconfig.h"



//
typedef typename boost::asio::experimental::concurrent_channel<void(boost::system::error_code, std::string)> MyChannelType;



/*
 * Send commands to ROSAdapter
 * Receive command responses
 * Receive ros2 data (DDS) from ROSAdapter
 */
class rosadapter : public QObject
{
    Q_OBJECT

public:
    rosadapter();
    ~rosadapter();

    std::string MakeCommand( int nCmd );
    void SendCommand( std::string sCmd );
    void HandleData( std::string const& sData );

    //
    boost::asio::io_context& ioCtx();
    MyChannelType& cmdChannel();
    MyChannelType& dataChannel();
    //
    rosadapterconfig& config();

signals:
    void newDataAvailable( int nCmd );

private:
    //
    boost::asio::io_context _ioCtx;
    MyChannelType           _cmdChannel;
    MyChannelType           _dataChannel;
    //
    rosadapterconfig    _config;
};

#endif // ROSADAPTER_H
