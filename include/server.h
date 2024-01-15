#pragma once

#include "boost/asio.hpp"
#include "async.h"

class Session : public std::enable_shared_from_this<Session> {
public:
    explicit Session(boost::asio::ip::tcp::socket socket_, size_t num_commands_in_bulk_);

    void receive_and_process();

private:
    void receive_and_process_loop(const boost::system::error_code& ec, size_t size_read);

    boost::asio::ip::tcp::socket client_socket;
    char buffer_data[2];
    std::shared_ptr<Session> self;
    size_t num_commands_in_bulk;
    async::handle_t handle;
};

class Server {
public:
    explicit Server(int port_, size_t num_commands_in_bulk_, size_t num_threads_);

    void run();

    void accept();


private:
    int port;
    size_t num_commands_in_bulk;
    size_t num_threads;
    boost::asio::io_context server_service;
    boost::asio::ip::tcp::endpoint server_endpoint;
    boost::asio::ip::tcp::acceptor server_acceptor;
    boost::asio::ip::tcp::socket server_socket;
};
