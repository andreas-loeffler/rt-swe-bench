#include "kernel.hpp"
#include "rtk/check.hpp"
#include <vector>
#include <cmath>

int main() {
    rtk::setup();
    rtk::Run run;

    VelocityRampClamp k;
    const double max_v = 10.0;
    const double max_a = 2.0;
    const double dt = 0.1;
    
    // Configure the kernel
    k.configure(max_v, max_a, dt);

    // Reference simulation state
    double ref_vel = 0.0;
    const double step_limit = max_a * dt;

    auto ref_step = [&](double target) {
        double clamped_target = std::clamp(target, -max_v, max_v);
        double diff = clamped_target - ref_vel;
        if (std::abs(diff) > step_limit) {
            diff = (diff > 0) ? step_limit : -step_limit;
        }
        ref_vel += diff;
        return ref_vel;
    };

    // Test Case 1: Positive Ramp to Saturation
    for (int i = 0; i < 100; ++i) {
        double target = 20.0; // Above max_v
        double expected = ref_step(target);
        double actual = rtk::guarded(run, [&] { return k.step(target); });
        rtk::expect(run, rtk::near(actual, expected), "Positive ramp step " + std::to_string(i));
    }

    // Test Case 2: Sign Flip (Positive to Negative)
    for (int i = 0; i < 100; ++i) {
        double target = -20.0; // Below -max_v
        double expected = ref_step(target);
        double actual = rtk::guarded(run, [&] { return k.step(target); });
        rtk::expect(run, rtk::near(actual, expected), "Negative ramp step " + std::to_string(i));
    }

    // Test Case 3: Small oscillations around zero
    for (int i = 0; i < 100; ++i) {
        double target = (i % 2 == 0) ? 0.05 : -0.05;
        double expected = ref_step(target);
        double actual = rtk::guarded(run, [&] { return k.step(target); });
        rtk::expect(run, rtk::near(actual, expected), "Oscillation step " + std::to_string(i));
    }

    // Test Case 4: Target within acceleration limit (Immediate reach)
    for (int i = 0; i < 100; ++i) {
        double target = ref_vel + (step_limit * 0.5);
        double expected = ref_step(target);
        double actual = rtk::guarded(run, [&] { return k.step(target); });
        rtk::expect(run, rtk::near(actual, expected), "Small step " + std::to_string(i));
    }

    // Test Case 5: Steady state check
    for (int i = 0; i < 100; ++i) {
        double target = ref_vel;
        double expected = ref_step(target);
        double actual = rtk::guarded(run, [&] { return k.step(target); });
        rtk::expect(run, rtk::near(actual, expected), "Steady state step " + std::to_string(i));
        
        double current = rtk::guarded(run, [&] { return k.get_velocity(); });
        rtk::expect(run, rtk::near(current, expected), "Getter check " + std::to_string(i));
    }

    return rtk::report(run);
}
