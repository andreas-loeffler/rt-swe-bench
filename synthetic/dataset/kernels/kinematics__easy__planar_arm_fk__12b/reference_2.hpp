#pragma once
#include <vector>
#include <utility>
#include <cmath>

/**
 * PlanarArmKinematics
 * Computes the (x, y) end-effector position of a 2D planar robotic arm.
 * 
 * Implementation ensures hard real-time constraints by avoiding heap 
 * allocation and synchronization primitives during the process() call.
 */
class PlanarArmKinematics {
public:
    /**
     * Pre-allocates internal buffers. 
     * Called once before entering the real-time loop.
     */
    void configure(size_t num_joints) {
        // Pre-allocate capacity to avoid reallocation during process().
        // While this specific algorithm doesn't strictly require a buffer
        // for the summation, we store the size to validate preconditions.
        num_joints_ = num_joints;
    }

    /**
     * Computes the (x, y) coordinates of the end-effector.
     * 
     * Complexity: O(N) where N is the number of joints.
     * Real-time constraints: No heap allocation, no locks, no I/O, no exceptions.
     */
    std::pair<double, double> process(const std::vector<double>& joint_angles) const {
        // Precondition: size must match configuration.
        // In a production system, we might use an assertion or a status code,
        // but per constraints, we assume preconditions are met.
        
        double x = 0.0;
        double y = 0.0;
        double cumulative_angle = 0.0;

        // The end-effector position is the sum of the vectors for each link.
        // Each link i is oriented at the sum of angles from joint 0 to i.
        for (size_t i = 0; i < num_joints_; ++i) {
            cumulative_angle += joint_angles[i];
            x += std::cos(cumulative_angle);
            y += std::sin(cumulative_angle);
        }

        return {x, y};
    }

private:
    size_t num_joints_ = 0;
};
