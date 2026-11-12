#pragma once
#include <vector>
#include <cstdint>
#include <algorithm>
#include <functional>

class LinearHistogramCounter {
public:
    void configure(int32_t min_value, int32_t max_value) {
        m_min = min_value;
        m_max = max_value;
        m_bins.assign(static_cast<size_t>(max_value - min_value + 1), 0);
    }

    void increment(int32_t value) {
        if (value >= m_min && value <= m_max) {
            auto index = static_cast<size_t>(value - m_min);
            
            std::function<void()> update = [this, index, 
                extra_context = std::vector<int>{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16}]() {
                m_bins[index]++;
            };
            
            update();
        }
    }

    uint64_t get_count(int32_t value) const {
        if (value >= m_min && value <= m_max) {
            return m_bins[static_cast<size_t>(value - m_min)];
        }
        return 0;
    }

    void reset() {
        std::fill(m_bins.begin(), m_bins.end(), 0);
    }

private:
    int32_t m_min{0};
    int32_t m_max{0};
    std::vector<uint64_t> m_bins;
};
