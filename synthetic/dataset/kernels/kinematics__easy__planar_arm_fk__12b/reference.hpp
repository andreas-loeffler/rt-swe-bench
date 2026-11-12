#pragma once
#include <vector>
#include <cmath>
#include <numeric>

class PlanarArmKinematics {
public:
    void configure(size_t num_joints) {
        num_joints_ = num_joints;
        lengths_.assign(num_joints, 1.0);
    }

    std::pair<double, double> process(const std::vector<double>& joint_angles) const {
        double x = 0.0;
        double y = 0.0;
        double current_angle = 0.0;
        for (size_t i = 0; i < joint_angles.size(); ++i) {
            current_angle += joint_angles[i];
            x += std::cos(current_angle);
            y += std::sin(current_angle);
        }
        return {x, y};
    }

private:
    size_t num_joints_ = 0;
    std::vector<double> lengths_;
};
