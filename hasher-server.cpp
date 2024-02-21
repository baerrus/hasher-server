//
// Copyright (c) 2024 Vlad Troyanker
//
#include <errno.h>

#include "hasher-server.h"


ClientConnection::ClientConnection(tcp::socket socket, asio::thread_pool& compute)
    : socket_(io_context_)
    , strand_(compute.executor())
    //,strand_(io_context.get_executor())
    , compute_(compute)
{
    // must transfer socket ownership since client connection
    // operates on a different io_context from the acceptor
    socket_.assign(tcp::v4(), socket.release());
}

void ClientConnection::start()
{
    try {
        std::cout << "New client: " << socket_.remote_endpoint()
                  << " threadId: " << std::this_thread::get_id() << std::endl;
        do_read();
        io_context_.run();

        std::cout << "Client thread ended: " << std::this_thread::get_id() << std::endl;
    } catch (std::exception& e) {
        std::cerr << "** client exception: " << e.what() << "\n";
    }
}

void ClientConnection::do_read()
{
    auto buffer = make_buffer(max_read_size);
    if (socket_.is_open())
        socket_.async_read_some(asio::buffer(buffer->data(), max_read_size),
            [this, buffer, self = shared_from_this()](std::error_code ec, std::size_t length) {
                if (!ec) {
                    buffer->resize(length);
                    asio::post(compute_, asio::bind_executor(strand_, [this, self, buffer](){
                    hasher_.work(buffer, [self](Buffer buffer) {
                        self->do_write(buffer);
                    	}); // end of work callback
                    })); // end of post handler
                    self->do_read();
                } else if (ec == asio::error::misc_errors::eof) {
                    std::cout << "Client disconnected: " << socket_.remote_endpoint() << std::endl;
                    socket_.shutdown(asio::ip::tcp::socket::shutdown_receive);
                } else {
                    std::cerr << "read error: " << ec << std::endl;
                    self->do_close();
                }
            });
}

void ClientConnection::do_write(Buffer buffer)
{
    auto self(shared_from_this());
    if (socket_.is_open())
        asio::async_write(socket_, asio::buffer(buffer->data(), buffer->size()),
            [self, buffer](std::error_code ec, std::size_t /*length*/) {
                if (ec) {
                    std::cerr << "write error: " << ec << std::endl;
                    self->do_close();
                }
            });
}

void ClientConnection::do_close()
{
    socket_.close();
}

TcpServer::TcpServer(Configuration & config)
    : acceptor_(io_context_, tcp::endpoint(tcp::v4(), config.port.value_or(8000)))
    , socket_(io_context_)
    , io_pool_(config.conn_pool_capacity.value_or(16))
    , compute_pool_(config.compute_pool_capacity.value_or(16))
{
    std::cout << "Starting server on " << acceptor_.local_endpoint()
              << std::endl;
}

TcpServer::~TcpServer()
{
    std::cout << "Stopping server on " << acceptor_.local_endpoint() << std::endl;
    io_pool_.join();
    compute_pool_.join();
}

void TcpServer::serve()
{
    do_accept();
    io_context_.run();
}

void TcpServer::do_accept()
{
    acceptor_.async_accept(socket_, [this](std::error_code ec) {
        if (!ec) {
            asio::post(io_pool_, [this]() {
                std::make_shared<ClientConnection>(std::move(socket_), compute_pool_)->start();
            });
        }
        do_accept();
    });
}
