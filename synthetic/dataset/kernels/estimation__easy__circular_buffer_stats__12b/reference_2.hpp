#pragma once
#include <cstddef>
#include <vector>
#include <algorithm>

/**
 * CircularBufferStats implements a sliding window median.
 * To satisfy the hard real-time constraint of not allocating during process(),
 * we maintain a fixed-size buffer for the data and a pre-allocated workspace
 * for sorting.
 * 
 * Time Complexity: O(N) per process() call where N is capacity.
 * Space Complexity: O(N) allocated during configure().
 */
class CircularBufferStats {
public:
    void configure(size_t capacity) {
        capacity_ = capacity;
        buffer_.assign(capacity, 0.0);
        sorted_workspace_.assign(capacity, 0.0);
        write_idx_ = 0;
        count_ = 0;
    }

    double process(double value) {
        // Store value in circular buffer
        buffer_[write_idx_] = value;
        write_idx_ = (write_idx_ + 1) % capacity_;
        
        if (count_ < capacity_) {
            count_++;
        }

        // Copy current elements to workspace to avoid mutating the circular order
        // Using std::copy is O(N) and does not allocate.
        std::copy(buffer_.begin(), buffer_.begin() + count_, sorted_workspace_.begin());

        // Partially sort to find the median. 
        // nth_element is O(N) on average.
        if (count_ % 2 == 1) {
            // Odd: middle element
            size_t mid = count_ / 2;
            std::nth_element(sorted_workspace_.begin(), 
                             sorted_workspace_.begin() + mid, 
                             sorted_workspace_.begin() + count_);
            return sorted_workspace_[mid];
        } else {
            // Even: average of two middle elements
            size_t mid1 = count_ / 2 - 1;
            size_t mid2 = count_ / 2;
            
            std::nth_element(sorted_workspace_.begin(), 
                             sorted_workspace_.begin() + mid1, 
                             sorted_workspace_.begin() + count_);
            double val1 = sorted_workspace_[mid1];
            
            // Find the smallest element in the remaining range [mid1 + 1, count_)
            double val2 = *std::min_element(sorted_workspace_.begin() + mid1 + 1, 
                                            sorted_workspace_.begin() + count_);
            
            return (val1 + val2) * 0.5;
        }
    }

private:
    size_t capacity_ = 0;
    size_t write_idx_ = 0;
    size_t count_ = 0;
    std::vector<double> buffer_;
    std::vector<double> sorted_workspace_;
};
