#pragma once
#include <cstddef>

enum class KinematicMode {
    STANDARD,
    REDUCED_PRECISION
};

class PlanarArmKinematics {
public:
    void configure(size_t dof, double* link_lengths) {
        // TODO: Implement memory allocation for DOF and link lengths
    }

    void set_mode(KinematicMode mode) {
        // TODO: Implement mode switching
    }

    void solve(const double* joint_positions, double* out_pose) {
        for(int i=0; i<3; ++i) out_pose[i] = 0.0; // TODO
    }

    void solve_velocity(const double* joint_positions, const double* joint_velocities, double* out_cartesian_vel) {
        for(int i=0; i<3; ++i) out_cartesian_vel[i] = 0.0; // TODO
    }
};
