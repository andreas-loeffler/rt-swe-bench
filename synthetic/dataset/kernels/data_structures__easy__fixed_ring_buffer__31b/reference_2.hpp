#pragma once

#include <cstddef>
#include <memory>

template <typename T>
class FixedRingBuffer {
public:
    void configure(size_t capacity) {
        m_capacity = capacity;
        m_buffer = std::make_unique<T[]>(m_capacity);
        m_head = 0;
        m_count = 0;
    }

    T push(T value) {
        if (m_buffer == nullptr) return T{};

        if (m_count == m_capacity) {
            // Buffer is full, return the oldest value and overwrite it
            T oldest = m_buffer[m_head];
            m_buffer[m_head] = value;
            m_head = (m_head + 1) % m_capacity;
            return oldest;
        }

        // Buffer not full, just insert
        m_buffer[(m_head + m_count) % m_capacity] = value;
        m_count++;
        return T{};
    }

private:
    std::unique_ptr<T[]> m_buffer;
    size_t m_capacity = 0;
    size_t m_head = 0;
    size_t m_count = 0;
};
