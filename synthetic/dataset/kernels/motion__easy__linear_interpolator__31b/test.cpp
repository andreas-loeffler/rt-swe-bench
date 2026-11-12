#include "kernel.hpp"
#include "rtk/check.hpp"

int main() {
    rtk::setup();
    rtk::Run run;

    {
        LinearInterpolator k;
        const float start = 10.0f;
        const float end = 20.0f;
        const size_t duration = 500;
        
        k.configure(start, end, duration);

        for (size_t i = 0; i < 1000; ++i) {
            float expected;
            if (i < duration) {
                expected = start + (end - start) * (static_cast<float>(i) / static_cast<float>(duration));
            } else {
                expected = end;
            }

            float actual = rtk::guarded(run, [&]() { return k.step(); });
            rtk::expect(run, rtk::near(actual, expected), "Sample " + std::to_string(i));
        }
    }

    {
        // Edge case: zero duration
        LinearInterpolator k_zero;
        k_zero.configure(10.0f, 20.0f, 0);
        for (int i = 0; i < 10; ++i) {
            float actual = rtk::guarded(run, [&]() { return k_zero.step(); });
            rtk::expect(run, rtk::near(actual, 20.0f), "Zero duration sample " + std::to_string(i));
        }
    }

    return rtk::report(run);
}
