#pragma once
#include <vector>
#include <numeric>

class TimeoutMonitor {
public:
    void configure(size_t window_size, double threshold_ms) {
        window_size = window_size > 1 ? window_size : 2;
        m_window_size = window_size;
        m_threshold = threshold_ms;
        m_buffer.resize(m_window_size);
        m_head = 0;
        m_count = 0;
    }

    bool push(double timestamp_ms) {
        m_buffer[m_head] = timestamp_ms;
        m_head = (m_head + 1) % m_window_size;
        if (m_count < m_window_size) m_count++;
        return true;
    }

    bool check_timeout() {
        if (m_count < 2) return false;
        
        double sum_diff = 0.0;
        // The circular buffer stores timestamps. 
        // To find inter-arrival times, we need to traverse chronologically.
        // Since we assume monotonic input, the oldest is at current head (if count == window_size)
        // or at the index after the one we just wrote.
        
        // However, a more robust way to get inter-arrival is to sum the differences
        // between adjacent timestamps in chronological order.
        double total_inter_arrival = 0.0;
        
        // Indices in chronological order:
        // If count < window_size, they are 0, 1, ..., count-1
        // If count == window_size, the oldest is at m_head, then (m_head+1)%size...
        
        size_t start = (m_count < m_window_size) ? 0 : m_head;
        for (size_t i = 1; i < m_count; ++i) {
            size_t current = (start + i) % m_window_size;
            size_t prev = (start + i - 1) % m_window_size;
            total_inter_arrival += (m_buffer[current] - m_buffer[prev]);
        }

        double avg = total_inter_arrival / (m_count - 1);
        return avg > m_threshold;
    }

private:
    size_t m_window_size = 0;
    double m_threshold = 0.0;
    std::vector<double> m_buffer;
    size_t m_head = 0;
    size_t m_count = 0;
};
