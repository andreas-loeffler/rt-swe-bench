#include "kernel.hpp"
#include "rtk/check.hpp"
#include <vector>
#include <numeric>

int main() {
    rtk::setup();
    rtk::Run run;

    MovingAverageFilter k;
    const size_t N = 10;
    k.configure(N);

    // We will feed a ramp signal and verify the average
    // The test harness computes the ground truth using a simple vector
    std::vector<float> history;
    
    for (int i = 0; i < 1000; ++i) {
        float input = static_cast<float>(i);
        
        float result = rtk::guarded(run, [&] { 
            return k.process(input); 
        });

        // Independent calculation for ground truth
        history.push_back(input);
        float expected = 0.0f;
        size_t start = history.size() > N ? history.size() - N : 0;
        double sum = 0.0;
        for (size_t j = start; j < history.size(); ++j) {
            sum += history[j];
        }
        expected = static_cast<float>(sum / (history.size() - start));

        rtk::expect(run, rtk::near(result, expected), "Sample " + std::to_string(i));
    }

    return rtk::report(run);
}
