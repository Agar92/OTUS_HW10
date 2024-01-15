#include <sstream>
#include <iostream>
#include "server.h"
#include "logger.h"
#include "async.h"

int main(int argc, char *argv[]) {
    bool executed_correctly = true;
    std::stringstream whats_wrong;

    size_t num_commands_in_bulk = 0;
    int port = 0;

    if (argc != 3) {
        executed_correctly = false;
        whats_wrong << "Incorrect number of arguments";
    } else {
        try {
            port = std::stoi(argv[1]);
            if (port < 0) {
                whats_wrong << "Port must be non-negative";
                executed_correctly = false;
            }
        } catch (std::exception&) {
            executed_correctly = false;
            whats_wrong << "Port must be a number";
        }
        try {
            num_commands_in_bulk = std::stoul(argv[2]);
            if (num_commands_in_bulk == 0) {
                whats_wrong << "Number of commands in bulk must be positive";
                executed_correctly = false;
            }
        } catch (std::exception&) {
            executed_correctly = false;
            whats_wrong << "Number of commands in bulk must be a number";
        }
    }

    if (!executed_correctly) {
        std::cout << "Error: " << whats_wrong.str() << std::endl;
        std::cout << "Execute with two arguments: port and bulk_size, e.g.:" << std::endl;
        std::cout << argv[0] << " 9000 3" << std::endl;
        exit(0);
    }

    Logger::get_logger().reserve_thread_for_stdout();
    Logger::get_logger().add_threads_for_filewriters(2);
    async::reserve_threads_for_tasks(4);

    Server server(port, num_commands_in_bulk, /*num_threads=*/1);
    server.run();

    return 0;
}
