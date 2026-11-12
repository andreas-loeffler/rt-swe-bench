#pragma once

#include <vector>
#include <cmath>
#include <algorithm>

/**
 * Helper class to handle the 1D complementary filter logic.
 */
class AxisFilter {
public:
    void init(double dt, double alpha) {
        m_dt = dt;
        m_alpha = alpha;
        m_state = 0.0;
    }

    double step(double gyro, double accel) {
        // High-pass: Integrate gyro
        double angle_gyro = m_state + (gyro * m_dt);
        // Low-pass: Trust accel
        // The "Complementary" fusion
        m_state = (m_alpha * angle_gyro) + ((1.0 - m_alpha) * accel);
        return m_state;
    }

    void reset() { m_state = 0.0; }

private:
    double m_dt = 0.0;
    double m_alpha = 0.0;
    double m_state = 0.0;
};

class ComplementaryFilterBank {
public:
    void configure(size_t num_axes, double sample_rate, double alpha) {
        m_num_axes = num_axes;
        m_dt = 1.0 / sample_rate;
        m_alpha = alpha;
        
        m_filters.clear();
        m_filters.resize(num_axes);
        for (auto& f : m_filters) {
            f.init(m_dt, m_alpha);
        }
    }

    void process(const double* gyro_rates, const double* accels, double* output) {
        for (size_t i = 0; i < m_num_axes; ++i) {
            output[i] = m_filters[i].step(gyro_rates[i], accels[i]);
        }
    }

private:
    size_t m_num_axes = 0;
    double m_dt = 0.0;
    double m_alpha = 0.0;
    std::vector<AxisFilter> m_filters;
};
