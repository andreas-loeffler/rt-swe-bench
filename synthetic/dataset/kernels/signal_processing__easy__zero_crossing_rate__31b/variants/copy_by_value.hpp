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

        std::vector<float> current_window;
        if (full_) {
            current_window.reserve(size_);
            for (size_t i = 0; i < size_; ++i) {
                current_window.push_back(buffer_[(write_pos_ + i) % size_]);
            }
        } else {
            current_window.reserve(write_pos_);
            for (size_t i = 0; i < write_pos_; ++i) {
                current_window.push_back(buffer_[i]);
            }
        }

        size_t count = 0;
        for (size_t i = 0; i < current_window.size() - 1; ++i) {
            if ((current_window[i] > 0 && current_window[i + 1] < 0) || 
                (current_window[i] < 0 && current_window[i + 1] > 0)) {
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
