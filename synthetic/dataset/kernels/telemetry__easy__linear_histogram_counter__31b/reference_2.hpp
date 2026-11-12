#pragma once
#include <cstdint>
#include <vector>
#include <algorithm>

class LinearHistogramCounter {
public:
    void configure(int32_t min_value, int32_t max_value) {
        // Precondition: max_value >= min_value
        // We use a vector to manage the heap allocation for the bins.
        // The number of bins is (max_value - min_value + 1).
        size_t range = static_cast<size_t>(max_value) - static_cast<size_t>(min_value) + 1;
        
        // Ensure range is reasonable to prevent overflow or extreme allocation,
        // though the prompt doesn't specify limits, we follow standard vector allocation.
        bins_.clear();
        bins_.resize(range, 0);
        min_val_ = min_value;
        max_val_ = max_value;
    }

    void increment(int32_t value) {
        // Real-time safe: No allocation, no locks, no exceptions.
        if (value >= min_val_ && value <= max_val_) {
            // Mapping value to index: index = value - min_val_
            bins_[static_cast<size_t>(value - min_val_)]++;
        }
    }

    uint64_t get_count(int32_t value) const {
        // Real-time safe: No allocation, no locks, no exceptions.
        if (value < min_val_ || value > max_val_) {
            return 0;
        }
        return static_cast<uint64_t>(bins_[static_cast<size_t>(value - min_val_)]);
    }

    void reset() {
        // Real-time safe: No allocation, no locks, no exceptions.
        // std::fill is generally O(N) but does not allocate or lock.
        std::fill(bins_.begin(), bins_.end(), 0);
    }

private:
    std::vector<uint64_t> bins_;
    int32_t min_val_ = 0;
    int32_t max_val_ = 0;
};
