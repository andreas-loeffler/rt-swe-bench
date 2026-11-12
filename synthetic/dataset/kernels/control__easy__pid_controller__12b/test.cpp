#include "kernel.hpp"
#include "rtk/check.hpp"

int main() {
    rtk::setup();
    rtk::Run run;

    const double Kp = 1.5;
    const double Ki = 0.5;
    const double Kd = 0.1;
    const double dt = 0.1;

    PidController k;
    k.configure(Kp, Ki, Kd, dt);

    double current_integral = 0.0;
    double prev_error = 0.0;

    // Test 500 iterations of a descending setpoint
    // and 500 iterations of an ascending setpoint to exercise range
    for (int i = 0; i < 1000; ++i) {
        double setpoint = (i < 500) ? (100.0 - i * 0.1) : (i * 0.1);
        double current_val = 50.0; // Static value to ensure error changes

        // Reference calculation
        double error = setpoint - current_val;
        current_integral += error * dt;
        double derivative = (error - prev_error) / dt;
        double expected = (Kp * error) + (Ki * current_integral) + (Kd * derivative);
        
        // Update reference state
        prev_error = error;

        // Real-time call
        double actual = rtk::guarded(run, [&] { 
            return k.process(setpoint, current_val); 
        });

        rtk::near(actual, expected, 1e-9, "PID calculation at step " + std::to_string(i));
    }

    // Edge case: Zero gains
    k.configure(0.0, 0.0, 0.0, dt);
    rtk::expect(run, k.process(10.0, 5.0) == 0.0, "Zero gains output zero");

    // Edge case: Large dt
    k.configure(1.0, 1.0, 1.0, 1000.0);
    // First call
    double error1 = 10.0 - 0.0;
    double expected1 = (1.0 * error1) + (1.0 * (error1 * 1000.0)) + (1.0 * (error1 / 1000.0));
    rtk::near(k.process(10.0, 0.0), expected1, 1e-9, "Large dt first call");

    return rtk::report(run);
}
