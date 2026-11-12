#include "kernel.hpp"
#include "rtk/check.hpp"
#include <vector>
#include <cmath>

int main() {
    rtk::setup();
    rtk::Run run;

    EnvelopeFollower ef;
    const float fs = 44100.0f;
    const float attack = 0.01f;
    const float release = 0.1f;
    
    ef.configure(fs, attack, release);

    // Pre-calculate expected values using plain code
    const float aCoef = 1.0f - std::exp(-1.0f / (attack * fs));
    const float rCoef = 1.0f - std::exp(-1.0f / (release * fs));
    
    std::vector<float> inputs;
    std::vector<float> expected;
    
    float currentEnv = 0.0f;
    
    // Test sequence: 
    // 1. Rapid rise (attack)
    for(int i=0; i<200; ++i) {
        inputs.push_back(1.0f);
        currentEnv += aCoef * (1.0f - currentEnv);
        expected.push_back(currentEnv);
    }
    // 2. Sudden drop to 0 (release)
    for(int i=0; i<200; ++i) {
        inputs.push_back(0.0f);
        currentEnv -= rCoef * (currentEnv - 0.0f);
        expected.push_back(currentEnv);
    }
    // 3. Oscillation/Negative values
    for(int i=0; i<200; ++i) {
        float val = (i % 2 == 0) ? -1.0f : 0.5f;
        inputs.push_back(val);
        float absVal = std::abs(val);
        if (absVal > currentEnv) {
            currentEnv += aCoef * (absVal - currentEnv);
        } else {
            currentEnv -= rCoef * (currentEnv - absVal);
        }
        expected.push_back(currentEnv);
    }

    for (size_t i = 0; i < inputs.size(); ++i) {
        float result = rtk::guarded(run, [&]() { 
            return ef.step(inputs[i]); 
        });
        rtk::expect(run, rtk::near(result, expected[i]), "Sample " + std::to_string(i));
    }

    return rtk::report(run);
}
