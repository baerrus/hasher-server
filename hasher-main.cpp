//
// Copyright (c) 2024 Vlad Troyanker
//


#include "hasher-server.h"
#include "hasher-config.h"


int main(int argc, char* argv[])
{
  try
  {
    Configuration config;
    
    TcpServer server(config);

    server.serve();

      return 0;
  }
  catch (std::exception& e)
  {
    std::cerr << "** main exception: " << e.what() << "\n";
  }
}