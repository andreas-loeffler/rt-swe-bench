#include "kernel.hpp" // Using reference.hpp as kernel.hpp
#include "rtk/check.hpp"
#include <vector>
#include <cmath>

int main() {
    rtk::setup();
    rtk::Run run;

    AlphaBetaFilter filter;
    const double dt = 0.01;
    const double alpha = 0.5;
    const double beta = 0.1;
    filter.configure(dt, alpha, beta);

    // Independent oracle state
    double oracle_x_hat = 0.0;
    double oracle_v_hat = 0.0;
    bool oracle_initialized = false;

    // Generate 1000 deterministic inputs (a ramp with noise)
    std::vector<double> inputs;
    for (int i = 0; i < 1000; ++i) {
        inputs.push_back(static_cast<double>(i) * 0.1 + std::sin(i * 0.1));
    }

    for (double z : inputs) {
        double expected;
        if (!oracle_initialized) {
            oracle_x_hat = z;
            oracle_v_hat = 0.0;
            oracle_initialized = true;
            expected = z;
            oracle_x_hat += oracle_v_hat * dt;
        } else {
            double innovation = z - oracle_x_hat;
            double x_final = oracle_x_hat + alpha * innovation;
            double v_final = oracle_v_hat + (beta / dt) * innovation;
            expected = x_final;
            oracle_x_hat = x_final + v_final * dt;
            oracle_v_hat = v_final;
        }

        double result = rtk::guarded(run, [&] {
            return filter.process(z);
        });

        rtk::expect(run, rtk::near(result, expected), "Filter output matches oracle");
    }

    return rtk::report(run);
}
