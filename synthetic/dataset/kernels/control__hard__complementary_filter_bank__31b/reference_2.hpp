#pragma once

#include <cstddef>
#include <vector>
#include <cmath>
#include <algorithm>

/**
 * Internal helper to manage per-axis state.
 * Designed for cache locality and O(1) access in the hot path.
 */
class AxisState {
public:
    double current_angle = 0.0;
    double last_gyro_integrated = 0.0;

    void update(double gyro_rate, double accel_val, double dt, double alpha) {
        // 1. Integrate gyroscope rate
        current_angle += gyro_rate * dt;

        // 2. Simplified linear mapping for the benchmark bank as per requirements
        // In a real system, this would be atan2(accel_y, accel_z), 
        // but the prompt specifies a simplified linear mapping for the benchmark.
        double accel_angle = accel_val; 

        // 3. Fuse using the complementary formula
        current_angle = (alpha * current_angle) + (1.0 - alpha) * accel_angle;
    }
};

class ComplementaryFilterBank {
public:
    void configure(size_t num_axes, double sample_rate, double alpha) {
        m_num_axes = num_axes;
        m_sample_rate = sample_rate;
        m_alpha = alpha;
        
        // Pre-allocate memory to satisfy real-time constraints
        m_states.assign(num_axes, AxisState{});
    }

    /**
     * Real-time method: No allocations, no locks, no exceptions.
     */
    void process(const double* gyro_rates, const double* accels, double* output) {
        // Sanity check for nullptrs (real-time safe as it's a simple branch)
        if (!gyro_rates || !accels || !output || m_states.empty()) {
            return;
        }

        const double dt = 1.0 / m_sample_rate;

        for (size_t i = 0; i < m_num_axes; ++i) {
            m_states[i].update(gyro_rates[i], accels[i], dt, m_alpha);
            output[i] = m_states[i].current_angle;
        }
    }

private:
    size_t m_num_axes = 0;
    double m_sample_rate = 1.0;
    double m_alpha = 0.98;
    std::vector<AxisState> m_states;
};
