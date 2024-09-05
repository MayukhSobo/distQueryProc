#include "driver.hpp"
#include <cstdlib>
#include <iostream>
#include <vector>

int main(int argc, char *argv[]) {
  // Get base path from environment variable FILE_PATHS
  const char *file_paths_env = std::getenv("FILE_PATHS");
  if (!file_paths_env) {
    std::cerr << "Error: FILE_PATHS environment variable not set." << std::endl;
    return 1;
  }

  std::string base_path = file_paths_env;

  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <engine_ports>" << std::endl;
    return 1;
  }

  // Collect engine ports from the command-line arguments
  std::vector<int> engine_ports;
  for (int i = 1; i < argc; ++i) {
    engine_ports.push_back(std::stoi(argv[i]));
  }

  // Run the driver logic
  run_driver(base_path, engine_ports);

  return 0;
}
