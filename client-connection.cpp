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
    , work_guard_(io_context_.get_executor()) // keep io_context running until this is released or reset
{
    // must transfer socket ownership since client connection
    // operates on a different io_context from the acceptor
    socket_.assign(tcp::v4(), socket.release());
}

ClientConnection::~ClientConnection(){
        // TODO: Ensure do_close is called on the correct io_context thread
        do_close(); 
        std::cout << this << " ClientConnection dtor: "<< " threadId: " << std::this_thread::get_id()<< std::endl;
}

// 
void ClientConnection::run()
{
    try {
        std::cout << this << " New client: " << socket_.remote_endpoint()
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
                                [this, buffer, self = shared_from_this()](std::error_code ec, std::size_t length)
                                {
                                    if (!ec)
                                    {
                                        buffer->resize(length);
                                        bq_.enqueue(buffer);
                                        asio::post(compute_, asio::bind_executor(strand_, [this, self]()
                                                                                { hasher_.proc_bytes(bq_, [self](Buffer buffer)
                                                                                                      { self->do_write(buffer); }); 
                                                                                })); // end of post handler

                                        self->do_read();
                                    }
                                    else if (ec == asio::error::misc_errors::eof)
                                    {
                                        asio::post(compute_, asio::bind_executor(strand_, [this, self]()
                                                                                { hasher_.finalize_bytes([self](Buffer buffer)
                                                                                                        { self->do_write(buffer); });
                                        
                                        asio::defer(io_context_, [self]() {      
                                                    self->finish();     // now that compute job is finished
                                                                        // we can close the connection
                                                                        std::cout << "Finished sent: use_count = " << self.use_count() << std::endl;
                                                });
                                        })); // end of post handler

                                        std::cout << "Client disconnected: " << socket_.remote_endpoint() << std::endl;
                                    }
                                    else
                                    {
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
* This is done by scheduling a post back to io_context from the compute context when it 
* has finished its work.
*/
void ClientConnection::finish()
{
    work_guard_.reset(); // all work is done
    std::cout << this << " ClientConnection finished" << std::endl;
    if( !bq_.empty())
        std::cout << this << "** pending buffer count: " << bq_.size() << std::endl;
}