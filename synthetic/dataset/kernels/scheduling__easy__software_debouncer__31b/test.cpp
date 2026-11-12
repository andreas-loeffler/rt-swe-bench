#include "kernel.hpp"
#include "rtk/check.hpp"
#include <vector>

int main() {
    rtk::setup();
    rtk::Run run;

    SoftwareDebouncer debouncer;
    const uint32_t threshold = 5;
    debouncer.configure(threshold);

    // Test dataset: (input, expected_output)
    // We generate a sequence of 600 samples to exercise all branches
    struct Sample {
        bool input;
        bool expected;
    };
    std::vector<Sample> sequence;
    
    bool current_stable = false;
    uint32_t count = 0;

    for (int i = 0; i < 600; ++i) {
        // Generate patterns: 
        // 0-100: stability
        // 100-200: noise (alternating)
        // 200-300: slow transition
        // 300-400: interrupted transition
        // 400-600: random stress
        bool input;
        if (i < 100) input = false;
        else if (i < 200) input = (i % 2 == 0);
        else if (i < 300) input = (i >= 205); // stable change at 205
        else if (i < 400) input = (i % 3 != 0); // intermittent noise
        else input = (i % 7 == 0);

        // Reference logic for test expectation
        if (input == current_stable) {
            count = 0;
        } else {
            count++;
            if (count >= threshold) {
                current_stable = input;
                count = 0;
            }
        }
        sequence.push_back({input, current_stable});
    }

    for (const auto& s : sequence) {
        bool result = rtk::guarded(run, [&] { 
            return debouncer.process(s.input); 
        });
        rtk::expect(run, result == s.expected, "Debouncer consistency check");
    }

    return rtk::report(run);
}
