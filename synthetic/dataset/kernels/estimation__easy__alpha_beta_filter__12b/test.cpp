#include "kernel.hpp"
#include "rtk/check.hpp"
#include <vector>

int main() {
    rtk::setup();
    rtk::Run run;

    AlphaBetaFilter filter;
    double initial_alpha = 0.5;
    double initial_val = 10.0;
    filter.configure(initial_alpha, initial_val);

    // Test sequence: 500 steps
    // State update: s_next = alpha * measurement + (1 - alpha) * s_prev
    double current_state = initial_val;
    
    // Test different alphas and measurements to exercise the linear combination
    for (int i = 0; i < 500; ++i) {
        double measurement = static_cast<double>(i % 100);
        // Vary alpha based on i to ensure we aren't just testing a constant
        double alpha = static_cast<double>(i % 100) / 100.0;
        
        double expected = alpha * measurement + (1.0 - alpha) * current_state;
        
        double result = rtk::guarded(run, [&] { 
            return filter.process(measurement, alpha); 
        });

        rtk::expect(run, rtk::near(result, expected, 1e-9), "Filter step " + std::to_string(i));
        current_state = expected;
    }

    // Edge cases
    // Alpha = 1.0 (should exactly match measurement)
    double state_before_edge = current_state;
    double measurement_edge = 50.0;
    double result_edge = rtk::guarded(run, [&] {
        return filter.process(measurement_edge, 1.0);
    });
    rtk::expect(run, rtk::near(result_edge, 50.0, 1e-9), "Alpha 1.0 edge case");

    // Alpha = 0.0 (should remain at previous state)
    double state_before_zero = result_edge;
    double result_zero = rtk::guarded(run, [&] {
        return filter.process(measurement_edge, 0.0);
    });
    rtk::expect(run, rtk::near(result_zero, state_before_zero, 1e-9), "Alpha 0.0 edge case");

    return rtk::report(run);
}
