#include "server.h"
#include "async.h"
#include "boost/asio.hpp"
#include <iostream>
#include "command_processor.h"
#include <thread>

namespace ba = boost::asio;

AsyncProcessor::AsyncProcessor(ba::ip::tcp::socket socket_, size_t num_commands_in_bulk_) :
        client_socket{std::move(socket_)},
        buffer_data{0},
        num_commands_in_bulk{num_commands_in_bulk_} {
    handle = async::connect(num_commands_in_bulk);
}

void AsyncProcessor::receive_and_process() {
    // https://stackoverflow.com/questions/34719233/why-capture-this-as-well-as-shared-pointer-to-this-in-lambdas/34719861
    self = shared_from_this();
    client_socket.async_read_some(ba::buffer(buffer_data),
                                  [this](const boost::system::error_code& ec, size_t size_read) {
                                      receive_and_process_loop(ec, size_read);
                                  });
}


void AsyncProcessor::receive_and_process_loop(const boost::system::error_code& ec, size_t size_read) {
    if (ec) {
        client_socket.close();
        async::disconnect(handle);
        self = nullptr;
        return;
    }
    async::receive(handle, buffer_data, size_read);
    client_socket.async_read_some(ba::buffer(buffer_data),
                                  [this](const boost::system::error_code& ec, size_t size_read) {
                                      receive_and_process_loop(ec, size_read);
                                  });
}


Server::Server(int port_, size_t num_commands_in_bulk_, size_t num_threads_) :
        port{port_},
        num_commands_in_bulk{num_commands_in_bulk_},
        num_threads{num_threads_},
        server_endpoint{ba::ip::tcp::v4(), static_cast<unsigned short>(port_)},
        server_acceptor{server_service, server_endpoint},
        server_socket{server_service} {
    GlobalCommandProcessor::get_processor().set_bulk(static_cast<int>(num_commands_in_bulk_));
}


void Server::run() {
    ba::signal_set signal_set(server_service, SIGTERM, SIGINT); // interrupt, kill
    signal_set.async_wait(
            [this](const boost::system::error_code& ec, int signal_number) {
                server_service.stop();
            });
    accept();

    server_service.run();
    std::vector<std::thread> threads;
    for(size_t i = 1; i < num_threads; i++)
        threads.emplace_back([this](){ server_service.run(); });
    for(auto& t: threads)
        t.join();
}


void Server::accept() {
    server_acceptor.async_accept(server_socket, [this](const boost::system::error_code& ec) {
        if (!ec) {
            auto client_handler_ptr = std::make_shared<AsyncProcessor>(std::move(server_socket), num_commands_in_bulk);
            client_handler_ptr->receive_and_process();
        }
        accept();
    });
}
