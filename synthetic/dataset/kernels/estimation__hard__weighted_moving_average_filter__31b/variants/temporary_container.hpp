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
            this->size_ = size;
            this->mode_ = mode;
            this->sigma_ = sigma;
        }

        std::vector<float> generate_active_weights(size_t count) const {
            std::vector<float> weights;
            weights.reserve(count);
            for (size_t i = 0; i < count; ++i) {
                if (mode_ == WeightMode::Linear) {
                    weights.push_back(static_cast<float>(size_ - i));
                } else {
                    float x = static_cast<float>(i);
                    weights.push_back(std::exp(-(x * x) / (2.0f * sigma_ * sigma_)));
                }
            }
            return weights;
        }

    private:
        size_t size_ = 0;
        WeightMode mode_ = WeightMode::Linear;
        float sigma_ = 1.0f;
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

        std::vector<float> current_weights = weights_.generate_active_weights(n);
        
        float weighted_sum = 0.0f;
        float weight_total = 0.0f;

        for (size_t i = 0; i < n; ++i) {
            float w = current_weights[i];
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
