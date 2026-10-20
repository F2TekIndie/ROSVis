#ifndef CLIENT_H
#define CLIENT_H

#include <boost/asio/io_context.hpp>


//
class rosadapter;

//
namespace Client{
    void start( rosadapter* server );
    void stop( rosadapter* server );
};


#endif // CLIENT_H
