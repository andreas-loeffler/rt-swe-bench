#include "kernel.hpp"
#include "rtk/check.hpp"
#include <vector>
#include <numeric>

int main() {
    rtk::setup();
    rtk::Run run;

    JointVelocityFilter filter;
    const size_t window_size = 10;
    filter.configure(window_size);

    // Reference state for independent verification
    std::vector<double> ref_buffer;
    double ref_sum = 0.0;

    auto get_expected = [&](double sample) {
        if (ref_buffer.size() < window_size) {
            ref_buffer.push_back(sample);
            ref_sum += sample;
        } else {
            ref_sum -= ref_buffer[ref_buffer.size() % window_size]; // This is conceptually wrong for a simple vector, use a proper ring
            // Re-calculating sum for absolute correctness in test harness
            // but the logic below handles it properly.
        }
        return 0.0; // Placeholder, logic handled in loop
    };

    // Since we need 500+ calls and deterministic inputs
    // We will simulate the window manually in the test loop
    std::vector<double> history;
    
    for (int i = 0; i < 1000; ++i) {
        double sample = static_cast<double>(i);
        
        // Compute expected value independently
        history.push_back(sample);
        if (history.size() > window_size) {
            history.erase(history.begin());
        }
        double expected_sum = std::accumulate(history.begin(), history.end(), 0.0);
        double expected_avg = expected_sum / history.size();

        double actual = rtk::guarded(run, [&] {
            return filter.process(sample);
        });

        rtk::expect(run, rtk::near(actual, expected_avg), "Iter " + std::to_string(i));
    }

    return rtk::report(run);
}
