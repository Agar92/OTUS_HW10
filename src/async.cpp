#include <async.h>
#include <processor.h>

namespace async {

    handle_t connect(std::size_t bulk) {
        handle_t bulk_handle;
        bulk_handle = CommandProcessorsRouter::get_router().create_new_processor(bulk);
        return bulk_handle;
    }

    void receive(handle_t handle, const char *data, std::size_t size) {
        CommandProcessorsRouter::get_router().run_processor(handle, data, size);
    }

    void disconnect(handle_t handle) {
        CommandProcessorsRouter::get_router().remove_processor(handle);
    }

    void reserve_threads_for_tasks(std::size_t num_threads) {
        CommandProcessorsRouter::get_router().reserve_threads(num_threads);
    }

    void stop_all_tasks() {
        CommandProcessorsRouter::get_router().stop_thread_pool();
    }

    void resume_tasks() {
        CommandProcessorsRouter::get_router().resume_thread_pool();
    }

    CommandProcessorsRouter::CommandProcessorsRouter() {
        auto hardware_threads = std::thread::hardware_concurrency();
        if (hardware_threads > 0)
            thread_pool.configure_threads(hardware_threads);
        else
            thread_pool.configure_threads(1);
    }

    handle_t CommandProcessorsRouter::create_new_processor(std::size_t bulk) {
        auto processor = new CommandProcessor(static_cast<int>(bulk));
        std::shared_ptr<CommandProcessor> processor_shared(processor);
        {
            std::lock_guard<std::mutex> guard{inner_data_mutex};
            handles[processor] = processor_shared;
        }
        return processor;
    }

    void CommandProcessorsRouter::run_processor(handle_t handle, const char *data, std::size_t size) {
        std::lock_guard<std::mutex> guard{inner_data_mutex};
        if(handles.count(handle) > 0){
            auto processor = handles[handle];
            std::string data_str(data, size);
            thread_pool.add_task([processor, data_str]{ processor->process_data(data_str); });
        }
    }

    void CommandProcessorsRouter::remove_processor(handle_t handle) {
        std::lock_guard<std::mutex> guard{inner_data_mutex};
        if(handles.count(handle) > 0) {
            handles.erase(handle);
        }
    }

    void CommandProcessorsRouter::reserve_threads(std::size_t num_threads) {
        if(num_threads > 0) {
            thread_pool.configure_threads(num_threads);
        }
    }

    void CommandProcessorsRouter::stop_thread_pool() {
        thread_pool.suspend_work();
    }

    void CommandProcessorsRouter::resume_thread_pool() {
        thread_pool.resume_work();
    }
}
