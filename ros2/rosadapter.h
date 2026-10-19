
#ifndef ROSADAPTER_H
#define ROSADAPTER_H

#include <boost/asio.hpp>
#include <map>
#include <QString>
#include "ros2/data.h"



/*
 * Send commands to ROSAdapter
 * Receive command responses
 * Receive ros2 data (DDS) from ROSAdapter
 */
class rosadapter
{
public:
    rosadapter();

private:
    std::map< QString, std::shared_ptr<Data> > _topicData = {};
};

#endif // ROSADAPTER_H
