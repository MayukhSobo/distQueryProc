#include "save_task.hpp"
#include <fstream>
#include <iostream>

void save_results_to_file(const std::map<int, YearData> &aggregated_results) {
  std::ofstream output_file("output.txt");

  if (!output_file.is_open()) {
    std::cerr << "Error: Unable to open output.txt for writing!" << std::endl;
    return;
  }

  // Write header, including the new 'Total' and 'Sum' columns for debugging
  // output_file << "Year,Min,Max,Avg\n";

  // Write data for each year
  for (const auto &[year, data] : aggregated_results) {
    long long avg =
        (data.count > 0) ? static_cast<long long>(data.sum) / data.count : 0LL;
    output_file << year << "," << data.min << "," << data.max << "," << avg
                << "\n"; // Added total and sum
  }

  output_file.close();
  // std::cout << "Results saved to output.txt" << std::endl;
}
