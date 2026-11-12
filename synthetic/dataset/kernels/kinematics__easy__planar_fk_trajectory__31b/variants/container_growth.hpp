#pragma once

#include <vector>
#include <cmath>
#include <algorithm>

class PlanarForwardKinematics {
public:
    struct Point {
        double x;
        double y;
    };

    void configure(size_t max_trajectory_length, double link1_len, double link2_len) {
        m_l1 = link1_len;
        m_l2 = link2_len;
        m_max_len = max_trajectory_length;
        m_results.clear();
    }

    size_t process(const double* joint_angles, size_t count) {
        if (!joint_angles) return 0;
        
        const size_t actual_count = std::min(count, m_max_len);
        
        m_results.clear();
        for (size_t i = 0; i < actual_count; ++i) {
            const double q1 = joint_angles[2 * i];
            const double q2 = joint_angles[2 * i + 1];
            
            double x = m_l1 * std::cos(q1) + m_l2 * std::cos(q1 + q2);
            double y = m_l1 * std::sin(q1) + m_l2 * std::sin(q1 + q2);
            
            m_results.push_back({x, y});
        }
        
        return actual_count;
    }

    const Point& get_result(size_t index) const {
        return m_results[index];
    }

private:
    double m_l1 = 0.0;
    double m_l2 = 0.0;
    size_t m_max_len = 0;
    std::vector<Point> m_results;
};
