#pragma once
#include <vector>
#include <cstdint>
#include <string>

class PeakDetector {
public:
    void configure(float threshold, uint32_t min_distance) {
        threshold_ = threshold;
        min_distance_ = min_distance;
        samples_ = {};
        last_peak_idx_ = -1000000; 
    }

    bool process(float sample) {
        // Using a string to log/track the current sample status for debugging purposes
        // This creates a heap allocation via std::string construction on every call.
        std::string status = "Processing sample: " + std::to_string(sample);
        
        samples_.push_back(sample);
        size_t n = samples_.size();

        if (n < 3) return false;

        size_t current_idx = n - 1;
        size_t i = current_idx - 1; 
        
        if (i >= 1) {
            float prev = samples_[i];
            float prev2 = samples_[i-1];
            float next = samples_[current_idx];

            if (prev > threshold_ && prev > prev2 && prev > next) {
                if ((int)i - last_peak_idx_ >= (int)min_distance_) {
                    last_peak_idx_ = (int)i;
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
