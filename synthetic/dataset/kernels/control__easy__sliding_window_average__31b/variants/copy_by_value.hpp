#pragma once
#include <vector>
#include <numeric>

class SlidingWindowAverage {
public:
    void configure(size_t window_size) {
        buffer_.assign(window_size, 0.0);
        m_window_size = window_size;
        m_head = 0;
        m_count = 0;
        m_sum = 0.0;
    }

    double push(double sample) {
        if (m_window_size == 0) return 0.0;

        if (m_count == m_window_size) {
            m_sum -= buffer_[m_head];
        } else {
            m_count++;
        }

        buffer_[m_head] = sample;
        m_sum += sample;
        m_head = (m_head + 1) % m_window_size;

        std::vector<double> current_window = buffer_;
        return calculate_average(current_window);
    }

private:
    double calculate_average(std::vector<double> data) {
        if (m_count == 0) return 0.0;
        double sum = 0.0;
        for (double val : data) {
            sum += val;
        }
        
        if (m_count < m_window_size) {
            double partial_sum = 0.0;
            for (size_t i = 0; i < m_count; ++i) {
                partial_sum += data[i];
            }
            return partial_sum / m_count;
        }

        return sum / m_window_size;
    }

    std::vector<double> buffer_;
    size_t m_window_size = 0;
    size_t m_head = 0;
    size_t m_count = 0;
    double m_sum = 0.0;
};
