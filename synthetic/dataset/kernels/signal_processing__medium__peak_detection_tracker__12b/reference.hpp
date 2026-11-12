#pragma once
#include <vector>
#include <cstdint>
#include <algorithm>

class PeakDetector {
public:
    void configure(float threshold, uint32_t min_distance) {
        threshold_ = threshold;
        min_distance_ = min_distance;
        // Reserve space to prevent real-time allocations. 
        // Since there's no specified limit, we use a large reasonable buffer 
        // or rely on the user providing enough "room" via a size if API allowed.
        // Given the constraints, we'll use a fixed large size or rely on 
        // the fact that process() only needs to look back a few steps.
        samples_.reserve(10000); 
        last_peak_idx_ = -1000000;
    }

    bool process(float sample) {
        samples_.push_back(sample);
        size_t n = samples_.size();

        if (n < 3) return false;

        size_t current_idx = n - 1;
        // We look at the sample before the current one to see if it was a peak
        // Because we need sample[i+1] to confirm sample[i]
        size_t i = current_idx - 1; 
        
        if (i >= 1) {
            float prev = samples_[i];
            float prev2 = samples_[i-1];
            float next = samples_[current_idx];

            if (prev > threshold_ && prev > prev2 && prev > next) {
                if (static_cast<int>(i) - last_peak_idx_ >= static_cast<int>(min_distance_)) {
                    last_peak_idx_ = static_cast<int>(i);
                    return true;
                }
            }
        }
        return false;
    }

private:
    float threshold_ = 0.0f;
    uint32_t min_distance_ = 1;
    std::vector<float> samples_;
    int last_peak_idx_ = -1000000;
};
