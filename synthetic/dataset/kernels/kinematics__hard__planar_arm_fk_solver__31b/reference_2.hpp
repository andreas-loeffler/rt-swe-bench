#pragma once

#include <cstddef>
#include <cmath>
#include <algorithm>
#include <vector>

enum class KinematicMode {
    STANDARD,
    REDUCED_PRECISION
};

/**
 * @brief PlanarArmKinematics computes Forward Kinematics and 
 * Differential Kinematics for a multi-link planar robot arm.
 */
class PlanarArmKinematics {
private:
    size_t dof_ = 0;
    KinematicMode mode_ = KinematicMode::STANDARD;
    double* lengths_ = nullptr;
    double* workspace_ = nullptr; // Stores cumulative joint angles (theta_0 + ... + theta_i)

public:
    PlanarArmKinematics() = default;

    ~PlanarArmKinematics() {
        delete[] lengths_;
        delete[] workspace_;
    }

    /**
     * @brief Initializes the solver. Memory is pre-allocated for real-time safety.
     * @param dof Number of degrees of freedom.
     * @param link_lengths Array of link lengths of size dof.
     */
    void configure(size_t dof, double* link_lengths) {
        delete[] lengths_;
        delete[] workspace_;

        dof_ = dof;
        lengths_ = new double[dof];
        workspace_ = new double[dof];

        for (size_t i = 0; i < dof; ++i) {
            lengths_[i] = link_lengths[i];
        }
    }

    void set_mode(KinematicMode mode) {
        mode_ = mode;
    }

    /**
     * @brief Computes forward kinematics (FK).
     * @param joint_positions Array of size dof (radians).
     * @param out_pose Output array of size 3: [x, y, phi].
     */
    void solve(const double* joint_positions, double* out_pose) {
        if (dof_ == 0) return;

        double x = 0.0;
        double y = 0.0;
        double current_angle = 0.0;

        for (size_t i = 0; i < dof_; ++i) {
            current_angle += joint_positions[i];
            x += lengths_[i] * std::cos(current_angle);
            y += lengths_[i] * std::sin(current_angle);
        }

        out_pose[0] = x;
        out_pose[1] = y;
        out_pose[2] = current_angle;
    }

    /**
     * @brief Computes differential kinematics (Jacobian).
     * @param joint_positions Array of size dof.
     * @param joint_velocities Array of size dof (rad/s).
     * @param out_cartesian_vel Output array of size 3: [vx, vy, omega].
     */
    void solve_velocity(const double* joint_positions, const double* joint_velocities, double* out_cartesian_vel) {
        if (dof_ == 0) return;

        // Step 1: Calculate cumulative angles for the Jacobian.
        // Let theta_i be the relative angle of joint i.
        // Let T_i = sum_{j=0}^{i} theta_j be the absolute orientation of link i.
        double current_angle = 0.0;
        for (size_t i = 0; i < dof_; ++i) {
            current_angle += joint_positions[i];
            workspace_[i] = current_angle;
        }

        double vx = 0.0;
        double vy = 0.0;
        double omega = 0.0;

        // The Jacobian J maps joint velocities to Cartesian velocities: V = J * dq.
        // For a planar arm:
        // x = sum_{i=0}^{n-1} L_i * cos(T_i)
        // y = sum_{i=0}^{n-1} L_i * sin(T_i)
        // Velocity vx = sum_{i=0}^{n-1} (dL_i/dt) * cos(T_i) - sum_{i=0}^{n-1} L_i * sin(T_i) * (dT_i/dt)
        // Since L_i is constant, dL_i/dt = 0.
        // dT_i/dt = sum_{j=0}^{i} dq_j.
        // Thus, vx = sum_{i=0}^{n-1} [ -L_i * sin(T_i) * (sum_{j=0}^{i} dq_j) ]
        // And vy = sum_{i=0}^{n-1} [ L_i * cos(T_i) * (sum_{j=0}^{i} dq_j) ]
        // omega = sum_{j=0}^{n-1} dq_j
        
        // To compute these sums in O(n) time:
        // Let S_i = sum_{j=0}^{i} dq_j.
        // vx = sum_{i=0}^{n-1} -L_i * sin(T_i) * S_i
        // vy = sum_{i=0}^{n-1} L_i * cos(T_i) * S_i
        
        double current_s = 0.0;
        for (size_t i = 0; i < dof_; ++i) {
            current_s += joint_velocities[i];
            vx += -lengths_[i] * std::sin(workspace_[i]) * current_s;
            vy +=  lengths_[i] * std::cos(workspace_[i]) * current_s;
        }
        omega = current_s;

        out_cartesian_vel[0] = vx;
        out_cartesian_vel[1] = vy;
        out_cartesian_vel[2] = omega;
    }
};
