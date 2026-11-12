#pragma once
#include <vector>
#include <numeric>
#include <deque>

class LinearRegressionEstimator {
public:
    void configure(size_t window_size) {
        m_window_size = window_size;
        buffer_.clear();
    }

    double push(double sample) {
        buffer_.push_back(sample);
        if (buffer_.size() > m_window_size) {
            buffer_.pop_front();
        }

        size_t count = buffer_.size();
        if (count < 2) {
            return 0.0;
        }

        double sum_x = 0.0;
        double sum_y = 0.0;
        double sum_xy = 0.0;
        double sum_x2 = 0.0;

        for (size_t i = 0; i < count; ++i) {
            double x = static_cast<double>(i);
            double y = buffer_[i];

            sum_x += x;
            sum_y += y;
            sum_xy += x * y;
            sum_x2 += x * x;
        }

        double denominator = (static_cast<double>(count) * sum_x2) - (sum_x * sum_x);
        if (denominator == 0.0) return 0.0;
        
        return ((static_cast<double>(count) * sum_xy) - (sum_x * sum_y)) / denominator;
    }

private:
    std::deque<double> buffer_;
    size_t m_window_size = 0;
};
