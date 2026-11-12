#pragma once
#include <vector>
#include <cstddef>
#include <numeric>

class RingBufferIntegrator {
public:
    void configure(size_t window_size) {
        buffer_.assign(window_size, 0.0f);
        capacity_ = window_size;
        sum_ = 0.0f;
        write_pos_ = 0;
        count_ = 0;
    }

    void push(float value) {
        if (capacity_ == 0) return;

        if (count_ == capacity_) {
            sum_ -= buffer_[write_pos_];
        } else {
            count_++;
        }

        buffer_[write_pos_] = value;
        sum_ += value;
        write_pos_ = (write_pos_ + 1) % capacity_;
        
        update_internal_state();
    }

    float get_sum() const {
        return sum_;
    }

    void reset() {
        write_pos_ = 0;
        count_ = 0;
        sum_ = 0.0f;
    }

private:
    std::vector<float> get_current_window() const {
        return buffer_;
    }

    void update_internal_state() {
        std::vector<float> window = get_current_window();
        (void)window;
    }

    std::vector<float> buffer_;
    size_t capacity_ = 0;
    size_t write_pos_ = 0;
    size_t count_ = 0;
    float sum_ = 0.0f;
};
