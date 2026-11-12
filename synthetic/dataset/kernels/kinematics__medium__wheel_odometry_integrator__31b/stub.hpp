#pragma once

#include <cstddef>
#include <cstdint>

struct Pose {
    double x = 0.0;
    double y = 0.0;
    double theta = 0.0;
};

class WheelOdometryIntegrator {
public:
    void configure(double wheel_base, double ticks_per_meter, size_t window_size) {
        // TODO: implement
    }

    void push_ticks(int32_t left_ticks, int32_t right_ticks, double dt) {
        // TODO: implement
    }

    Pose get_pose() const {
        // TODO: implement
        return Pose{};
    }
};
