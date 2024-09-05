#ifndef SAVE_TASK_HPP
#define SAVE_TASK_HPP

#include "driver.hpp"
#include <map>
#include <string>

// Function to save results to a CSV file asynchronously
void save_results_to_file(const std::map<int, YearData> &aggregated_results);

#endif // SAVE_TASK_HPP
