#pragma once

#include <vector>
#include <cmath>
#include <algorithm>
#include <memory>

/**
 * Internal state and calculation logic for a single axis.
 */
struct FilterState {
    double dt;
    double alpha;
    double state = 0.0;

    FilterState(double dt, double alpha) : dt(dt), alpha(alpha) {}

    double update(double gyro, double accel) {
        double angle_gyro = state + (gyro * dt);
        state = (alpha * angle_gyro) + ((1.0 - alpha) * accel);
        return state;
    }
};

class ComplementaryFilterBank {
public:
    void configure(size_t num_axes, double sample_rate, double alpha) {
        m_num_axes = num_axes;
        m_dt = 1.0 / sample_rate;
        m_alpha = alpha;
        
        m_filters.clear();
        for (size_t i = 0; i < num_axes; ++i) {
            m_filters.push_back(nullptr);
        }
    }

    void process(const double* gyro_rates, const double* accels, double* output) {
        for (size_t i = 0; i < m_num_axes; ++i) {
            output[i] = apply_filter(i, gyro_rates[i], accels[i]);
        }
    }

private:
    double apply_filter(size_t axis_index, double gyro, double accel) {
        if (!m_filters[axis_index]) {
            m_filters[axis_index] = std::make_shared<FilterState>(m_dt, m_alpha);
        }
        return m_filters[axis_index]->update(gyro, accel);
    }

    size_t m_num_axes = 0;
    double m_dt = 0.0;
    double m_alpha = 0.0;
    std::vector<std::shared_ptr<FilterState>> m_filters;
};
