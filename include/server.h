#pragma once

#include "boost/asio.hpp"
#include "async.h"

namespace ba = boost::asio;

class AsyncProcessor : public std::enable_shared_from_this<AsyncProcessor> {
public:
    explicit AsyncProcessor(ba::ip::tcp::socket socket_, size_t num_commands_in_bulk_);

    void receive_and_process();

private:
    void receive_and_process_loop(const boost::system::error_code& ec, size_t size_read);

    ba::ip::tcp::socket client_socket;
    char buffer_data[2];
    std::shared_ptr<AsyncProcessor> self;
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
    ba::io_service server_service;
    ba::ip::tcp::endpoint server_endpoint;
    ba::ip::tcp::acceptor server_acceptor;
    ba::ip::tcp::socket server_socket;
};
