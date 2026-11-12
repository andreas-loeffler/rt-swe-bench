#include "kernel.hpp"
#include "rtk/check.hpp"
#include <vector>
#include <numeric>

int main() {
    rtk::setup();
    rtk::Run run;

    SlidingWindowAverage k;
    const size_t window_size = 10;
    k.configure(window_size);

    std::vector<double> test_data(1000);
    for (size_t i = 0; i < test_data.size(); ++i) {
        test_data[i] = static_cast<double>(i * 1.5);
    }

    std::vector<double> window;
    for (size_t i = 0; i < test_data.size(); ++i) {
        double sample = test_data[i];
        
        // Independent calculation
        window.push_back(sample);
        if (window.size() > window_size) {
            window.erase(window.begin());
        }
        double expected_sum = std::accumulate(window.begin(), window.end(), 0.0);
        double expected_avg = expected_sum / window.size();

        double result = rtk::guarded(run, [&] { 
            return k.push(sample); 
        });

        rtk::expect(run, rtk::near(result, expected_avg), "Sample " + std::to_string(i));
    }

    return rtk::report(run);
}
