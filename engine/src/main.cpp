#include "engine.hpp"
#include <cstdlib>
#include <iostream>

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <port_number>" << std::endl;
    return 1;
  }

  // Get port number from command-line arguments
  int port_number = std::stoi(argv[1]);

  // Run the engine process with the given port number
  engine_process(port_number);

  return 0;
}
