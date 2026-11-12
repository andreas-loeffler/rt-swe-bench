#include "kernel.hpp"
#include "rtk/check.hpp"
#include <vector>
#include <cmath>
#include <string>

double calculate_expected_slope(const std::vector<double>& data) {
    if (data.size() < 2) return 0.0;
    double n = static_cast<double>(data.size());
    double sum_x = 0, sum_y = 0, sum_xy = 0, sum_x2 = 0;
    for (size_t i = 0; i < data.size(); ++i) {
        sum_x += i;
        sum_y += data[i];
        sum_xy += i * data[i];
        sum_x2 += i * i;
    }
    double denom = (n * sum_x2) - (sum_x * sum_x);
    return (denom == 0) ? 0.0 : ((n * sum_xy) - (sum_x * sum_y)) / denom;
}

int main() {
    rtk::setup();
    rtk::Run run;

    LinearRegressionEstimator k;
    const size_t window_size = 10;
    k.configure(window_size);

    std::vector<double> history;
    
    // Test 1000 iterations of a ramping signal with noise
    for (int i = 0; i < 1000; ++i) {
        double val = static_cast<double>(i) * 0.5 + (i % 3 == 0 ? 0.1 : -0.1);
        
        double actual = rtk::guarded(run, [&] { 
            return k.push(val); 
        });

        history.push_back(val);
        if (history.size() > window_size) {
            history.erase(history.begin());
        }

        double expected = calculate_expected_slope(history);
        rtk::expect(run, rtk::near(actual, expected), "Slope match at iter " + std::to_string(i));
    }

    // Test edge case: Constant signal
    history.clear();
    // Reset the kernel to clear previous window state for the constant test
    k.configure(window_size); 
    for (int i = 0; i < 20; ++i) {
        double val = 5.0;
        double actual = rtk::guarded(run, [&] { return k.push(val); });
        history.push_back(val);
        if (history.size() > window_size) history.erase(history.begin());
        rtk::expect(run, rtk::near(actual, 0.0), "Constant signal slope at " + std::to_string(i));
    }

    return rtk::report(run);
}
