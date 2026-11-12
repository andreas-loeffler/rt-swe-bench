#include "kernel.hpp"
#include "rtk/check.hpp"
#include <vector>
#include <algorithm>
#include <string>

int main() {
    rtk::setup();
    rtk::Run run;

    const size_t capacity = 10;
    CircularBufferStats k;
    k.configure(capacity);

    // Test 1: Fill buffer and check progression
    std::vector<double> inputs;
    for (int i = 0; i < 1000; ++i) {
        double val = (i % 2 == 0) ? (double)i : (double)(i + 100);
        inputs.push_back(val);
    }

    std::vector<double> buffer;
    auto get_median = [](std::vector<double>& b) -> double {
        std::vector<double> sorted = b;
        std::sort(sorted.begin(), sorted.end());
        if (sorted.empty()) return 0.0;
        size_t n = sorted.size();
        if (n % 2 == 1) return sorted[n / 2];
        return (sorted[n / 2 - 1] + sorted[n / 2]) / 2.0;
    };

    for (size_t i = 0; i < inputs.size(); ++i) {
        double input = inputs[i];
        
        if (buffer.size() < capacity) {
            buffer.push_back(input);
        } else {
            buffer[i % capacity] = input;
        }
        
        double expected = get_median(buffer);

        double result = rtk::guarded(run, [&] {
            return k.process(input);
        });

        rtk::near(expected, result, 1e-9, "Median check at index " + std::to_string(i));
    }

    // Verification: Manual check of specific points
    // Point 1: [1.0] -> 1.0
    CircularBufferStats k1;
    k1.configure(5);
    rtk::guarded(run, [&] { return k1.process(1.0); });
    rtk::near(1.0, 1.0, 1e-9, "Median of [1.0] is 1.0");

    // Point 2: [1.0, 2.0] -> 1.5
    CircularBufferStats k2;
    k2.configure(5);
    k2.process(1.0);
    double res2 = rtk::guarded(run, [&] { return k2.process(2.0); });
    rtk::near(1.5, res2, 1e-9, "Median of [1.0, 2.0] is 1.5");

    // Point 3: [1.0, 2.0, 3.0] -> 2.0
    CircularBufferStats k3;
    k3.configure(5);
    k3.process(1.0);
    k3.process(2.0);
    double res3 = rtk::guarded(run, [&] { return k3.process(3.0); });
    rtk::near(2.0, res3, 1e-9, "Median of [1.0, 2.0, 3.0] is 2.0");

    // Point 4: [1.0, 2.0, 3.0, 4.0] -> 2.5
    CircularBufferStats k4;
    k4.configure(5);
    k4.process(1.0);
    k4.process(2.0);
    k4.process(3.0);
    double res4 = rtk::guarded(run, [&] { return k4.process(4.0); });
    rtk::near(2.5, res4, 1e-9, "Median of [1.0, 2.0, 3.0, 4.0] is 2.5");

    // Point 5: Overwriting capacity (Capacity 2)
    // [1, 2] -> 1.5
    // [1, 2, 3] -> [2, 3] -> 2.5
    // [2, 3, 4] -> [3, 4] -> 3.5
    CircularBufferStats k5;
    k5.configure(2);
    k5.process(1.0);
    k5.process(2.0);
    double res5a = rtk::guarded(run, [&] { return k5.process(3.0); });
    rtk::near(2.5, res5a, 1e-9, "Median of [2, 3] is 2.5");
    double res5b = rtk::guarded(run, [&] { return k5.process(4.0); });
    rtk::near(3.5, res5b, 1e-9, "Median of [3, 4] is 3.5");

    return rtk::report(run);
}
