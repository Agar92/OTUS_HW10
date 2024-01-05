#include <fstream>
#include <sstream>
#include "command_accumulator.h"
#include "logger.h"

void CommandAccumulator::add_command(const std::string& command) {
    if (empty()) {
        using namespace std::chrono;
        bulk_first_command_time = duration_cast<seconds>(system_clock::now().time_since_epoch()).count();
    }
    commands.emplace_back(command);
}

void CommandAccumulator::clear() {
    commands.clear();
}

void CommandAccumulator::log_commands() {
    if (empty()) return; // log nothing
    std::ostringstream ss;
    ss << "bulk: ";
    for (size_t i = 0; i < commands.size(); i++) {
        if(i > 0) ss << ", ";
        ss << commands[i];
    }
    ss << "\n";
    Logger::get_logger().log_to_cout(ss.str(), size());
    auto base_file_name = get_log_file_basename(bulk_first_command_time);
    Logger::get_logger().log_to_file(base_file_name, ss.str(), size());
}

void CommandAccumulator::log_commands_and_clear() {
    log_commands();
    clear();
}

size_t CommandAccumulator::size() {
    return commands.size();
}

bool CommandAccumulator::empty() {
    return commands.empty();
}
