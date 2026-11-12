#pragma once
#include <vector>
#include <cmath>

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

        size_t count = 0;
        size_t available = full_ ? size_ : write_pos_;
        if (available < 2) return 0.0f;

        for (size_t i = 0; i < available - 1; ++i) {
            size_t idx1 = full_ ? (write_pos_ + i) % size_ : i;
            size_t idx2 = full_ ? (write_pos_ + i + 1) % size_ : i + 1;
            if ((buffer_[idx1] > 0 && buffer_[idx2] < 0) || 
                (buffer_[idx1] < 0 && buffer_[idx2] > 0)) {
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
