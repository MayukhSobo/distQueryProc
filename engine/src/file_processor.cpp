#include "file_processor.hpp"
#include <algorithm>
#include <fstream>
#include <limits>
#include <numeric>
#include <sstream>
#include <xsimd/xsimd.hpp>

using namespace xsimd;

void process_chunk(const std::vector<int> &data, batch<int> &min_vec,
                   batch<int> &max_vec, batch<int> &sum_vec) {
  size_t i = 0;
  for (; i + batch<int>::size <= data.size(); i += batch<int>::size) {
    batch<int> current_vec = load_unaligned(&data[i]);
    min_vec = min(current_vec, min_vec);
    max_vec = max(current_vec, max_vec);
    sum_vec += current_vec;
  }

  // Handle remaining elements that do not fit into SIMD width
  for (; i < data.size(); ++i) {
    int value = data[i];
    min_vec = min(batch<int>::broadcast(value), min_vec);
    max_vec = max(batch<int>::broadcast(value), max_vec);
    sum_vec += batch<int>::broadcast(value);
  }
}
