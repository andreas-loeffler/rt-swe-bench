#pragma once
#include <algorithm>
#include <vector>

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
        if (t <= 0.0) {
            return {m_p0, 0.0, 0.0};
        }
        if (t >= m_duration) {
            return {m_p1, 0.0, 0.0};
        }

        return calculate_state(t);
    }

private:
    State calculate_state(double t) {
        std::vector<double> powers;
        double x = t / m_duration;
        
        double current_power = x;
        for (int i = 1; i <= 5; ++i) {
            powers.push_back(current_power);
            current_power *= x;
        }

        const double delta_p = m_p1 - m_p0;
        const double inv_t = 1.0 / m_duration;
        const double inv_t2 = inv_t * inv_t;

        double pos = m_p0 + delta_p * (10.0 * powers[2] - 15.0 * powers[3] + 6.0 * powers[4]);
        double vel = delta_p * (30.0 * powers[1] - 60.0 * powers[2] + 30.0 * powers[3]) * inv_t;
        double acc = delta_p * (60.0 * powers[0] - 180.0 * powers[1] + 120.0 * powers[2]) * inv_t2;

        return {pos, vel, acc};
    }

    double m_duration = 1.0;
    double m_p0 = 0.0;
    double m_p1 = 0.0;
};
