#ifndef DRIVER_HPP
#define DRIVER_HPP

#include <limits> // Include this header for std::numeric_limits
#include <map>
#include <string>
#include <vector>

// Structure to hold year-wise min, max, sum, and count
struct YearData {
  int min = std::numeric_limits<int>::max();
  int max = std::numeric_limits<int>::min();
  int sum = 0;
  int count = 0;
};

// Declaration of main function to run the driver logic
void run_driver(const std::string &base_path,
                const std::vector<int> &engine_ports);

#endif // DRIVER_HPP
