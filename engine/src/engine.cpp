#include "engine.hpp"
#include "file_processor.hpp"
#include "message.pb.h"
#include <arpa/inet.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <netinet/in.h>
#include <numeric>
#include <sstream>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

struct YearData {
  int min = std::numeric_limits<int>::max();
  int max = std::numeric_limits<int>::min();
  int sum = 0;
  int count = 0;
};

using YearlyScores = std::map<int, YearData>; // Map for year-wise statistics

// Coroutine to process each file and yield (year, score) pairs
Generator process_file_coroutine(const std::string &file_name) {
  std::ifstream file(file_name);
  if (!file.is_open()) {
    std::cerr << "Error: Could not open the file " << file_name << std::endl;
    co_return;
  }

  std::string line;
  while (std::getline(file, line)) {
    std::stringstream ss(line);
    std::string student_id, batch_year_str, score_str;

    if (!std::getline(ss, student_id, ',') ||
        !std::getline(ss, batch_year_str, ',') ||
        !std::getline(ss, score_str, ',')) {
      std::cerr << "Invalid line format in file " << file_name << ": " << line
                << std::endl;
      continue; // Skip lines that don't match the expected format
    }

    try {
      int batch_year = std::stoi(batch_year_str);
      int score = std::stoi(score_str);
      co_yield std::make_pair(batch_year, score); // Yield a pair (year, score)
    } catch (const std::invalid_argument &e) {
      std::cerr << "Invalid value in file " << file_name << ": " << line
                << std::endl;
      continue; // Skip invalid values
    }
  }
}

// Function to process chunked data by year using SIMD for min, max, sum
// calculations
void process_chunk_by_year(const std::vector<std::pair<int, int>> &data,
                           YearlyScores &yearly_scores) {
  std::map<int, std::vector<int>> scores_by_year;

  // Group scores by year
  for (const auto &entry : data) {
    int year = entry.first;
    int score = entry.second;
    scores_by_year[year].push_back(score);
  }

  for (auto &[year, scores] : scores_by_year) {
    auto &year_data = yearly_scores[year];

    // SIMD processing
    xsimd::batch<int> min_vec = xsimd::batch<int>::broadcast(year_data.min);
    xsimd::batch<int> max_vec = xsimd::batch<int>::broadcast(year_data.max);
    xsimd::batch<int> sum_vec = xsimd::batch<int>::broadcast(0);

    size_t i = 0;
    for (; i + xsimd::batch<int>::size <= scores.size();
         i += xsimd::batch<int>::size) {
      xsimd::batch<int> current_vec = xsimd::load_unaligned(&scores[i]);
      min_vec = xsimd::min(current_vec, min_vec);
      max_vec = xsimd::max(current_vec, max_vec);
      sum_vec += current_vec;
    }

    // Store SIMD results in arrays
    std::array<int, xsimd::batch<int>::size> min_array, max_array, sum_array;
    min_vec.store_unaligned(min_array.data());
    max_vec.store_unaligned(max_array.data());
    sum_vec.store_unaligned(sum_array.data());

    // Update min, max, sum with SIMD results
    year_data.min = std::min(
        year_data.min, *std::min_element(min_array.begin(), min_array.end()));
    year_data.max = std::max(
        year_data.max, *std::max_element(max_array.begin(), max_array.end()));
    year_data.sum += std::accumulate(sum_array.begin(), sum_array.end(), 0);

    // Handle remaining elements that do not fit into SIMD width
    for (; i < scores.size(); ++i) {
      int value = scores[i];
      year_data.min = std::min(year_data.min, value);
      year_data.max = std::max(year_data.max, value);
      year_data.sum += value;
    }

    // Update count
    year_data.count += scores.size();
  }
}

void engine_process(int port) {
  int server_fd, new_socket;
  struct sockaddr_in address;
  int opt = 1;
  int addrlen = sizeof(address);

  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
    perror("socket failed");
    exit(EXIT_FAILURE);
  }

  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt,
                 sizeof(opt))) {
    perror("setsockopt");
    exit(EXIT_FAILURE);
  }

  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(port);

  if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
    perror("bind failed");
    exit(EXIT_FAILURE);
  }

  if (listen(server_fd, 3) < 0) {
    perror("listen");
    exit(EXIT_FAILURE);
  }

  while (true) {
    std::cout << "Waiting for a connection..." << std::endl;
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address,
                             (socklen_t *)&addrlen)) < 0) {
      perror("accept");
      exit(EXIT_FAILURE);
    }

    std::cout << "Connection established!" << std::endl;

    // Receive data
    char buffer[4096];
    int bytes_received = read(new_socket, buffer, sizeof(buffer));
    std::string received_data(buffer, bytes_received);

    // Deserialize the received data
    FileRequest request;
    request.ParseFromString(received_data);

    std::string base_path = request.base_path();
    std::string file_list = request.file_list();

    YearlyScores yearly_scores;

    std::stringstream ss(file_list);
    std::string file_name;

    // Iterate through the file names and process each file
    while (std::getline(ss, file_name, '\n')) {
      std::string full_file_path =
          (std::filesystem::path(base_path) / file_name).string();

      try {
        Generator gen = process_file_coroutine(full_file_path);

        std::vector<std::pair<int, int>> scores; // Store (year, score) pairs
        while (true) {
          auto result = gen.next();
          if (result.first == -1 && result.second == -1) {
            break; // End of coroutine
          }
          scores.push_back(result);
        }

        // Process the scores per year here...
        process_chunk_by_year(scores, yearly_scores);
      } catch (const std::exception &e) {
        std::cerr << "Error processing file " << full_file_path << ": "
                  << e.what() << std::endl;
        continue;
      }
    }

    // Prepare the response
    FileResponse response;
    for (const auto &[year, data] : yearly_scores) {
      auto *yearly_result = response.add_yearly_results();
      yearly_result->set_year(year);
      yearly_result->set_min_score(data.min);
      yearly_result->set_max_score(data.max);
      yearly_result->set_sum_score(static_cast<int64_t>(data.sum)); // Use int64
      yearly_result->set_count(static_cast<int64_t>(data.count));   // Use int64
    }

    // Serialize and send the response
    std::string serialized_response;
    response.SerializeToString(&serialized_response);

    send(new_socket, serialized_response.data(), serialized_response.size(), 0);
    close(new_socket);
  }
}
