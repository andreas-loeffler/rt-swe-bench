#pragma once
#include <cstddef>
#include <algorithm>
#include <cmath>
#include <vector>

class SignalPeakAccumulator {
public:
    void configure(size_t batch_size) {
        batch_size_ = batch_size;
        current_peak_ = 0.0f;
        sample_count_ = 0;
    }

    float process(float sample) noexcept {
        // The first sample is processed at count 0
        // The batch is complete when count reaches batch_size - 1
        // and the final sample is processed.
        
        float abs_sample = std::abs(sample);
        if (abs_sample > current_peak_) {
            current_peak_ = abs_sample;
        }

        sample_count_++;

        if (sample_count_ >= batch_size_) {
            float result = current_peak_;
            // Reset for next batch
            current_peak_ = 0.0f;
            sample_count_ = 0;
            return result;
        }

        return 0.0f;
    }

private:
    size_t batch_size_ = 0;
    size_t sample_count_ = 0;
    float current_peak_ = 0.0f;
};
