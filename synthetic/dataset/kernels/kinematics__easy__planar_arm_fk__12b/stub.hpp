#pragma once
#include <vector>
#include <utility>

class PlanarArmKinematics {
public:
    void configure(size_t num_joints) {
        // TODO
    }

    std::pair<double, double> process(const std::vector<double>& joint_angles) const {
        // TODO
        return {0.0, 0.0};
    }
};
