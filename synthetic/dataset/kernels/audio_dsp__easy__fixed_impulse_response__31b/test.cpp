#include "kernel.hpp"
#include "rtk/check.hpp"
#include <vector>
#include <span>
#include <string>

int main() {
    rtk::setup();
    rtk::Run run;

    ImpulseResponseConvolver k;
    std::vector<float> coeffs = {0.5f, 0.2f, -0.1f}; // Length 3
    k.configure(coeffs);

    // To verify, we maintain a local history for the "plain code" expectation
    std::vector<float> history(coeffs.size(), 0.0f);
    auto expected_calc = [&](float input) {
        history.insert(history.begin(), input);
        history.pop_back();
        float sum = 0.0f;
        for (size_t i = 0; i < coeffs.size(); ++i) {
            sum += coeffs[i] * history[i];
        }
        return sum;
    };

    // Test 1: Impulse response (step responses)
    std::vector<float> inputs = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f};
    for (float in : inputs) {
        float expected = expected_calc(in);
        float actual = rtk::guarded(run, [&] { return k.process(in); });
        rtk::expect(run, rtk::near(actual, expected, 1e-6f), "Impulse step");
    }

    // Test 2: Long sequence to test wrap-around and stability
    for (int i = 0; i < 1000; ++i) {
        float in = static_cast<float>(i % 10) * 0.1f;
        float expected = expected_calc(in);
        
        float actual = rtk::guarded(run, [&] { return k.process(in); });
        rtk::expect(run, rtk::near(actual, expected, 1e-6f), "Iter " + std::to_string(i));
    }

    return rtk::report(run);
}
