#pragma once

#include <string>
#include <vector>
#include <chrono>
#include <sstream>
#include <thread>
#include <filesystem>

class CommandAccumulator {
public:
    CommandAccumulator(){std::filesystem::create_directory("log");}
    void add_command(const std::string& command);
    void clear();
    void log_commands();
    void log_commands_and_clear();
    size_t size();
    bool empty();

private:
    std::vector<std::string> commands;
    long long int bulk_first_command_time = 0;

    static std::string get_log_file_basename(long long int first_command_time) {
        return "log/bulk" + std::to_string(first_command_time);
    }
};
