#pragma once
#include <vector>
#include <string_view>
#include <cstring>
#include <algorithm>

class CircularLogBuffer {
    size_t m_max_entries = 0;
    size_t m_max_string_len = 0;
    size_t m_head = 0;
    size_t m_count = 0;
    char* m_buffer = nullptr;

public:
    void configure(size_t max_entries, size_t max_string_len) {
        m_max_entries = max_entries;
        m_max_string_len = max_string_len;
        size_t entry_size = sizeof(double) + max_string_len;
        m_buffer = new char[max_entries * entry_size];
        m_head = 0;
        m_count = 0;
    }

    ~CircularLogBuffer() {
        delete[] m_buffer;
    }

    void push(double timestamp, const char* message) {
        size_t offset = m_head * (sizeof(double) + m_max_string_len);
        
        std::memcpy(m_buffer + offset, &timestamp, sizeof(double));
        
        char* msg_ptr = m_buffer + offset + sizeof(double);
        size_t len = std::min(std::strlen(message), m_max_string_len - 1);
        std::memcpy(msg_ptr, message, len);
        msg_ptr[len] = '\0';

        m_head = (m_head + 1) % m_max_entries;
        if (m_count < m_max_entries) {
            m_count++;
        }
    }

    bool read(size_t index, double& timestamp, char* out_message, size_t out_len) {
        if (index >= m_count) return false;

        size_t start_pos = (m_count < m_max_entries) ? 0 : m_head;
        size_t actual_idx = (start_pos + index) % m_max_entries;
        
        size_t offset = actual_idx * (sizeof(double) + m_max_string_len);
        
        std::memcpy(&timestamp, m_buffer + offset, sizeof(double));
        
        char* msg_ptr = m_buffer + offset + sizeof(double);
        size_t len = std::min(std::strlen(msg_ptr), out_len - 1);
        std::memcpy(out_message, msg_ptr, len);
        out_message[len] = '\0';
        
        return true;
    }

    size_t size() const {
        return m_count;
    }
};
