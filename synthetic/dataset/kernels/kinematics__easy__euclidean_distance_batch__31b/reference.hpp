#pragma once
#include <vector>
#include <cmath>
#include <algorithm>

class DistanceCalculator {
public:
    void configure(size_t num_targets) {
        m_num_targets = num_targets;
        m_targets.assign(num_targets * 3, 0.0f);
    }

    void set_targets(const float* coords) {
        if (coords) {
            std::copy(coords, coords + (m_num_targets * 3), m_targets.begin());
        }
    }

    void process(const float ref_point[3], float* distances) {
        for (size_t i = 0; i < m_num_targets; ++i) {
            float dx = m_targets[i * 3 + 0] - ref_point[0];
            float dy = m_targets[i * 3 + 1] - ref_point[1];
            float dz = m_targets[i * 3 + 2] - ref_point[2];
            distances[i] = std::sqrt(dx * dx + dy * dy + dz * dz);
        }
    }

private:
    size_t m_num_targets = 0;
    std::vector<float> m_targets;
};
