#pragma once
#include <vector>
#include <cmath>
#include <algorithm>

class ZeroCrossingRate {
public:
    void configure(size_t window_size) {
        buffer_.assign(window_size, 0.0f);
        size_ = window_size;
        write_pos_ = 0;
        full_ = false;
    }

    float push(float sample) {
        buffer_[write_pos_] = sample;
        write_pos_ = (write_pos_ + 1) % size_;
        if (!full_ && write_pos_ == 0) full_ = true;

        size_t available = full_ ? size_ : write_pos_;
        if (available < 2) return 0.0f;

        std::vector<float> window_data;
        window_data.reserve(available);

        for (size_t i = 0; i < available; ++i) {
            size_t idx = full_ ? (write_pos_ + i) % size_ : i;
            window_data.push_back(buffer_[idx]);
        }

        size_t count = 0;
        for (size_t i = 0; i < window_data.size() - 1; ++i) {
            if ((window_data[i] > 0 && window_data[i + 1] < 0) || 
                (window_data[i] < 0 && window_data[i + 1] > 0)) {
                count++;
            }
        }

        return static_cast<float>(count) / static_cast<float>(available - 1);
    }

private:
    std::vector<float> buffer_;
    size_t size_ = 0;
    size_t write_pos_ = 0;
    bool full_ = false;
};
