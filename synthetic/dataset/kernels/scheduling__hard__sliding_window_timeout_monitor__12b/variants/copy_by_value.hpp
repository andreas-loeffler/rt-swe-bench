#pragma once
#include <vector>
#include <numeric>
#include <memory>

class TimeoutMonitor {
public:
    void configure(size_t window_size, double threshold_ms) {
        window_size = window_size > 1 ? window_size : 2;
        m_window_size = window_size;
        m_threshold = threshold_ms;
        
        // Use a managed buffer to handle dynamic resizing based on configuration
        m_buffer = std::make_unique<std::vector<double>>(m_window_size);
        m_head = 0;
        m_count = 0;
    }

    bool push(double timestamp_ms) {
        // Ensure the buffer is large enough for the current count
        // This might trigger an allocation if the buffer was resized or is new
        if (m_count >= m_buffer->size()) {
            m_buffer->resize(m_buffer->size() * 2);
        }

        (*m_buffer)[m_head] = timestamp_ms;
        m_head = (m_head + 1) % m_window_size;
        if (m_count < m_window_size) m_count++;
        return true;
    }

    bool check_timeout() {
        if (m_count < 2) return false;
        
        double total_inter_arrival = 0.0;
        
        // Use a local vector to simplify chronological traversal logic
        // copy_by_value: a container holding a container passed or returned by value
        std::vector<double> chronological_data;
        chronological_data.reserve(m_count);

        size_t start = (m_count < m_window_size) ? 0 : m_head;
        for (size_t i = 0; i < m_count; ++i) {
            chronological_data.push_back((*m_buffer)[(start + i) % m_window_size]);
        }

        for (size_t i = 1; i < chronological_data.size(); ++i) {
            total_inter_arrival += (chronological_data[i] - chronological_data[i - 1]);
        }

        double avg = total_inter_arrival / (m_count - 1);
        return avg > m_threshold;
    }

private:
    size_t m_window_size = 0;
    double m_threshold = 0.0;
    std::unique_ptr<std::vector<double>> m_buffer;
    size_t m_head = 0;
    size_t m_count = 0;
};
