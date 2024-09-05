#ifndef FILE_PROCESSOR_HPP
#define FILE_PROCESSOR_HPP

#include "driver.hpp"
#include <map>
#include <string>
#include <vector>

// Function to send request to an engine and receive response
void send_request_to_engine(const std::string &base_path,
                            const std::vector<std::string> &file_paths,
                            const std::string &engine_ip, int port,
                            std::map<int, YearData> &aggregated_results);

#endif // FILE_PROCESSOR_HPP
