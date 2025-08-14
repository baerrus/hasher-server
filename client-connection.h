//
// Copyright (c) 2025 Vlad Troyanker
//
#include <asio/bind_executor.hpp>
#include <asio/strand.hpp>
#include <asio/thread_pool.hpp>
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>
#include <asio/steady_timer.hpp>

#include <memory>
using asio::ip::tcp;

#include "buffer.h"
#include "hasher-stream.h"


class ClientConnection : public std::enable_shared_from_this<ClientConnection> {
public:
    ClientConnection(tcp::socket socket, asio::thread_pool& compute);
   ~ClientConnection();
   
    void run();

private:
    void do_read();
    void do_write(Buffer buffer);
    void do_close();
    void finish();
    
    asio::io_context io_context_;
    tcp::socket socket_;
    HasherStream hasher_;                                   // hash compute engine
    asio::strand<asio::thread_pool::executor_type> strand_; // serialize all compute within a single client
    asio::thread_pool &compute_;                            // @hasher_ executes on this pool
    BufferQueue bq_;                                        // pending buffers
    asio::steady_timer fin_timer;                     // timer to close the connection after all pending buffers are processed

    enum
    {
        max_read_size = 1024
    };
};