#include "kernel.hpp"
#include "rtk/check.hpp"
#include <vector>
#include <cmath>

int main() {
    rtk::setup();
    rtk::Run run;

    ComplementaryFilterBank k;
    const size_t axes = 3;
    const double fs = 100.0;
    const double alpha = 0.98;
    
    k.configure(axes, fs, alpha);

    // Test data generation
    const int iterations = 600;
    std::vector<double> gyros(axes);
    std::vector<double> accels(axes);
    std::vector<double> results(axes);
    
    // Expected values computed independently
    // State: state = alpha * (state + gyro/fs) + (1-alpha) * accel
    std::vector<double> expected_state(axes, 0.0);
    const double dt = 1.0 / fs;

    for (int i = 0; i < iterations; ++i) {
        // Deterministic inputs: slow sine wave for gyro, noise for accel
        for (size_t a = 0; a < axes; ++a) {
            gyros[a] = std::sin(i * 0.1 + a);
            accels[a] = std::cos(i * 0.05 + a);
            
            // Manual calculation of expected result
            double angle_gyro = expected_state[a] + (gyros[a] * dt);
            expected_state[a] = (alpha * angle_gyro) + ((1.0 - alpha) * accels[a]);
        }

        rtk::guarded_void(run, [&] {
            k.process(gyros.data(), accels.data(), results.data());
        });

        for (size_t a = 0; a < axes; ++a) {
            rtk::expect(run, rtk::near(results[a], expected_state[a], 1e-12), 
                        "Iteration " + std::to_string(i) + " axis " + std::to_string(a));
        }
    }

    return rtk::report(run);
}
