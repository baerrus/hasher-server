//
// Copyright (c) 2024 Vlad Troyanker
//


#include "hasher-server.h"



int main(int argc, char* argv[])
{
  try
  {
    const auto port = 8000;
    TcpServer server(port);

    server.serve();

  }
  catch (std::exception& e)
  {
    std::cerr << "** main exception: " << e.what() << "\n";
  }

  return 0;
}