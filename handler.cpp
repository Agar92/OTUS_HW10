#include <boost/asio/streambuf.hpp>
#include <iostream>
#include <chrono>
#include <fstream>
#include <sstream>
#include <filesystem>

#include <handler.h>

namespace async_bulk
{
  Handler::Handler(std::size_t n, short unsigned int port)
    : m_default_size{n},
      m_port(port)
  {
    std::filesystem::create_directory("log");
    m_threads[0] = std::thread(&Handler::consoleWriter, this);
    m_threads[1] = std::thread(&Handler::fileWriter, this);
    m_threads[2] = std::thread(&Handler::fileWriter, this);
  }
    
  Handler::~Handler()
  {
    stop();
    for(auto& th : m_threads)
      th.join();
  }
  
  void Handler::handle(std::string_view sv)
  {
    std::stringstream input(sv.data());
    std::string line;
    std::unique_lock<std::mutex> lock(m_mutex);
    lock.unlock();
    while(std::getline(input, line))
    {
      if(line == "{")
      {
        lock.lock();
        if(!m_block_started)
        {
          m_block_started = true;
          lock.unlock();
          write();
        }
        else lock.unlock();
        continue;
      }
      if(line == "}")
      {
        lock.lock();
        if(m_block_started)
        {
          m_block_started = false;
          lock.unlock();
          write();
        }
        else lock.unlock();
        continue;
      }
      lock.lock();
      m_lines << line << " ";
      m_counter++;
      if(!m_block_started)
      {
        if(m_counter == m_default_size)
        {
          lock.unlock();
          write();
        }
      }
    }
  }
  
  void Handler::stop()
  {
    if(m_counter > 0) write();
    std::unique_lock<std::mutex> lock(m_mutex);
    for(auto& th : m_threads)
    {
      m_threadStates[th.get_id()].m_type = StateType::Terminating;
      m_threadStates[th.get_id()].m_done = false;
    }
    m_events.push(Event::Terminate);
    m_cv.wait(lock, [&]{
      return
        m_threadStates[m_threads[0].get_id()].m_done &&
        m_threadStates[m_threads[1].get_id()].m_done &&
        m_threadStates[m_threads[2].get_id()].m_done;
    });
    for(auto& th : m_threads)
	  m_threadStates[th.get_id()].m_done = false;
    m_events.pop();
  }
  
  void Handler::write()
  {
    if(m_counter == 0) return;
    std::unique_lock<std::mutex> lock(m_mutex);
    for(auto& th : m_threads)
    {
      m_threadStates[th.get_id()].m_type = StateType::Writing;
      m_threadStates[th.get_id()].m_done = false;
    }
    m_events.push(Event::Write);
    m_cv.wait(lock, [&]{
      return
        m_threadStates[m_threads[0].get_id()].m_done &&
        m_threadStates[m_threads[1].get_id()].m_done &&
        m_threadStates[m_threads[2].get_id()].m_done;
    });
    for(auto& th : m_threads)
      m_threadStates[th.get_id()].m_done = false;
    m_events.pop();
    m_counter = 0;
    m_lines.str("");
  }
  
  void Handler::consoleWriter()
  {
    auto id = std::this_thread::get_id();
    {
      std::unique_lock<std::mutex> lock{m_mutex};
      m_threadStates[id].m_type = StateType::Pending;
      m_threadStates[id].m_done = false;
    }
    Event event;   
    do
    {
      {
        std::unique_lock<std::mutex> lock{m_mutex};
        if (m_events.empty()|| m_threadStates[id].m_done)
        {
          lock.unlock();
          std::this_thread::sleep_for(std::chrono::milliseconds(10));
          continue;
        }
        event = m_events.front();
      }
      std::unique_lock<std::mutex> lock(m_mutex);
      if(event == Event::Write)
      {
        std::cout << "bulk: " << m_lines.str() << "\n";
        m_threadStates[id].m_type = StateType::Pending;
        m_threadStates[id].m_done = true;
        m_cv.notify_all();
        lock.unlock();
        continue;
      }
      if(event == Event::Terminate)
      {
        m_threadStates[id].m_type = StateType::Stopped;
        m_threadStates[id].m_done = true;
        m_cv.notify_all();
        lock.unlock();
        return;
      }
    }
    while(true);
  }
  
  void Handler::fileWriter()
  {
    auto id = std::this_thread::get_id();
    {
      std::unique_lock<std::mutex> lock{m_mutex};
      m_threadStates[id].m_type = StateType::Pending;
      m_threadStates[id].m_done = false;
    }
    std::stringstream fname;
    std::ofstream file;
    Event event;
    do
    {
      {
        std::unique_lock<std::mutex> lock{m_mutex};
        if (m_events.empty()|| m_threadStates[id].m_done)
        {
          lock.unlock();
          std::this_thread::sleep_for(std::chrono::milliseconds(10));
          continue;
        }
        event = m_events.front();
      }
      std::unique_lock<std::mutex> lock(m_mutex);
      if(event == Event::Write)
      {
        lock.unlock();
        fname << "log/bulk-" << id << "-" << std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        file.open (fname.str(), std::ios::out);
        fname.str("");
        lock.lock();
        file << "bulk: " << m_lines.str() << "\n";
        m_threadStates[id].m_type = StateType::Pending;
        m_threadStates[id].m_done = true;
        m_cv.notify_all();
        lock.unlock();
        file.close();
        continue;
      }
      if(event == Event::Terminate)
      {
        m_threadStates[id].m_type = StateType::Stopped;
        m_threadStates[id].m_done = true;
        m_cv.notify_all();
        lock.unlock();
        return;
      }
    }
    while(true);
  }
} // namespace async_bulk
