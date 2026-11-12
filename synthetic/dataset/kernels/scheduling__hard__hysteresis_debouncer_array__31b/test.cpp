#include "kernel.hpp"
#include "rtk/check.hpp"
#include <vector>
#include <string>

int main() {
    rtk::setup();
    rtk::Run run;

    HysteresisDebouncerArray k;
    const size_t num_ch = 3;
    const double period = 0.01; // 10ms
    const double threshold = 0.03; // 30ms (requires 3 consecutive samples of change)

    k.configure(num_ch, period, threshold);

    // Use std::vector<char> because std::vector<bool> is specialized and doesn't provide bool*
    std::vector<char> raw_inputs(num_ch, 0);
    std::vector<char> raw_outputs(num_ch, 0);

    // Reference state for local verification
    struct RefState {
        bool state = false;
        double timer = 0.0;
    };
    std::vector<RefState> ref(num_ch);

    auto update_ref = [&](size_t i, bool input) {
        if (input == ref[i].state) {
            ref[i].timer = 0.0;
        } else {
            ref[i].timer += period;
            if (ref[i].timer >= threshold) {
                ref[i].state = input;
                ref[i].timer = 0.0;
            }
        }
        return ref[i].state;
    };

    // Test 500+ iterations
    for (int step = 0; step < 600; ++step) {
        // Create deterministic noise
        // Channel 0: Slow transitions
        raw_inputs[0] = ((step / 100) % 2 == 0); 
        // Channel 1: Fast jitter (should not debounce)
        raw_inputs[1] = (step % 2 == 0);
        // Channel 2: Intermittent stability
        raw_inputs[2] = ((step > 200 && step < 205) || (step > 400 && step < 410));

        rtk::guarded_void(run, [&] {
            k.process(reinterpret_cast<const bool*>(raw_inputs.data()), reinterpret_cast<bool*>(raw_outputs.data()));
        });

        for (size_t i = 0; i < num_ch; ++i) {
            bool expected = update_ref(i, static_cast<bool>(raw_inputs[i]));
            rtk::expect(run, static_cast<bool>(raw_outputs[i]) == expected, "Step " + std::to_string(step) + " Ch " + std::to_string(i));
        }
    }

    return rtk::report(run);
}
