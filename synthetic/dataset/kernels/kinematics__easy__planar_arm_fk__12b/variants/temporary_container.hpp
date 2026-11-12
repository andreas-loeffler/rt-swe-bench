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
        std::vector<double> cumulative_angles;
        cumulative_angles.reserve(joint_angles.size());
        
        double current_angle = 0.0;
        for (double angle : joint_angles) {
            current_angle += angle;
            cumulative_angles.push_back(current_angle);
        }

        double x = 0.0;
        double y = 0.0;
        for (double angle : cumulative_angles) {
            x += std::cos(angle);
            y += std::sin(angle);
        }

        return {x, y};
    }

private:
    size_t num_joints_ = 0;
    std::vector<double> lengths_;
};
