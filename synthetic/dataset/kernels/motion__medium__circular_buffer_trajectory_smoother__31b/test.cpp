#include "kernel.hpp"
#include "rtk/check.hpp"
#include <vector>

int main() {
    rtk::setup();
    rtk::Run run;
    TrajectorySmoother smoother;

    const size_t WINDOW = 10;
    smoother.configure(WINDOW);

    // Local state to compute expected values
    std::vector<float> history;
    float expected_sum = 0.0f;

    auto calculate_expected = [&]() {
        if (history.empty()) return 0.0f;
        return expected_sum / history.size();
    };

    // 1. Warm-up phase: fill buffer partially
    for (int i = 1; i <= 5; ++i) {
        float val = static_cast<float>(i * 10);
        rtk::guarded_void(run, [&] { smoother.push(val); });
        
        history.push_back(val);
        expected_sum += val;

        float actual = rtk::guarded(run, [&] { return smoother.get_smoothed(); });
        rtk::expect(run, rtk::near(actual, calculate_expected()), "Warm-up average " + std::to_string(i));
    }

    // 2. Reach full capacity
    for (int i = 6; i <= 10; ++i) {
        float val = static_cast<float>(i * 10);
        rtk::guarded_void(run, [&] { smoother.push(val); });
        
        history.push_back(val);
        expected_sum += val;

        float actual = rtk::guarded(run, [&] { return smoother.get_smoothed(); });
        rtk::expect(run, rtk::near(actual, calculate_expected()), "Filling average " + std::to_string(i));
    }

    // 3. Wrap-around phase: push 500 samples to ensure multiple cycles
    for (int i = 0; i < 500; ++i) {
        float val = static_cast<float>(i);
        rtk::guarded_void(run, [&] { smoother.push(val); });

        // Maintain local window for ground truth
        if (history.size() >= WINDOW) {
            expected_sum -= history[0];
            history.erase(history.begin());
        }
        history.push_back(val);
        expected_sum += val;

        float actual = rtk::guarded(run, [&] { return smoother.get_smoothed(); });
        rtk::expect(run, rtk::near(actual, calculate_expected()), "Wrap-around average iter " + std::to_string(i));
    }

    // 4. Test zero/negative values
    rtk::guarded_void(run, [&] { smoother.push(-100.0f); });
    if (history.size() >= WINDOW) {
        expected_sum -= history[0];
        history.erase(history.begin());
    }
    history.push_back(-100.0f);
    expected_sum += -100.0f;
    
    float actual = rtk::guarded(run, [&] { return smoother.get_smoothed(); });
    rtk::expect(run, rtk::near(actual, calculate_expected()), "Negative value handling");

    return rtk::report(run);
}
