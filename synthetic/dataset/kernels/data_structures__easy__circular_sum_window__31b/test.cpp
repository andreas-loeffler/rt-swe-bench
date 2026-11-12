#include "kernel.hpp"
#include "rtk/check.hpp"
#include <vector>
#include <string>

int main() {
    rtk::setup();
    rtk::Run run;

    // Test Case 1: Window size 0 (catches line 17 changes)
    {
        CircularSumWindow k0;
        k0.configure(0);
        double result = rtk::guarded(run, [&]() {
            return k0.push(10.0);
        });
        rtk::expect(run, rtk::near(result, 0.0), "Zero window size returns 0");
    }

    // Test Case 2: Standard sliding window
    {
        CircularSumWindow k;
        const size_t N = 3;
        k.configure(N);

        // Sequence: 1.0, 2.0, 3.0, 4.0, 5.0
        // Expected sums:
        // push(1.0) -> 1.0
        // push(2.0) -> 3.0
        // push(3.0) -> 6.0
        // push(4.0) -> 2+3+4 = 9.0 (1.0 drops)
        // push(5.0) -> 3+4+5 = 12.0 (2.0 drops)
        
        double inputs[] = {1.0, 2.0, 3.0, 4.0, 5.0};
        double expected[] = {1.0, 3.0, 6.0, 9.0, 12.0};

        for (int i = 0; i < 5; ++i) {
            double result = rtk::guarded(run, [&]() {
                return k.push(inputs[i]);
            });
            rtk::expect(run, rtk::near(result, expected[i]), "Sum at step " + std::to_string(i));
        }
    }

    // Test Case 3: Large sequence to ensure wrap-around (catches line 19 etc)
    {
        CircularSumWindow k;
        const size_t N = 10;
        k.configure(N);

        double running_sum = 0.0;
        std::vector<double> history;
        
        for (int i = 0; i < 100; ++i) {
            double val = static_cast<double>(i + 1);
            running_sum += val;
            history.push_back(val);
            if (history.size() > N) {
                running_sum -= history[history.size() - N - 1];
            }

            double result = rtk::guarded(run, [&]() {
                return k.push(val);
            });
            rtk::expect(run, rtk::near(result, running_sum), "Long sequence sum " + std::to_string(i));
        }
    }

    return rtk::report(run);
}
