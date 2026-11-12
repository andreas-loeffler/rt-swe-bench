#include "kernel.hpp"
#include "rtk/check.hpp"
#include <vector>
#include <cmath>
#include <string>

int main() {
    rtk::setup();
    rtk::Run run;

    PeakHoldAnalyzer analyzer;
    float decay = 2.0f; // Changed from 1.0f to ensure division is tested
    float min_sat = -10.0f;
    float max_sat = 10.0f;
    analyzer.configure(decay, min_sat, max_sat);

    // Local state to track expected values independently
    float exp_max = max_sat;
    float exp_min = min_sat;

    auto update_expected = [&](float sample, float dt) {
        float clamped = sample;
        if (clamped < min_sat) clamped = min_sat;
        if (clamped > max_sat) clamped = max_sat;

        if (clamped > exp_max) {
            exp_max = clamped;
        } else {
            exp_max -= (exp_max - clamped) * (dt / decay);
        }

        if (clamped < exp_min) {
            exp_min = clamped;
        } else {
            exp_min += (clamped - exp_min) * (dt / decay);
        }
    };

    // Test suite: 600 iterations covering saturation, peaks, and decay
    for (int i = 0; i < 600; ++i) {
        float sample;
        float dt = 0.01f;

        if (i < 100) {
            // ramp up to test saturation
            sample = (float)i * 0.2f; 
        } else if (i < 200) {
            // hold high (decay should be 0 as clamped == exp_max)
            sample = 20.0f; 
        } else if (i < 300) {
            // drop low to test max decay and min peak
            sample = -20.0f;
        } else if (i < 400) {
            // return to middle to test both decay
            sample = 0.0f;
        } else if (i < 500) {
            // oscillating values
            sample = (i % 2 == 0) ? 5.0f : -5.0f;
        } else {
            // random-ish noise
            sample = std::sin((float)i);
        }

        update_expected(sample, dt);

        rtk::guarded_void(run, [&] {
            analyzer.process(sample, dt);
        });

        auto res = rtk::guarded(run, [&] {
            return analyzer.get_peaks();
        });

        rtk::expect(run, rtk::near(res.max_peak, exp_max), "Max Peak Match Iter " + std::to_string(i));
        rtk::expect(run, rtk::near(res.min_peak, exp_min), "Min Peak Match Iter " + std::to_string(i));
    }

    return rtk::report(run);
}
