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
            // In REDUCED_PRECISION mode, we could use a fast sine/cosine approx, 
            // but for this kernel we demonstrate the state dispatch.
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
public:
    void compute_velocity(size_t dof, const double* qs, const double* qdots, const double* lengths, double* v_out) {
        double vx = 0, vy = 0, omega = 0;
        double current_phi = 0;
        
        // Pre-calculate cumulative angles for Jacobian columns
        for (size_t j = 0; j < dof; ++j) {
            current_phi += qs[j];
            
            // The j-th column of the Jacobian for a planar arm:
            // dx/dq_j = -sum_{i=j}^{dof-1} length[i] * sin(sum_{k=0}^i q_k)
            // dy/dq_j =  sum_{i=j}^{dof-1} length[i] * cos(sum_{k=0}^i q_k)
            // dphi/dq_j = 1
            
            double col_vx = 0, col_vy = 0;
            double temp_phi = 0;
            for (size_t i = 0; i < dof; ++i) {
                temp_phi += qs[i];
                if (i >= j) {
                    col_vx -= lengths[i] * std::sin(temp_phi);
                    col_vy += lengths[i] * std::cos(temp_phi);
                }
            }
            vx += col_vx * qdots[j];
            vy += col_vy * qdots[j];
            omega += qdots[j];
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
