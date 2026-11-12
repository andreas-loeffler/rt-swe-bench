#pragma once
#include <vector>
#include <cmath>
#include <algorithm>

enum class KinematicMode {
    STANDARD,
    REDUCED_PRECISION
};

class TransformChain {
public:
    void compute_pose(size_t dof, const double* qs, const double* lengths, double* pose, KinematicMode mode) {
        double x = 0, y = 0, phi = 0;
        for (size_t i = 0; i < dof; ++i) {
            phi += qs[i];
            double s = std::sin(phi);
            double c = std::cos(phi);
            x += lengths[i] * c;
            y += lengths[i] * s;
        }
        pose[0] = x;
        pose[1] = y;
        pose[2] = phi;
    }
};

class JacobianSolver {
    struct JacobianColumn {
        double vx;
        double vy;
        double omega;
    };

    std::vector<JacobianColumn> calculate_columns(size_t dof, const double* qs, const double* lengths) {
        std::vector<JacobianColumn> cols(dof);
        for (size_t j = 0; j < dof; ++j) {
            double col_vx = 0, col_vy = 0;
            double temp_phi = 0;
            for (size_t i = 0; i < dof; ++i) {
                temp_phi += qs[i];
                if (i >= j) {
                    col_vx -= lengths[i] * std::sin(temp_phi);
                    col_vy += lengths[i] * std::cos(temp_phi);
                }
            }
            cols[j] = {col_vx, col_vy, 1.0};
        }
        return cols;
    }

public:
    void compute_velocity(size_t dof, const double* qs, const double* qdots, const double* lengths, double* v_out) {
        double vx = 0, vy = 0, omega = 0;
        
        auto columns = calculate_columns(dof, qs, lengths);
        
        for (size_t j = 0; j < dof; ++j) {
            vx += columns[j].vx * qdots[j];
            vy += columns[j].vy * qdots[j];
            omega += columns[j].omega * qdots[j];
        }
        v_out[0] = vx;
        v_out[1] = vy;
        v_out[2] = omega;
    }
};

class PlanarArmKinematics {
public:
    void configure(size_t dof, double* link_lengths) {
        this->dof_ = dof;
        lengths_.assign(link_lengths, link_lengths + dof);
        mode_ = KinematicMode::STANDARD;
    }

    void set_mode(KinematicMode mode) {
        mode_ = mode;
    }

    void solve(const double* joint_positions, double* out_pose) {
        chain_.compute_pose(dof_, joint_positions, lengths_.data(), out_pose, mode_);
    }

    void solve_velocity(const double* joint_positions, const double* joint_velocities, double* out_cartesian_vel) {
        jacobian_.compute_velocity(dof_, joint_positions, joint_velocities, lengths_.data(), out_cartesian_vel);
    }

private:
    size_t dof_ = 0;
    std::vector<double> lengths_;
    KinematicMode mode_ = KinematicMode::STANDARD;
    TransformChain chain_;
    JacobianSolver jacobian_;
};
