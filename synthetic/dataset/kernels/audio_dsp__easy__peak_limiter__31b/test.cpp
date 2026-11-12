#include "kernel.hpp"
#include "rtk/check.hpp"
#include <vector>
#include <cmath>

int main() {
    rtk::setup();
    rtk::Run run;

    PeakLimiter k;
    const float threshold = 0.5f;
    const size_t lookahead = 10;
    
    k.configure(threshold, lookahead);

    // We will feed 1000 samples to ensure we cover wrap-around and many peaks
    const int num_samples = 1000;
    std::vector<float> inputs(num_samples);
    std::vector<float> expected(num_samples);
    
    // Generate test data: 
    // Indices 0-9: filling buffer (expect 0)
    // Indices 10-19: input peaks (expect 0)
    // Indices 20-29: expect clamped peaks
    for (int i = 0; i < num_samples; ++i) {
        // Every 15 samples, create a peak that exceeds threshold
        inputs[i] = (i % 15 == 0) ? 1.0f : 0.1f;
        // Create a negative peak
        if (i % 20 == 0) inputs[i] = -1.0f;
        
        // Manual reference implementation for checking
        // output[i] = input[i - lookahead] clamped
        if (i < (int)lookahead) {
            expected[i] = 0.0f;
        } else {
            float delayed = inputs[i - lookahead];
            if (delayed > threshold) expected[i] = threshold;
            else if (delayed < -threshold) expected[i] = -threshold;
            else expected[i] = delayed;
        }
    }

    for (int i = 0; i < num_samples; ++i) {
        float result = 0.0f;
        rtk::guarded_void(run, [&] {
            result = k.process(inputs[i]);
        });
        
        rtk::expect(run, rtk::near(result, expected[i]), "Sample " + std::to_string(i));
    }

    return rtk::report(run);
}
