#include "kernel.hpp"
#include "rtk/check.hpp"
#include <vector>

struct TestCase {
    double setpoint;
    double outer_meas;
    double inner_meas;
    bool mode;
};

int main() {
    rtk::setup();
    rtk::Run run;

    CascadedPIDController k;
    
    const double dt = 0.01;
    const double oKp = 1.0, oKi = 0.1, oKd = 0.01;
    const double iKp = 2.0, iKi = 0.2, iKd = 0.02;
    const double min_out = -10.0;
    const double max_out = 10.0;

    k.configure(dt, oKp, oKi, oKd, iKp, iKi, iKd, min_out, max_out);

    // Local simulation to generate ground truth
    auto simulate_step = [&](double setpoint, double outer_meas, double inner_meas, 
                             double& o_int, double& o_prev, double& i_int, double& i_prev, bool active) {
        if (!active) {
            o_int = 0; o_prev = 0; i_int = 0; i_prev = 0;
            return 0.0;
        }
        double o_err = setpoint - outer_meas;
        o_int += o_err * dt;
        double o_der = (o_err - o_prev) / dt;
        double inner_sp = (oKp * o_err) + (oKi * o_int) + (oKd * o_der);
        o_prev = o_err;

        double i_err = inner_sp - inner_meas;
        i_int += i_err * dt;
        double i_der = (i_err - i_prev) / dt;
        double out = (iKp * i_err) + (iKi * i_int) + (iKd * i_der);
        i_prev = i_err;

        double clamped = std::clamp(out, min_out, max_out);
        if (out > max_out || out < min_out) {
            i_int = std::clamp(i_int, min_out, max_out);
        }
        return clamped;
    };

    double o_int = 0, o_prev = 0, i_int = 0, i_prev = 0;
    bool current_active = false;

    // 600 iterations to cover steady state, transients, and mode switches
    for (int i = 0; i < 600; ++i) {
        bool next_active = current_active;
        double sp = 1.0, om = 0.0, im = 0.0;

        if (i == 0) next_active = true;
        if (i == 200) next_active = false;
        if (i == 300) next_active = true;
        
        // Introduce signal dynamics
        sp = (i > 100) ? 2.0 : 1.0;
        om = 0.1 * i * 0.1; 
        im = 0.05 * i * 0.1;

        rtk::guarded_void(run, [&] { k.set_mode(next_active); });
        
        double expected = simulate_step(sp, om, im, o_int, o_prev, i_int, i_prev, next_active);
        
        double actual = rtk::guarded(run, [&] { 
            return k.step(sp, om, im); 
        });

        rtk::expect(run, rtk::near(actual, expected, 1e-7), "Step " + std::to_string(i));
        current_active = next_active;
    }

    return rtk::report(run);
}
