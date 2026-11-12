#include "kernel.hpp"
#include "rtk/check.hpp"
#include <vector>

int main() {
    rtk::setup();
    rtk::Run run;

    MovingAverageFilter filter;
    const size_t N = 10;
    filter.configure(N);

    // Test setup for reference calculation
    std::vector<double> history;
    auto get_expected = [&](double sample) {
        history.push_back(sample);
        double sum = 0.0;
        size_t start = history.size() > N ? history.size() - N : 0;
        size_t count = history.size() - start;
        for (size_t i = start; i < history.size(); ++i) {
            sum += history[i];
        }
        return sum / count;
    };

    // 1. Test ramp-up (empty to full)
    for (int i = 1; i <= N; ++i) {
        double val = static_cast<double>(i);
        double expected = get_expected(val);
        double actual = rtk::guarded(run, [&] { return filter.process(val); });
        rtk::near(run, actual, expected, 1e-9);
    }

    // 2. Test wrap-around and steady state
    for (int i = 11; i <= 500; ++i) {
        double val = static_cast<double>(i % 100);
        double expected = get_expected(val);
        double actual = rtk::guarded(run, [&] { return filter.process(val); });
        rtk::expect(run, rtk::near(actual, expected, 1e-9), "Iter " + std::to_string(i));
    }

    return rtk::report(run);
}
