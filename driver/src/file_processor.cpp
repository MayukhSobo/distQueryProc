#include "file_processor.hpp"
#include "message.pb.h"
#include <arpa/inet.h>
#include <iostream>
#include <netinet/in.h>
#include <stdexcept>
#include <sys/socket.h>
#include <unistd.h>

void send_request_to_engine(const std::string &base_path,
                            const std::vector<std::string> &file_paths,
                            const std::string &engine_ip, int port,
                            std::map<int, YearData> &aggregated_results) {
  // Create the FileRequest message using Protobuf
  FileRequest request;
  request.set_base_path(base_path);

  // Concatenate the file names into a single string with newlines separating
  // them
  std::string file_list;
  for (const auto &file_path : file_paths) {
    file_list += file_path + "\n"; // Add each file to the list
  }
  request.set_file_list(file_list); // Use set_file_list

  // Serialize the request to send it to the engine
  std::string serialized_request;
  if (!request.SerializeToString(&serialized_request)) {
    throw std::runtime_error("Failed to serialize request");
  }

  // Create socket and connect to the engine
  int sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0) {
    throw std::runtime_error("Socket creation failed");
  }

  struct sockaddr_in serv_addr;
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(port);

  if (inet_pton(AF_INET, engine_ip.c_str(), &serv_addr.sin_addr) <= 0) {
    close(sock);
    throw std::runtime_error("Invalid address/ Address not supported");
  }

  if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
    close(sock);
    throw std::runtime_error("Connection failed");
  }

  // Send the serialized request to the engine
  send(sock, serialized_request.data(), serialized_request.size(), 0);

  // Receive the response from the engine
  char buffer[4096] = {0};
  int valread = read(sock, buffer, sizeof(buffer));
  if (valread <= 0) {
    close(sock);
    throw std::runtime_error("Failed to read response from engine");
  }

  // Deserialize the response
  FileResponse response;
  if (!response.ParseFromArray(buffer, valread)) {
    close(sock);
    throw std::runtime_error("Failed to parse response");
  }

  // Aggregate the year-wise statistics (sum, count, min, max)
  for (const auto &yearly_result : response.yearly_results()) {
    int year = yearly_result.year();
    auto &data = aggregated_results[year];

    // Aggregate the sum, count, min, and max values from each engine
    data.min = std::min(data.min, yearly_result.min_score());
    data.max = std::max(data.max, yearly_result.max_score());
    data.sum += yearly_result.sum_score(); // Use sum_score (int64)
    data.count += yearly_result.count();   // Use count (int64)
  }

  // Close the socket connection
  close(sock);
}
