#include "kernel.hpp"
#include "rtk/check.hpp"
#include <vector>
#include <string>

int main() {
    rtk::setup();
    rtk::Run run;

    {
        // Test Case 1: Boundary check for expiration (<= 0.0)
        // If the code uses < 0.0 instead of <= 0.0, it will fail to trigger exactly at timeout.
        WatchdogSupervisor supervisor;
        supervisor.configure(1, {0.1});
        
        // Exactly 0.1s elapsed: should be failsafe
        bool res = rtk::guarded(run, [&] { return supervisor.process(0.1); });
        rtk::expect(run, res == true, "Failsafe should trigger exactly at timeout (<= 0.0)");
    }

    {
        // Test Case 2: Correct indexing/looping
        // If the loop is i < num_sources - 1 or similar, the last source is ignored.
        WatchdogSupervisor supervisor;
        const std::vector<double> timeouts = {1.0, 1.0, 0.1};
        supervisor.configure(3, timeouts);
        
        // Advance by 0.11s: Only the 3rd source (index 2) should expire
        bool res = rtk::guarded(run, [&] { return supervisor.process(0.11); });
        rtk::expect(run, res == true, "Failsafe should trigger when the last source expires");
    }

    {
        // Test Case 3: Integration and Sticky state
        WatchdogSupervisor supervisor;
        const size_t num_src = 3;
        const std::vector<double> timeouts = {0.1, 0.5, 1.0};
        supervisor.configure(num_src, timeouts);

        // Independent state tracking for the loop
        std::vector<double> ref_timers = timeouts;
        bool ref_failsafe = false;

        auto ref_kick = [&](size_t id) {
            if (id < num_src) ref_timers[id] = timeouts[id];
        };

        auto ref_process = [&](double dt) {
            if (ref_failsafe) return true;
            for (size_t i = 0; i < num_src; ++i) {
                ref_timers[i] -= dt;
                if (ref_timers[i] <= 0.0) {
                    ref_failsafe = true;
                    return true;
                }
            }
            return false;
        };

        for (int i = 0; i < 600; ++i) {
            if (i % 10 == 0) {
                size_t id = i % num_src;
                rtk::guarded_void(run, [&] { supervisor.kick(id); });
                ref_kick(id);
            }

            double dt = 0.02;
            bool result = rtk::guarded(run, [&] { return supervisor.process(dt); });
            bool expected = ref_process(dt);
            rtk::expect(run, result == expected, "Step " + std::to_string(i));
        }

        // Verify stickiness
        rtk::guarded_void(run, [&] { supervisor.process(2.0); });
        for (size_t id = 0; id < num_src; ++id) {
            rtk::guarded_void(run, [&] { supervisor.kick(id); });
        }
        bool result_after_kick = rtk::guarded(run, [&] { return supervisor.process(0.01); });
        rtk::expect(run, result_after_kick == true, "Failsafe should be sticky");
    }

    return rtk::report(run);
}
