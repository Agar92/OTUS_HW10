#pragma once

#include <array>
#include <condition_variable>
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <string_view>
#include <thread>
#include <vector>

namespace async_bulk
{
  constexpr int threads_number = 3;
  
  enum class Event 
  {
    Sleep,
    Write,
    Terminate
  };
  
  enum class StateType
  {
    Pending,
    Writing,
    Terminating,
    Stopped
  };
  
  class State
  {
  public:
    StateType m_type{StateType::Pending};
    bool m_done{false};
  }; 
  
  class Handler
  {
  public:
    Handler(std::size_t, short unsigned int);
    ~Handler();
    
    void stop();
    void handle(std::string_view);
      
  private:
    void write();
    void consoleWriter();
    void fileWriter();
    
    std::size_t                             m_default_size{1};
    std::size_t                             m_counter{0};
    bool                                    m_block_started{false};
    std::map<std::thread::id, State>        m_threadStates;
    std::array<std::thread, threads_number> m_threads;
    std::mutex                              m_mutex;
    std::condition_variable                 m_cv;
    std::queue<Event>                       m_events;
    std::stringstream                       m_lines;
    short unsigned int                      m_port;
  };
    
} //namespace async_bulk
