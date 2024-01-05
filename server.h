#pragma once

#include <boost/asio.hpp>
#include <string_view>

#include <handler.h>

namespace net
{
  using boost::asio::ip::tcp;
  
  class connection : public std::enable_shared_from_this<connection>
  {
  public:
    using pointer = std::shared_ptr<connection>;

    static pointer create(tcp::socket socket, std::shared_ptr<async_bulk::Handler> h)
    {
      return pointer(new connection(std::move(socket), std::move(h)));
    }
    
    ~connection() = default;

    void receive() { read(); }

  private:
    enum{MAX_PACKET_SIZE=1024};
    connection(tcp::socket socket, std::shared_ptr<async_bulk::Handler> h) :
        m_socket(std::move(socket)), m_handler(h) {}
  
    void read()
    {
      auto self(shared_from_this());
      m_socket.async_read_some(boost::asio::buffer(m_data, MAX_PACKET_SIZE),
          [this, self](boost::system::error_code ec, std::size_t length)
          {
            if (!ec)
            {
              m_handler->handle(std::string{m_data, length});
              read();
            }
          });
    }

    tcp::socket                          m_socket;
    char                                 m_data[MAX_PACKET_SIZE];
    std::shared_ptr<async_bulk::Handler> m_handler;
  };
    
  class server
  {
  public:
    server(short unsigned int port, boost::asio::io_service& svc, std::shared_ptr<async_bulk::Handler> handler) :
    m_acceptor(svc, tcp::endpoint(tcp::v4(), port)), m_handler(handler)
    {
      listenAndServe();
    }
    
    ~server() = default;
    
    void listenAndServe()
    {
      m_acceptor.async_accept(
        [this](boost::system::error_code ec, tcp::socket socket)
        {
          if (!ec)
          {
            auto conn = connection::create(std::move(socket), m_handler);
            conn->receive();
            listenAndServe();
          }
        }
      );
    }
  private:
    boost::asio::io_context               io_context;
    tcp::acceptor                         m_acceptor;
    std::shared_ptr<async_bulk::Handler>  m_handler;
  };
} // namespace net;
