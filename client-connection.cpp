//
// Copyright (c) 2025 Vlad Troyanker
//
#include "client-connection.h"


#include <iostream>
#include <thread>
#include <format>

ClientConnection::ClientConnection(tcp::socket socket, asio::thread_pool& compute)
    : socket_(io_context_)
    , strand_(compute.executor())
    , compute_(compute)
    , fin_timer(io_context_)
{
    // must transfer socket ownership since client connection
    // operates on a different io_context from the acceptor
    socket_.assign(tcp::v4(), socket.release());
}

ClientConnection::~ClientConnection(){
        // TODO: Ensure do_close is called on the correct io_context thread
        do_close(); 
        std::cout << "ClientConnection dtor: "<< std::endl;
}

// 
void ClientConnection::run()
{
    try {
        std::cout << "New client: " << socket_.remote_endpoint()
                  << " threadId: " << std::this_thread::get_id() << std::endl;
        do_read();
        io_context_.run();
        std::cout << "ClientConnection thread ending: " << std::this_thread::get_id() << std::endl;
    } catch (std::exception& e) {
        std::cerr << "** client exception: " << e.what() << std::endl;
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
                    bq_.enqueue(buffer);
                    asio::post(compute_, asio::bind_executor(strand_, [this, self](){
	                     hasher_.proc_bytes(bq_, [self](Buffer buffer) {
	                               self->do_write(buffer);
	                    			});
                    })); // end of post handler

                    self->do_read();
                }
                else if (ec == asio::error::misc_errors::eof)
                {
                    asio::post(compute_, asio::bind_executor(strand_, [this, self](){
                    hasher_.finalize_bytes([self](Buffer buffer)
                                           { self->do_write(buffer); });
                }));

                    std::cout << "Client disconnected: " << socket_.remote_endpoint() << std::endl;
                    std::cout << "self->use_count: " << self.use_count() << std::endl;
                    std::cout << "pending buffer count: " << bq_.size() << std::endl;
                    self->finish(); // schedule a grace timeout
                } else {
                    std::cerr << "read error: " << ec << std::endl;
                    self->do_close();
                }
                });
}

void ClientConnection::do_write(Buffer buffer)
{
    if (socket_.is_open())
    {
        auto self(shared_from_this());
        asio::async_write(socket_, asio::buffer(buffer->data(), buffer->size()),
                          [self, buffer](std::error_code ec, std::size_t /*length*/)
                          {
                              if (ec)
                              {
                                  std::cerr << "write error: " << ec << std::endl;
                                  self->do_close();
                              }
                          });
    }
    else
    {
        std::cout << "discard write buffer, socket closed" << std::endl;
    }
}

void ClientConnection::do_close()
{
    socket_.close();
}
/*
* When client closes the connection there may be some data still being processed.
* Therefore it is critical to let all pending data complete its processing before
* destroying this object.
* This is done by scheduling a grace timeout to the current I/O context. The timeout
* duration is proportional to the number of pending buffers.
*/
void ClientConnection::finish()
{
    using namespace std::literals::chrono_literals;
    auto timeout = std::chrono::milliseconds(bq_.size() + 1);
    fin_timer.expires_after(timeout);
    std::cout << "ClientConnection finish scheduled: " << std::endl;
    fin_timer.async_wait([](std::error_code ec)
                         { std::cout << "ClientConnection finish timeout: " << ec.message()  << std::endl; });
}