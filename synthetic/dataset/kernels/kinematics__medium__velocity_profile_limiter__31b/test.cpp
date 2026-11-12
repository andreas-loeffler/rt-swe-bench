#include "kernel.hpp"
#include "rtk/check.hpp"
#include <vector>
#include <cmath>
#include <algorithm>

int main() {
    rtk::setup();
    rtk::Run run;

    VelocityProfileLimiter limiter;
    const size_t num_joints = 2;
    const double v_max = 1.0;
    const double a_max = 0.1;
    const double dt = 0.1;
    
    limiter.configure(num_joints, v_max, a_max, dt);

    // Independent reference state for validation
    std::vector<double> ref_state(num_joints, 0.0);
    const double dv_max = a_max * dt;

    // Generate a sequence of inputs: 
    // 1. Rapid ramp up (tests accel limit)
    // 2. Overshoot v_max (tests vel limit)
    // 3. Rapid drop (tests negative accel limit)
    // 4. Oscillations (tests state transitions)
    std::vector<double> inputs_j0, inputs_j1;
    for (int i = 0; i < 600; ++i) {
        if (i < 200) {
            inputs_j0.push_back(2.0); // Aim for high velocity
            inputs_j1.push_back(0.5); // Aim for medium velocity
        } else if (i < 400) {
            inputs_j0.push_back(-2.0); // Rapid reversal
            inputs_j1.push_back(0.5);  // Maintain
        } else {
            inputs_j0.push_back(std::sin(i * 0.1) * 2.0);
            inputs_j1.push_back(std::cos(i * 0.1) * 2.0);
        }
    }

    double d_vel[num_joints];
    double a_vel[num_joints];

    for (size_t i = 0; i < inputs_j0.size(); ++i) {
        d_vel[0] = inputs_j0[i];
        d_vel[1] = inputs_j1[i];

        rtk::guarded_void(run, [&] {
            limiter.process(d_vel, a_vel);
        });

        for (size_t j = 0; j < num_joints; ++j) {
            double desired = (j == 0) ? inputs_j0[i] : inputs_j1[i];
            double prev = ref_state[j];
            
            // Reference calculation
            double v_lim = prev;
            if (desired > prev) v_lim = std::min(desired, prev + dv_max);
            else if (desired < prev) v_lim = std::max(desired, prev - dv_max);
            v_lim = std::clamp(v_lim, -v_max, v_max);
            
            rtk::expect(run, rtk::near(a_vel[j], v_lim), "Joint " + std::to_string(j) + " at step " + std::to_string(i));
            ref_state[j] = v_lim;
        }
    }

    return rtk::report(run);
}
