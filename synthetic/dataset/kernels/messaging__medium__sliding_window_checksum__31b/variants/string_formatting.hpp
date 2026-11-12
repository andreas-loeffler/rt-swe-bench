#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include <sstream>

class SlidingWindowChecksum {
public:
    void configure(size_t window_size) {
        m_window_size = window_size;
        m_buffer.assign(window_size, 0);
        m_head = 0;
        m_count = 0;
        m_sum = 0;
    }

    void push(uint8_t byte) {
        log_push_event(byte);

        if (m_count == m_window_size) {
            m_sum -= m_buffer[m_head];
        } else {
            m_count++;
        }

        m_buffer[m_head] = byte;
        m_sum += byte;
        m_head = (m_head + 1) % m_window_size;
    }

    uint64_t current_checksum() const {
        return m_sum;
    }

    size_t current_window_fill() const {
        return m_count;
    }

private:
    void log_push_event(uint8_t byte) {
        std::ostringstream oss;
        oss << "Pushing byte: " << std::to_string(byte) << " at index " << m_head;
        std::string msg = oss.str();
    }

    size_t m_window_size = 0;
    size_t m_head = 0;
    size_t m_count = 0;
    uint64_t m_sum = 0;
    std::vector<uint8_t> m_buffer;
};
