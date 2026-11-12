#include "kernel.hpp"
#include "rtk/check.hpp"
#include <vector>
#include <cmath>

int main() {
    rtk::setup();
    rtk::Run run;

    AlphaBetaTracker k;
    
    const double dt = 0.1;
    const double af = 0.8;
    const double bf = 0.3;
    const double as = 0.1;
    const double bs = 0.05;
    const double thresh = 0.5;

    rtk::guarded_void(run, [&] {
        k.configure(dt, af, bf, as, bs, thresh);
    });

    // Local shadow state for verification
    double ref_pos = 0;
    double ref_vel = 0;
    bool ref_init = false;

    auto step_ref = [&](double z) {
        if (!ref_init) {
            ref_pos = z;
            ref_vel = 0;
            ref_init = true;
            return;
        }
        double pred = ref_pos + ref_vel * dt;
        double err = z - pred;
        double a = (std::abs(err) > thresh) ? af : as;
        double b = (std::abs(err) > thresh) ? bf : bs;
        ref_pos = pred + a * err;
        ref_vel = ref_vel + (b * err) / dt;
    };

    // Test Case 1: Constant velocity ramp (Slow mode)
    for (int i = 0; i < 200; ++i) {
        double val = i * 0.1; // velocity = 1.0
        rtk::guarded_void(run, [&] { k.push(val); });
        step_ref(val);
        rtk::expect(run, rtk::near(k.get_position(), ref_pos), "pos_ramp_" + std::to_string(i));
        rtk::expect(run, rtk::near(k.get_velocity(), ref_vel), "vel_ramp_" + std::to_string(i));
    }

    // Test Case 2: Sudden jump (Fast mode trigger)
    for (int i = 0; i < 100; ++i) {
        double val = 20.0 + i * 0.1;
        rtk::guarded_void(run, [&] { k.push(val); });
        step_ref(val);
        rtk::expect(run, rtk::near(k.get_position(), ref_pos), "pos_jump_" + std::to_string(i));
        rtk::expect(run, rtk::near(k.get_velocity(), ref_vel), "vel_jump_" + std::to_string(i));
    }

    // Test Case 3: High frequency noise (Toggling modes)
    for (int i = 0; i < 200; ++i) {
        double noise = (i % 2 == 0) ? 1.0 : -1.0;
        double val = 20.0 + noise;
        rtk::guarded_void(run, [&] { k.push(val); });
        step_ref(val);
        rtk::expect(run, rtk::near(k.get_position(), ref_pos), "pos_noise_" + std::to_string(i));
        rtk::expect(run, rtk::near(k.get_velocity(), ref_vel), "vel_noise_" + std::to_string(i));
    }

    // Test Case 4: Reset
    rtk::guarded_void(run, [&] { k.reset(); });
    ref_pos = 0; ref_vel = 0; ref_init = false;
    rtk::expect(run, rtk::near(k.get_position(), 0.0), "reset_pos");
    rtk::expect(run, rtk::near(k.get_velocity(), 0.0), "reset_vel");

    // Final verification after reset
    for (int i = 0; i < 50; ++i) {
        double val = i * 0.5;
        rtk::guarded_void(run, [&] { k.push(val); });
        step_ref(val);
        rtk::expect(run, rtk::near(k.get_position(), ref_pos), "post_reset_pos_" + std::to_string(i));
        rtk::expect(run, rtk::near(k.get_velocity(), ref_vel), "post_reset_vel_" + std::to_string(i));
    }

    return rtk::report(run);
}
