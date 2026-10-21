
#ifndef ROSADAPTER_H
#define ROSADAPTER_H

#include <QObject>
#include <boost/asio.hpp>
#include <boost/asio/experimental/concurrent_channel.hpp>
#include <boost/asio/io_context.hpp>
#include "rosadapterconfig.h"



//
typedef typename boost::asio::experimental::concurrent_channel<void(boost::system::error_code, std::string)> MyChannelType;

//
struct Data;

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
    void HandleData( std::string_view sData );

    //
    boost::asio::io_context& ioCtx();
    MyChannelType& cmdChannel();
    MyChannelType& dataChannel();
    //
    rosadapterconfig& config();

    //
    std::shared_ptr<Data> GetData( int nCmd );

signals:
    void newDataAvailable( int nCmd );

private:
    //
    boost::asio::io_context _ioCtx;
    MyChannelType           _cmdChannel;
    MyChannelType           _dataChannel;
    //
    rosadapterconfig    _config;
    //
    std::map< int, std::shared_ptr<Data> > _dataContainer = {};
};

#endif // ROSADAPTER_H
