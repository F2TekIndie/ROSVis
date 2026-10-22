
#include "client.h"
#include "ros2/rosadapter.h"
#include <thread>
#include <iostream>
#include <chrono>
#include <boost/asio/io_context.hpp>
#include <boost/asio.hpp>
#include <boost/asio/experimental/concurrent_channel.hpp>
#include <boost/asio/experimental/awaitable_operators.hpp>



//
namespace{
    using namespace boost::asio::experimental::awaitable_operators;
    using namespace std::chrono_literals;

    // --- Reader: pulls from socket, pushes to channel ---
    boost::asio::awaitable<void> reader(boost::asio::ip::tcp::socket& sock, MyChannelType& ch) {
        try {
            while (true) {
                std::array<unsigned char, 4> header;
                co_await boost::asio::async_read( sock, boost::asio::buffer(header), boost::asio::use_awaitable );
                //
                std::uint32_t size = (std::uint32_t(header[3])<<24) | (std::uint32_t(header[1]<<16)) | (std::uint32_t(header[2])<<8) | (std::uint32_t(header[3]));
                constexpr std::uint32_t maxSize = 16 * 1024 * 1024;
                if ( size > maxSize ){
                    throw std::runtime_error("Message to long");
                }
                //
                std::string sPayload(size,'\0');
                co_await boost::asio::async_read( sock, boost::asio::buffer(sPayload), boost::asio::use_awaitable );
                //
                co_await ch.async_send(boost::system::error_code{},std::move(sPayload), boost::asio::use_awaitable);
            }
        } catch (const std::exception&) { /* connection closed */ }
        ch.close(); // signal writer to stop
    }

    // --- Writer: pulls from channel, pushes to socket ---
    boost::asio::awaitable<void> writer(boost::asio::ip::tcp::socket& sock, MyChannelType& ch) {
        try {
            while (true){
                auto msg = co_await ch.async_receive(boost::asio::use_awaitable);
                constexpr std::size_t maxSize = 16*1024*1024;
                if ( msg.size() > maxSize ){
                    throw std::runtime_error("Message to long");
                }
                auto size = static_cast<std::uint32_t>(msg.size());
                std::array<unsigned char, 4> header{
                    static_cast<unsigned char>(size >> 24),
                    static_cast<unsigned char>(size >> 16),
                    static_cast<unsigned char>(size >> 8),
                    static_cast<unsigned char>(size)
                };
                std::array<boost::asio::const_buffer, 2> buffers{
                    boost::asio::buffer(header),
                    boost::asio::buffer(msg)
                };
                // Transform or just echo
                co_await boost::asio::async_write(sock, boost::asio::buffer(msg), boost::asio::use_awaitable);
            }
        } catch (const std::exception&) { /* connection closed */ }
    }

    boost::asio::awaitable<void> client(boost::asio::io_context& ctx, boost::asio::ip::tcp::endpoint endpoint, rosadapter* server ) {
        boost::asio::ip::tcp::socket socket( ctx );
        //
        co_await socket.async_connect( endpoint, boost::asio::use_awaitable );
        //
        auto& cmdCh = server->cmdChannel();
        auto& dataCh = server->dataChannel();
        // Run reader and writer in parallel; await BOTH completing
        co_await (reader(socket, dataCh) && writer(socket, cmdCh));
        //
        socket.close();
    }

    void runAsioCommunication( boost::asio::io_context& ctx ){
        // Move to thread
        ctx.run();
    }

    void receiveData( rosadapter& server ){
        auto& cmdCh = server.dataChannel();
        cmdCh.async_receive(
            [&](boost::system::error_code ec, std::string sMsg){
                if ( !ec ){
                    // Messages are terminated with new line.
                    // Needs to be stripped for valid command.
                    server.HandleData( sMsg );
                }
                if ( !server.ioCtx().stopped() ){
                    receiveData( server );
                }
        });
    }
}

//
namespace Client{
    std::jthread tWorker;
    std::jthread tReceiverParser;
    //
    void start( rosadapter* server ){
        Q_ASSERT( server );
        // IP & PORT
        // Start connection
        boost::asio::io_context& ctx = server->ioCtx();
        rosadapterconfig& config = server->config();
        auto endpoint = boost::asio::ip::tcp::endpoint( boost::asio::ip::make_address( config.sIp ), config.nPort );
        boost::asio::co_spawn( ctx, client(ctx, endpoint, server), boost::asio::detached);

        tWorker = std::jthread( runAsioCommunication, std::ref( ctx ) );
        tReceiverParser = std::jthread( receiveData, std::ref(*server) );
    }
    //
    void stop( rosadapter* server ){
        Q_ASSERT( server );
        // Close connections
        // Stop listening
        server->ioCtx().stop();
        tWorker.join();
        tReceiverParser.join();
    }
    //
    void loadConfiguration( rosadapter* server ){
        Q_ASSERT( server );
        //
        rosadapterconfig& config = server->config();
        //
        ;
    }
};