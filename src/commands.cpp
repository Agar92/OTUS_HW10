#include <fstream>
#include <sstream>
#include "commands.h"
#include "logger.h"

void Commands::add_command(const std::string& command) {
    if (empty()) {
        using namespace std::chrono;
        bulk_first_command_time = duration_cast<seconds>(system_clock::now().time_since_epoch()).count();
    }
    commands.emplace_back(command);
}

void Commands::clear() {
    commands.clear();
}

void Commands::log_commands() {
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

void Commands::log_commands_and_clear() {
    log_commands();
    clear();
}

size_t Commands::size() {
    return commands.size();
}

bool Commands::empty() {
    return commands.empty();
}
