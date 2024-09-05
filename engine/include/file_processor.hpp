#ifndef FILE_PROCESSOR_HPP
#define FILE_PROCESSOR_HPP

#include <string>
#include <vector>
#include <xsimd/xsimd.hpp>

void process_chunk(const std::vector<int> &data, xsimd::batch<int> &min_vec,
                   xsimd::batch<int> &max_vec, xsimd::batch<int> &sum_vec);
#endif // FILE_PROCESSOR_HPP
