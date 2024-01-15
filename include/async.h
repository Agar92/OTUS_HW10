#pragma once

#include <cstddef>
#include <unordered_map>
#include <mutex>
#include <processor.h>
#include <threadpool.h>

namespace async {

    using handle_t = void*;

    handle_t connect(std::size_t bulk);

    void receive(handle_t handle, const char *data, std::size_t size);

    void disconnect(handle_t handle);

    void reserve_threads_for_tasks(std::size_t num_threads);

    void stop_all_tasks();

    class CommandProcessorsRouter {
    public:
        static CommandProcessorsRouter& get_router() {
            static CommandProcessorsRouter router;
            return router;
        }

        handle_t create_new_processor(std::size_t bulk);
        void remove_processor(handle_t handle);
        void run_processor(handle_t handle, const char *data, std::size_t size);
        void reserve_threads(std::size_t num_threads);
        void stop_thread_pool();
        void resume_thread_pool();

    private:
        CommandProcessorsRouter();

        std::unordered_map<handle_t, std::shared_ptr<CommandProcessor>> handles;
        std::mutex inner_data_mutex;
        ThreadPool thread_pool;
    };
}
