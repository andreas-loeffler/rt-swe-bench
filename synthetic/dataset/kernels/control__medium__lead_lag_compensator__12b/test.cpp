#include "kernel.hpp"
#include "rtk/check.hpp"
#include <vector>

int main() {
    rtk::setup();
    rtk::Run run;

    LeadLagCompensator k;
    
    // Test Parameters
    const double alpha = 0.2;
    const double beta = 0.1;
    
    k.configure(alpha, beta);

    // Reference state for manual calculation
    double ref_prev_x = 0.0;
    double ref_prev_y = 0.0;

    // Exercise 500 samples with varying inputs
    for (int i = 0; i < 500; ++i) {
        // Alternating inputs to test different dynamics
        double input = (i % 2 == 0) ? 1.0 : -1.0;
        
        // Manually calculate expected output
        double expected_y = alpha * input + (1.0 - alpha - beta) * ref_prev_y + beta * ref_prev_x;
        
        // Update reference state
        ref_prev_x = input;
        ref_prev_y = expected_y;

        double actual_y = rtk::guarded(run, [&] {
            return k.process(input);
        });

        rtk::expect(run, actual_y == expected_y, "process_match");
    }

    // Test Reset
    k.reset();
    ref_prev_x = 0.0;
    ref_prev_y = 0.0;
    
    double input_after_reset = 1.0;
    double expected_y_reset = alpha * input_after_reset + (1.0 - alpha - beta) * 0.0 + beta * 0.0;
    
    double actual_y_reset = rtk::guarded(run, [&] {
        return k.process(input_after_reset);
    });

    rtk::expect(run, actual_y_reset == expected_y_reset, "reset_check");

    return rtk::report(run);
}
