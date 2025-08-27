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

#include "hasher-stream.h"


class ClientConnection : public std::enable_shared_from_this<ClientConnection> {
public:
    ClientConnection(tcp::socket socket);
   ~ClientConnection();
   
    void run();

private:
    void do_read();
    void do_write(Buffer buffer);
    void do_close();
    void finish();
    
    asio::io_context io_context_;
    tcp::socket socket_;
    HasherStream hasher_;                    // hash compute engine
    asio::thread_pool compute_{1};           // @hasher_ executes on this pool. It must be a single thread as that implicitly keep the order of the input data

    asio::executor_work_guard<asio::io_context::executor_type> work_guard_; // keeps io_context running until this is released or reset
    asio::steady_timer fin_timer_;                           // timer to delay closing the connection so all pending buffers are processed
    std::atomic<int> pending_write_ops_{0};                    // count of pending write operations

    enum
    {
        max_read_size = 1024
    };
};