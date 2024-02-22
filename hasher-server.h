//
// Copyright (c) 2024 Vlad Troyanker
//
#include <iostream>
#include <thread>

#include <asio/bind_executor.hpp>
#include <asio/strand.hpp>
#include <asio/thread_pool.hpp>
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>

#include "buffer.h"
#include "hasher-stream.h"
#include "hasher-config.h"

using asio::ip::tcp;

class ClientConnection : public std::enable_shared_from_this<ClientConnection> {
public:
    ClientConnection(tcp::socket socket, asio::thread_pool& compute);

    void start();

private:
    void do_read();

    void do_write(Buffer buffer);

    void do_close();

    asio::io_context io_context_;
    tcp::socket socket_;
    HasherStream hasher_; // hash compute engine
    asio::strand<asio::thread_pool::executor_type> strand_; // serialize all compute within a single client
    asio::thread_pool& compute_; // @hasher_ executes on this pool
    BufferQueue bq_;  // pending buffers

    enum { max_read_size = 1024 };
};

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
    asio::thread_pool compute_pool_; // hash computations run here
};