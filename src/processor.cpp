#include <iostream>
#include <sstream>
#include <processor.h>

void GlobalCommandProcessor::process_1_command(const std::string& current_command) {
    if (current_command == "{" or current_command == "}")
        throw std::logic_error("Brackets not allowed in global processor");

    std::lock_guard<std::mutex> guard{data_mutex};
    accumulator.add_command(current_command);
    total_commands++;
    if (accumulator.size() == num_commands_in_bulk) {
        accumulator.log_commands_and_clear();
    }
}

void GlobalCommandProcessor::force_complete_bulk() {
    if (accumulator.size() > 0) {
        accumulator.log_commands_and_clear();
    }
}

void GlobalCommandProcessor::set_bulk(int num_commands_in_bulk_) {
    num_commands_in_bulk = num_commands_in_bulk_;
}

GlobalCommandProcessor::~GlobalCommandProcessor() {
    std::lock_guard<std::mutex> guard{data_mutex};
    accumulator.log_commands_and_clear();
}

CommandProcessor::CommandProcessor(int num_commands_in_bulk_) : num_commands_in_bulk(num_commands_in_bulk_) {}

void CommandProcessor::process_data(const std::string& data) {
    std::lock_guard<std::mutex> guard{data_mutex};
    buffer.append(data);
    auto last_space_pos = buffer.rfind(' ');
    if (last_space_pos == std::string::npos)
        last_space_pos = buffer.rfind('\n');

    if (last_space_pos != std::string::npos) {
        std::istringstream ss(buffer.substr(0, last_space_pos + 1));
        process_commands(ss, /*clear_after_end=*/false);
        buffer.erase(0, last_space_pos + 1);
    }
}

void CommandProcessor::process_1_command(const std::string& current_command) {
    if (current_command == "{") {
        if (num_brackets == 0) {
            GlobalCommandProcessor::get_processor().force_complete_bulk(); // force complete global bulk
        }
        num_brackets++;
    } else if (current_command == "}") {
        if (num_brackets <= 0) {
            std::cerr << "Warning: Unexpected bracket, ignoring" << std::endl;
        } else {
            num_brackets--;
            if (num_brackets == 0) {
                accumulator.log_commands_and_clear();
                total_blocks++;
            }
        }
    } else if (num_brackets > 0) { // command in {...}
        accumulator.add_command(current_command);
        total_commands++;
    } else { // num_brackets == 0, so redirect to global processor
        GlobalCommandProcessor::get_processor().process_1_command(current_command);
    }
}

void CommandProcessor::process_commands(std::istream& source_stream, bool clear_after_end) {
    std::lock_guard<std::mutex> guard{process_mutex};
    std::string current_command;

    while (source_stream >> current_command) {
        total_lines++;
        process_1_command(current_command);
    }

    if (clear_after_end && !accumulator.empty() && num_brackets == 0) {
        accumulator.log_commands_and_clear();
        total_blocks++;
    }
}

void CommandProcessor::print_statistics(std::ostream& output_stream) {
    std::lock_guard<std::mutex> guard{data_mutex};
    std::lock_guard<std::mutex> guard2{process_mutex};
    output_stream << "main thread - "
                  << total_lines << " lines, "
                  << total_commands << " commands, "
                  << total_blocks << " blocks" << std::endl;
}

CommandProcessor::~CommandProcessor() {
    std::lock_guard<std::mutex> guard{data_mutex};
    std::lock_guard<std::mutex> guard2{process_mutex};
    accumulator.log_commands_and_clear();
}
