#pragma once
#include <cstddef>
#include <vector>
#include <algorithm>

class SortedWindow {
public:
    void configure(size_t capacity) {
        // Reserve space to avoid allocations in process()
        // We need two buffers: one for the chronological order (FIFO)
        // and one for the sorted order to allow O(log N) search or O(N) shift.
        m_capacity = capacity;
        m_fifo.reserve(capacity);
        m_sorted.reserve(capacity);
        m_head = 0;
    }

    double process(double value) {
        if (m_fifo.size() < m_capacity) {
            // Window is filling up
            m_fifo.push_back(value);
            auto it = std::lower_bound(m_sorted.begin(), m_sorted.end(), value);
            m_sorted.insert(it, value);
        } else {
            // Window is full: evict oldest, insert newest
            double oldest = m_fifo[m_head];
            m_fifo[m_head] = value;
            m_head = (m_head + 1) % m_capacity;

            // Remove oldest from sorted vector
            auto it_old = std::lower_bound(m_sorted.begin(), m_sorted.end(), oldest);
            // Ensure we remove exactly one instance of the value
            // (though lower_bound is fine as any identical value works for median)
            m_sorted.erase(it_old);

            // Insert newest into sorted vector
            auto it_new = std::lower_bound(m_sorted.begin(), m_sorted.end(), value);
            m_sorted.insert(it_new, value);
        }

        // Calculate median
        size_t n = m_sorted.size();
        if (n % 2 == 1) {
            return m_sorted[n / 2];
        } else {
            return (m_sorted[n / 2 - 1] + m_sorted[n / 2]) * 0.5;
        }
    }

private:
    size_t m_capacity = 0;
    size_t m_head = 0;
    std::vector<double> m_fifo;   // Maintains chronological order
    std::vector<double> m_sorted; // Maintains sorted order
};
