//
// Copyright (c) 2024 Vlad Troyanker
//

#include <asio/bind_executor.hpp>
#include <asio/strand.hpp>
#include <asio/thread_pool.hpp>
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>

#include "hasher-config.h"

using asio::ip::tcp;



class TcpServer {
public:
    TcpServer(Configuration & config);

    ~TcpServer();

    void serve();

private:
    void do_accept();

    asio::io_context io_context_; // acceptor runs in this context
    tcp::acceptor acceptor_;
    tcp::socket socket_;
    asio::thread_pool io_pool_; // client connections run here
};