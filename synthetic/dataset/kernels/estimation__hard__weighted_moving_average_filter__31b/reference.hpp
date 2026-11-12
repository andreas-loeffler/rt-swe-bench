#pragma once

#include <vector>
#include <cmath>
#include <numeric>
#include <algorithm>

enum class WeightMode {
    Linear,
    Gaussian
};

namespace detail {
    template <typename T>
    class CircularBuffer {
    public:
        void reserve(size_t size) {
            data_.assign(size, T{});
            capacity_ = size;
        }
        void push(T val) {
            data_[head_] = val;
            head_ = (head_ + 1) % capacity_;
            if (count_ < capacity_) ++count_;
        }
        T operator[](size_t i) const {
            // i=0 is the most recent sample
            int idx = static_cast<int>(head_) - 1 - static_cast<int>(i);
            while (idx < 0) idx += capacity_;
            return data_[idx % capacity_];
        }
        void clear() {
            head_ = 0;
            count_ = 0;
        }
        size_t size() const { return count_; }
        size_t capacity() const { return capacity_; }

    private:
        std::vector<T> data_;
        size_t head_ = 0;
        size_t count_ = 0;
        size_t capacity_ = 0;
    };

    class WeightGenerator {
    public:
        void setup(size_t size, WeightMode mode, float sigma) {
            weights_.clear();
            weights_.reserve(size);
            for (size_t i = 0; i < size; ++i) {
                if (mode == WeightMode::Linear) {
                    weights_.push_back(static_cast<float>(size - i));
                } else {
                    float x = static_cast<float>(i);
                    weights_.push_back(std::exp(-(x * x) / (2.0f * sigma * sigma)));
                }
            }
        }
        float get_weight(size_t i) const { return weights_[i]; }
        float sum_of_weights(size_t count) const {
            float sum = 0.0f;
            for (size_t i = 0; i < count; ++i) sum += weights_[i];
            return sum;
        }
    private:
        std::vector<float> weights_;
    };
}

class WeightedMovingAverage {
public:
    void configure(size_t window_size, WeightMode mode, float sigma = 1.0f) {
        buffer_.reserve(window_size);
        weights_.setup(window_size, mode, sigma);
    }

    void push(float sample) {
        buffer_.push(sample);
    }

    float process() {
        size_t n = buffer_.size();
        if (n == 0) return 0.0f;

        float weighted_sum = 0.0f;
        float weight_total = 0.0f;

        for (size_t i = 0; i < n; ++i) {
            float w = weights_.get_weight(i);
            weighted_sum += buffer_[i] * w;
            weight_total += w;
        }

        return weighted_sum / weight_total;
    }

    void reset() {
        buffer_.clear();
    }

private:
    detail::CircularBuffer<float> buffer_;
    detail::WeightGenerator weights_;
};
