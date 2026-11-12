#pragma once
#include <vector>
#include <algorithm>

class MedianFilter {
public:
    void configure(size_t window_size) {
        buffer_.resize(window_size);
        max_size_ = window_size;
        head_ = 0;
        count_ = 0;
    }

    void push(float sample) {
        if (max_size_ == 0) return;

        buffer_[head_] = sample;
        head_ = (head_ + 1) % max_size_;
        if (count_ < max_size_) {
            count_++;
        }
    }

    float process() {
        if (count_ == 0) return 0.0f;

        std::vector<float> window = get_current_window();
        std::sort(window.begin(), window.end());

        if (window.size() % 2 == 1) {
            return window[window.size() / 2];
        } else {
            return (window[window.size() / 2 - 1] + window[window.size() / 2]) * 0.5f;
        }
    }

private:
    std::vector<float> get_current_window() {
        std::vector<float> window;
        window.reserve(count_);
        for (size_t i = 0; i < count_; ++i) {
            window.push_back(buffer_[i]);
        }
        return window;
    }

    std::vector<float> buffer_;
    size_t max_size_ = 0;
    size_t head_ = 0;
    size_t count_ = 0;
};
