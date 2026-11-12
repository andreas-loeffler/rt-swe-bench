#pragma once

#include <vector>
#include <cstddef>
#include <cmath>
#include <algorithm>

class PlanarForwardKinematics {
public:
    struct Point {
        double x;
        double y;
    };

    void configure(size_t max_trajectory_length, double link1_len, double link2_len) {
        max_len_ = max_trajectory_length;
        l1_ = link1_len;
        l2_ = link2_len;
        results_.resize(max_trajectory_length);
    }

    size_t process(const double* joint_angles, size_t count) {
        if (!joint_angles) return 0;
        
        size_t to_process = std::min(count, max_len_);
        
        for (size_t i = 0; i < to_process; ++i) {
            double q1 = joint_angles[i * 2];
            double q2 = joint_angles[i * 2 + 1];
            
            double cos1 = std::cos(q1);
            double sin1 = std::sin(q1);
            
            // angle2 is relative to link 1
            double cos12 = std::cos(q1 + q2);
            double sin12 = std::sin(q1 + q2);
            
            results_[i].x = l1_ * cos1 + l2_ * cos12;
            results_[i].y = l1_ * sin1 + l2_ * sin12;
        }
        
        return to_process;
    }

    const Point& get_result(size_t index) const {
        return results_[index];
    }

private:
    size_t max_len_ = 0;
    double l1_ = 0.0;
    double l2_ = 0.0;
    std::vector<Point> results_;
};
