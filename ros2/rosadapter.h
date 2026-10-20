
#ifndef ROSADAPTER_H
#define ROSADAPTER_H

#include <boost/asio.hpp>
#include <boost/asio/experimental/concurrent_channel.hpp>
#include <boost/asio/io_context.hpp>
#include <map>
#include <QString>
#include "ros2/data.h"


//
typedef typename boost::asio::experimental::concurrent_channel<void(boost::system::error_code, std::string)> MyChannelType;

/*
 * Send commands to ROSAdapter
 * Receive command responses
 * Receive ros2 data (DDS) from ROSAdapter
 */
class rosadapter
{
public:
    rosadapter();
    ~rosadapter();

    //
    boost::asio::io_context& ioCtx();
    MyChannelType& cmdChannel();
    MyChannelType& dataChannel();

private:
    std::map< QString, std::shared_ptr<Data> > _topicData = {};
    //
    boost::asio::io_context _ioCtx;
    MyChannelType           _cmdChannel;
    MyChannelType           _dataChannel;
};

#endif // ROSADAPTER_H
