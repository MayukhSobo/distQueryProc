#include "driver.hpp"
#include "file_processor.hpp"
#include "save_task.hpp"
#include <filesystem>
#include <iostream>
#include <thread>
#include <vector>

void run_driver(const std::string &base_path,
                const std::vector<int> &engine_ports) {
  // Enumerate and filter files in the base path
  std::vector<std::string> file_names;
  for (const auto &entry : std::filesystem::directory_iterator(base_path)) {
    if (entry.is_regular_file() && entry.path().extension() == ".csv") {
      file_names.push_back(
          entry.path().filename().string()); // Use only the file name
    }
  }

  // Debug: Print valid file names
  // std::cout << "Files to be sent to the engines:\n";
  // for (const auto &name : file_names) {
  //   std::cout << name << std::endl;
  // }

  // Store results across all engines
  std::map<int, YearData> aggregated_results;

  // Process files across multiple engines using threads
  std::vector<std::thread> threads;
  size_t file_count_per_engine = file_names.size() / engine_ports.size();

  // Create a mutex to protect shared access to aggregated_results
  std::mutex results_mutex;

  for (size_t i = 0; i < engine_ports.size(); ++i) {
    // Get a subset of file names for each engine
    std::vector<std::string> engine_file_names(
        file_names.begin() + i * file_count_per_engine,
        (i == engine_ports.size() - 1)
            ? file_names.end()
            : file_names.begin() + (i + 1) * file_count_per_engine);

    // Launch a thread to handle communication with each engine
    threads.emplace_back([=, &aggregated_results, &results_mutex]() {
      std::map<int, YearData> local_results;

      send_request_to_engine(base_path, engine_file_names, "127.0.0.1",
                             engine_ports[i], local_results);

      // Lock the results mutex before updating the shared aggregated_results
      std::lock_guard<std::mutex> lock(results_mutex);
      for (const auto &result : local_results) {
        auto &data = aggregated_results[result.first];
        // Summing the count and sum for final aggregation
        data.min = std::min(data.min, result.second.min);
        data.max = std::max(data.max, result.second.max);
        data.sum += result.second.sum;
        data.count += result.second.count;
      }
    });
  }

  // Wait for all threads to finish
  for (auto &thread : threads) {
    thread.join();
  }

  // Save results to output.csv
  save_results_to_file(aggregated_results);

  std::cout << "Processing complete, results are saved in output.csv."
            << std::endl;
}
