#include "kernel.hpp" // Using reference as kernel for testing
#include "rtk/check.hpp"
#include <vector>

int main() {
    rtk::setup();
    rtk::Run run;
    BiquadFilter k;

    // A simple low-pass filter example coefficients
    const float b0 = 0.1f;
    const float b1 = 0.2f;
    const float b2 = 0.1f;
    const float a1 = -0.5f;
    const float a2 = 0.2f;

    k.configure(b0, b1, b2, a1, a2);

    // Pre-calculate expected values using plain C++ (non-realtime)
    // Use the same mathematical definition to verify implementation
    std::vector<float> inputs;
    std::vector<float> expected;
    float x_val = 1.0f; 
    float z1_ref = 0.0f;
    float z2_ref = 0.0f;

    for (int i = 0; i < 600; ++i) {
        // Create a varying input signal (sine-like)
        float sample = std::sin(i * 0.1f);
        inputs.push_back(sample);
        
        // Reference Calculation
        float w_n = sample - (a1 * z1_ref) - (a2 * z2_ref);
        float y_n = (b0 * w_n) + (b1 * z1_ref) + (b2 * z2_ref);
        expected.push_back(y_n);
        
        z2_ref = z1_ref;
        z1_ref = w_n;
    }

    // Execute real-time calls
    for (size_t i = 0; i < inputs.size(); ++i) {
        float result = rtk::guarded(run, [&, i] {
            return k.process(inputs[i]);
        });
        rtk::near(run, result, expected[i], 1e-6f);
    }

    // Test reset functionality
    rtk::guarded_void(run, [&] { k.reset(); });
    
    // After reset, the first output of a unit impulse (1.0) should be b0 * 1.0
    float impulse = 1.0f;
    float result_post_reset = rtk::guarded(run, [&] {
        return k.process(impulse);
    });
    
    // Manual calc for post-reset: 
    // w[0] = 1.0 - 0 - 0 = 1.0
    // y[0] = b0*1.0 + 0 + 0 = b0
    rtk::near(run, result_post_reset, b0 * impulse, 1e-6f);

    return rtk::report(run);
}
