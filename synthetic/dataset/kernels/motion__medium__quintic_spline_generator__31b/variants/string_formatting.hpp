#pragma once
#include <algorithm>
#include <string>
#include <sstream>

class QuinticSplineGenerator {
public:
    struct State {
        double position;
        double velocity;
        double acceleration;
    };

    void configure(double duration) {
        m_duration = duration;
    }

    void set_targets(double start_pos, double end_pos) {
        m_p0 = start_pos;
        m_p1 = end_pos;
    }

    State step(double t) {
        log_internal_state(t);

        if (t <= 0.0) {
            return {m_p0, 0.0, 0.0};
        }
        if (t >= m_duration) {
            return {m_p1, 0.0, 0.0};
        }

        const double x = t / m_duration;
        const double x2 = x * x;
        const double x3 = x2 * x;
        const double x4 = x3 * x;
        const double x5 = x4 * x;

        const double delta_p = m_p1 - m_p0;
        const double inv_t = 1.0 / m_duration;
        const double inv_t2 = inv_t * inv_t;

        double pos = m_p0 + delta_p * (10.0 * x3 - 15.0 * x4 + 6.0 * x5);
        double vel = delta_p * (30.0 * x2 - 60.0 * x3 + 30.0 * x4) * inv_t;
        double acc = delta_p * (60.0 * x - 180.0 * x2 + 120.0 * x3) * inv_t2;

        return {pos, vel, acc};
    }

private:
    void log_internal_state(double t) {
        std::ostringstream oss;
        oss << "T: " << t << " P0: " << m_p0 << " P1: " << m_p1;
        std::string debug_msg = oss.str();
        (void)debug_msg;
    }

    double m_duration = 1.0;
    double m_p0 = 0.0;
    double m_p1 = 0.0;
};
