#pragma once
#include <vector>
#include <cstdint>
#include <algorithm>
#include <map>

class LinearHistogramCounter {
public:
    void configure(int32_t min_value, int32_t max_value) {
        m_min = min_value;
        m_max = max_value;
        m_counts.clear();
    }

    void increment(int32_t value) {
        if (value >= m_min && value <= m_max) {
            m_counts[value]++;
        }
    }

    uint64_t get_count(int32_t value) const {
        if (value >= m_min && value <= m_max) {
            auto it = m_counts.find(value);
            if (it != m_counts.end()) {
                return it->second;
            }
        }
        return 0;
    }

    void reset() {
        m_counts.clear();
    }

private:
    int32_t m_min{0};
    int32_t m_max{0};
    std::map<int32_t, uint64_t> m_counts;
};
