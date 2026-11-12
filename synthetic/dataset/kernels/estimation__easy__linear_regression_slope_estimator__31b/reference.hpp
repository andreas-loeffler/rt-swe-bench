#pragma once
#include <vector>

class LinearRegressionEstimator {
public:
    void configure(size_t window_size) {
        buffer_.assign(window_size, 0.0);
        m_window_size = window_size;
        m_count = 0;
        m_head = 0;
    }

    double push(double sample) {
        buffer_[m_head] = sample;
        m_head = (m_head + 1) % m_window_size;
        if (m_count < m_window_size) {
            m_count++;
        }

        if (m_count < 2) {
            return 0.0;
        }

        double sum_x = 0.0;
        double sum_y = 0.0;
        double sum_xy = 0.0;
        double sum_x2 = 0.0;

        for (size_t i = 0; i < m_count; ++i) {
            // If the window is full, the oldest sample is at m_head.
            // If the window is filling, the oldest sample is at index 0.
            size_t idx = (m_count < m_window_size) ? i : (m_head + i) % m_window_size;
            double x = static_cast<double>(i);
            double y = buffer_[idx];

            sum_x += x;
            sum_y += y;
            sum_xy += x * y;
            sum_x2 += x * x;
        }

        double n = static_cast<double>(m_count);
        double denominator = (n * sum_x2) - (sum_x * sum_x);
        if (denominator == 0.0) return 0.0;
        
        return ((n * sum_xy) - (sum_x * sum_y)) / denominator;
    }

private:
    std::vector<double> buffer_;
    size_t m_window_size = 0;
    size_t m_count = 0;
    size_t m_head = 0;
};
