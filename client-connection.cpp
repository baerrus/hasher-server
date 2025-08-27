//
// Copyright (c) 2025 Vlad Troyanker
//
#include "client-connection.h"


#include <iostream>
#include <thread>
#include <format>

ClientConnection::ClientConnection(tcp::socket socket)
    : socket_(io_context_)
    , work_guard_(io_context_.get_executor()) // keep io_context running until this is released or reset
    , fin_timer_(io_context_)
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
        std::cerr << " ** client exception: " << e.what() << std::endl;
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
                                        asio::post(compute_, [buffer, self]()
                                                   { self->hasher_.proc_bytes(buffer, [self](Buffer buffer)
                                                                              { self->do_write(buffer); }); }); // end of post handler

                                        do_read();
                                    }
                                    else if (ec == asio::error::misc_errors::eof)
                                    {
                                        std::cout << this << " Client disconnected: " << socket_.remote_endpoint() << std::endl;
                                        asio::post(compute_, [self]()
                                                   {
                                                       self->hasher_.finalize_bytes([self](Buffer buffer)
                                                                                    { self->do_write(buffer); });

                                                       asio::post(self->io_context_, [self]()
                                                                  {
                                                                      self->finish(); // now that compute job is finished
                                                                                      // we can close the connection
                                                                  });
                                                       std::cout << self.get() << " Finish sent " << std::endl;
                                                   }); // end of post handler
                                    }
                                    else
                                    {
                                        std::cerr << " ** read error: " << ec << std::endl;
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
                            self->pending_write_ops_--;
                              if (ec)
                              {
                                  std::cerr << "write error: " << ec << std::endl;
                                  self->do_close();
                              }
                          });
        pending_write_ops_++;
    }
    else
    {
        std::cout << this << " ** discard write buffer, socket closed" << std::endl;
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
    int pending = pending_write_ops_.load();
    if (pending <= 0)
    {
        work_guard_.reset(); // all work is done
        // Note: we call join here because we are guaranteed to be on io_context thread
        // unlike the destructor which may be called from any thread
        compute_.join(); // wait for compute thread to complete all outstanding work

        std::cout << this << " ClientConnection finished" << std::endl;
    }
    else
    {
        std::cout << this << " ** pending write count: " << pending << std::endl;
        fin_timer_.expires_after(std::chrono::milliseconds(10));
        fin_timer_.async_wait([self = shared_from_this()](const std::error_code &ec)
                              {
            if(!ec) {
                self->finish(); // check again
            } 
            else {
                std::cerr << " ** finish timer error: " << ec << std::endl;
            } });
    }
}