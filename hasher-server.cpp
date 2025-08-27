//
// Copyright (c) 2024 Vlad Troyanker
//
#include <errno.h>
#include <iostream>
#include <thread>

#include "hasher-server.h"
#include "client-connection.h"

TcpServer::TcpServer(Configuration & config)
    : acceptor_(io_context_, tcp::endpoint(tcp::v4(), config.port.value_or(8000)))
    , socket_(io_context_)
    , io_pool_(config.conn_pool_capacity.value_or(16))
    //, compute_pool_(config.compute_pool_capacity.value_or(16))
{
    std::cout << "Starting server on " << acceptor_.local_endpoint()
              << std::endl;
}

TcpServer::~TcpServer()
{
    std::cout << "Stopping server on " << acceptor_.local_endpoint() << std::endl;
    io_pool_.join();
}

void TcpServer::serve()
{
	acceptor_.listen();
    do_accept();
    io_context_.run();
}

void TcpServer::do_accept()
{
    acceptor_.async_accept(socket_, [this](std::error_code ec) {
        if (!ec) {
            auto conn = std::make_shared<ClientConnection>(std::move(socket_));
            asio::post(io_pool_, [conn]() {
                conn->run();
            });
        }
        else {
            std::cerr << " ** accept error: " << ec.message() << std::endl;
        }
        do_accept();
    });
}
