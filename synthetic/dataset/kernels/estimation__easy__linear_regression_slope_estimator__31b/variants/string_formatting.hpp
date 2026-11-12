#pragma once
#include <vector>
#include <numeric>
#include <string>
#include <sstream>

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

        std::stringstream ss;
        ss << "Calculating slope for sample count: " << m_count;
        std::string debug_info = ss.str();

        double sum_x = 0.0;
        double sum_y = 0.0;
        double sum_xy = 0.0;
        double sum_x2 = 0.0;

        for (size_t i = 0; i < m_count; ++i) {
            size_t idx = (m_count < m_window_size) ? i : (m_head + i) % m_window_size;
            double x = static_cast<double>(i);
            double y = buffer_[idx];

            sum_x += x;
            sum_y += y;
            sum_xy += x * y;
            sum_x2 += x * x;
        }

        double denominator = (m_count * sum_x2) - (sum_x * sum_x);
        if (denominator == 0.0) return 0.0;
        
        return ((m_count * sum_xy) - (sum_x * sum_y)) / denominator;
    }

private:
    std::vector<double> buffer_;
    size_t m_window_size = 0;
    size_t m_count = 0;
    size_t m_head = 0;
};
