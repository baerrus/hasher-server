//
// Copyright (c) 2024 Vlad Troyanker
//

#include <iostream>
#include <thread>

#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>
#include <asio/thread_pool.hpp>
#include <asio/strand.hpp>


#include "buffer.h"
#include "hasher-stream.h"


using asio::ip::tcp;


class ClientSession : public std::enable_shared_from_this<ClientSession> {
public:
    ClientSession(tcp::socket socket)
        : socket_(io_context_)
    {
        // must transfer socket ownership since client connection
        // operates on a different io_context from the acceptor
        socket_.assign(tcp::v4(), socket.release());
    }

    void start()
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

private:
    void do_read()
    {
        auto buffer = make_buffer(max_read_size);
        if(socket_.is_open())
            socket_.async_read_some( asio::buffer(buffer->data(), max_read_size),
                [this, buffer, self = shared_from_this()](std::error_code ec, std::size_t length) {
                    if (!ec) {
                        buffer->resize(length);
                        hasher_.work(buffer, [this, self](Buffer buffer){
                            self->do_write(buffer);
                        });
                        self->do_read();
                   } else if(ec == asio::error::misc_errors::eof) {

                        std::cout << "Client disconnected: " << socket_.remote_endpoint() << std::endl;
                        socket_.shutdown(asio::ip::tcp::socket::shutdown_receive);
                    }
                    else {
                        std::cerr << "read error: " << ec << std::endl;
                        self->do_close();
                    }
                });
    }

    void do_write(Buffer buffer)
    {
        auto self(shared_from_this());
        asio::async_write(socket_, asio::buffer(buffer->data(), buffer->size()),
            [self, buffer](std::error_code ec, std::size_t /*length*/) {
                if (ec) {
                    std::cerr << "write error: " << ec << std::endl;
                    self->do_close();
                }
            });
    }

    void do_close() {
      socket_.close();
    }

    asio::io_context io_context_;
    tcp::socket          socket_;
    HasherStream         hasher_;  // hash compute engine
    //asio::strand         strand_;  // serialize all compute within a single client

    enum { max_read_size = 1024 };
};


class TcpServer {
 public:
  TcpServer(short port)
      : acceptor_(io_context_, tcp::endpoint(tcp::v4(), port)),
        socket_(io_context_) {
    std::cout << "Starting server on " << tcp::endpoint(tcp::v4(), port)
              << std::endl;
  }

  ~TcpServer() { io_pool_.join(); }

  void serve() {
    do_accept();
    io_context_.run();
  }

 private:

  void do_accept() {
    acceptor_.async_accept(socket_, [this](std::error_code ec) {
      if (!ec) {
        asio::post(io_pool_, [this]() {
          std::make_shared<ClientSession>(std::move(socket_))->start();
        });
      }
      do_accept();
    });
  }

  asio::io_context io_context_;  // acceptor runs in this context
  tcp::acceptor acceptor_;
  tcp::socket socket_;
  asio::thread_pool io_pool_;  // client connections run here
};

int main(int argc, char* argv[])
{
  try
  {
    const auto port = 8000;
    TcpServer server(port);

    server.serve();

  }
  catch (std::exception& e)
  {
    std::cerr << "** main exception: " << e.what() << "\n";
  }

  return 0;
}