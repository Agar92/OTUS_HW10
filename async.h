#pragma once

#include <handler.cpp>
#include <server.h>

namespace async_bulk
{
  void run_server(std::size_t bulk, short unsigned int port)
  {
    try
    {
      std::shared_ptr<Handler> h(new Handler(bulk, port));
      boost::asio::io_context io_context;
      net::server server(port, io_context, h);
      io_context.run();
    }
    catch(std::exception& e) { std::printf("exception: %s\n", e.what()); }
  }
} // namespace async_bulk
