#pragma once

#include <vector>
#include <cstddef>

class PlanarForwardKinematics {
public:
    struct Point {
        double x;
        double y;
    };

    void configure(size_t max_trajectory_length, double link1_len, double link2_len) {
        // TODO: Implement allocation
    }

    size_t process(const double* joint_angles, size_t count) {
        // TODO: Implement FK logic
        return 0;
    }

    const Point& get_result(size_t index) const {
        static Point p{0.0, 0.0};
        return p;
    }
};
