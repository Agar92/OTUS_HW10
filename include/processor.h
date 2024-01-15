#pragma once

#include <iostream>
#include <mutex>
#include "logger.h"
#include "commands.h"


class GlobalCommandProcessor {
public:
    GlobalCommandProcessor(GlobalCommandProcessor const&) = delete;

    GlobalCommandProcessor& operator=(GlobalCommandProcessor const&) = delete;

    static GlobalCommandProcessor& get_processor() {
        static GlobalCommandProcessor processor;
        return processor;
    }

    void process_1_command(const std::string& command);
    void set_bulk(int num_commands_in_bulk_);
    void force_complete_bulk();
private:
    std::mutex data_mutex;
    int num_commands_in_bulk = 0;
    int total_commands = 0;
    Commands accumulator;
    GlobalCommandProcessor() = default;
    ~GlobalCommandProcessor();
};

class CommandProcessor {
public:
    explicit CommandProcessor(int num_commands_in_bulk_);

    void process_data(const std::string& data);

    void process_commands(std::istream& source_stream, bool clear_after_end = true);

    void print_statistics(std::ostream& output_stream);

    ~CommandProcessor();

private:
    void process_1_command(const std::string& command);

    Commands accumulator;
    std::mutex data_mutex;
    std::mutex process_mutex;
    std::string buffer;
    int num_brackets = 0;
    int num_commands_in_bulk = 0;
    int total_lines = 0;
    int total_commands = 0;
    int total_blocks = 0;
};
