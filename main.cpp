#include <iostream>

#include <async.h>

int main(int argc, char** argv)
{
  if(argc != 3)
  {
    std::cout << "block size and port are required as arguments\n";
    return 1;
  }
  int n = std::stoi(argv[1]);
  short unsigned int port = std::stoi(argv[2]);
  async_bulk::run_server(n, port);
  return 0;
}
