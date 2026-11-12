#include "kernel.hpp"
#include "rtk/check.hpp"
#include <vector>
#include <string>

int main() {
    rtk::setup();
    rtk::Run run;

    FirLowpassFilter filter;
    // Asymmetric coefficients to ensure the order of convolution is checked
    // h[0]=0.5, h[1]=0.2, h[2]=0.1, h[3]=0.1
    std::vector<float> coeffs = {0.5f, 0.2f, 0.1f, 0.1f};
    filter.configure(coeffs);

    // Local reference for expected value calculation
    std::vector<float> local_history = {0.0f, 0.0f, 0.0f, 0.0f};

    for (int i = 0; i < 100; ++i) {
        // Use a sequence that doesn't repeat quickly to expose history errors
        float input = static_cast<float>(i) * 1.1f;
        
        float result = rtk::guarded(run, [&] { 
            return filter.process(input); 
        });

        // Independent calculation: Shift history and compute dot product
        local_history.insert(local_history.begin(), input);
        local_history.pop_back();
        
        float expected = 0.0f;
        for (size_t j = 0; j < coeffs.size(); ++j) {
            expected += coeffs[j] * local_history[j];
        }

        rtk::expect(run, rtk::near(result, expected), "Sample " + std::to_string(i));
    }

    return rtk::report(run);
}
