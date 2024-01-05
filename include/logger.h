#pragma once

#include <iostream>
#include <sstream>

#include <vector>
#include <string>
#include <map>
#include <queue>

#include <functional>

#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

class Logger {
public:
    Logger(Logger const&) = delete;

    Logger& operator=(Logger const&) = delete;

    static Logger& get_logger() {
        static Logger logger;
        return logger;
    }

    void task_runner(std::mutex& tasks_mutex,
                     std::condition_variable& condition,
                     std::queue<std::function<void()>>& tasks);

    void reserve_thread_for_stdout();

    void add_threads_for_filewriters(int n);

    void log_to_cout(const std::string& content, size_t num_elements);

    void log_to_file(const std::string& base_file_name, const std::string& content, size_t num_elements);

    void print_statistics(std::ostream& output_stream);

    void suspend_work();
    
    void resume_work();

private:
    Logger() = default;

    std::string get_random_string(int len);

    ~Logger();

    struct Statistics {
        std::atomic<int> num_blocks = 0;
        std::atomic<int> num_commands = 0;
    };

    bool use_cout_thread = false;
    int num_file_threads = 0;

    std::vector<std::thread> filewriters_pool;
    std::map<decltype(std::this_thread::get_id()), int> filewriters_pool_ids;
    std::unique_ptr<std::thread> stdout_thread = nullptr;
    Statistics cout_statistics;
    std::map<int, Statistics> file_statistics;

    std::queue<std::function<void()>> cout_tasks;
    std::queue<std::function<void()>> file_tasks;
    std::mutex file_tasks_mutex;
    std::mutex cout_tasks_mutex;
    std::condition_variable file_tasks_condition;
    std::condition_variable cout_tasks_condition;

    bool working = true;
};
