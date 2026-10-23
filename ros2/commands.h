#ifndef COMMANDS_H
#define COMMANDS_H

#include <QObject>

namespace Commands{
    Q_NAMESPACE

    enum ROS_CMD{
        RC_INVALID = 0,
        RC_GET_TOPICS = 1,
        RC_SUBSCRIBE = 2,
        RC_UNSUBSCRIBE = 3,
        RC_EXIT = 4
    };
    Q_ENUM_NS( ROS_CMD )
}


#endif // COMMANDS_H
