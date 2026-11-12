#pragma once
#include <vector>
#include <string>
#include <cstring>
#include <algorithm>

class CircularLogBuffer {
    struct LogEntry {
        double timestamp;
        std::string message;
    };

    size_t m_max_entries = 0;
    size_t m_max_string_len = 0;
    size_t m_head = 0;
    size_t m_count = 0;
    std::vector<LogEntry> m_buffer;

public:
    void configure(size_t max_entries, size_t max_string_len) {
        m_max_entries = max_entries;
        m_max_string_len = max_string_len;
        m_buffer.resize(max_entries);
        m_head = 0;
        m_count = 0;
    }

    void push(double timestamp, const char* message) {
        std::string msg_content = message;
        if (msg_content.length() >= m_max_string_len) {
            msg_content.erase(m_max_string_len - 1);
        }

        LogEntry entry = {timestamp, msg_content};
        m_buffer[m_head] = entry;

        m_head = (m_head + 1) % m_max_entries;
        if (m_count < m_max_entries) {
            m_count++;
        }
    }

    bool read(size_t index, double& timestamp, char* out_message, size_t out_len) {
        if (index >= m_count) return false;

        size_t start_pos = (m_count < m_max_entries) ? 0 : m_head;
        size_t actual_idx = (start_pos + index) % m_max_entries;
        
        const LogEntry entry = m_buffer[actual_idx];
        timestamp = entry.timestamp;
        
        size_t len = std::min(entry.message.length(), out_len - 1);
        std::memcpy(out_message, entry.message.c_str(), len);
        out_message[len] = '\0';
        
        return true;
    }

    size_t size() const {
        return m_count;
    }
};
