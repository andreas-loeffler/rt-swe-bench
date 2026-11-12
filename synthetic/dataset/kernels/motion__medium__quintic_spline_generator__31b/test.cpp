#include "kernel.hpp"
#include "rtk/check.hpp"
#include <vector>
#include <cmath>

int main() {
    rtk::setup();
    rtk::Run run;

    QuinticSplineGenerator k;
    const double T = 2.0;
    const double P0 = 10.0;
    const double P1 = 20.0;

    rtk::guarded_void(run, [&] { 
        k.configure(T); 
    });
    rtk::guarded_void(run, [&] { 
        k.set_targets(P0, P1); 
    });

    auto expected_state = [&](double t) {
        if (t <= 0.0) return QuinticSplineGenerator::State{P0, 0.0, 0.0};
        if (t >= T) return QuinticSplineGenerator::State{P1, 0.0, 0.0};
        
        double x = t / T;
        double dp = P1 - P0;
        double p = P0 + dp * (10.0 * std::pow(x, 3) - 15.0 * std::pow(x, 4) + 6.0 * std::pow(x, 5));
        double v = dp * (30.0 * std::pow(x, 2) - 60.0 * std::pow(x, 3) + 30.0 * std::pow(x, 4)) / T;
        double a = dp * (60.0 * x - 180.0 * std::pow(x, 2) + 120.0 * std::pow(x, 3)) / (T * T);
        return QuinticSplineGenerator::State{p, v, a};
    };

    // Test under-run, mid-run, and over-run
    for (int i = 0; i < 600; ++i) {
        double t = -1.0 + (static_cast<double>(i) / 200.0); // range [-1.0, 2.0]
        auto exp = expected_state(t);
        
        auto act = rtk::guarded(run, [&] { 
            return k.step(t); 
        });

        rtk::near(run, act.position, exp.position);
        rtk::near(run, act.velocity, exp.velocity);
        rtk::near(run, act.acceleration, exp.acceleration);
    }

    // Test dynamic target update
    rtk::guarded_void(run, [&] { 
        k.set_targets(0.0, 100.0); 
    });
    
    auto exp_new = expected_state(1.0); // Using previous T=2.0 logic but with new targets
    // We must recreate a local expected for the new targets
    auto get_exp_custom = [&](double t, double p0, double p1, double dur) {
        if (t <= 0.0) return QuinticSplineGenerator::State{p0, 0.0, 0.0};
        if (t >= dur) return QuinticSplineGenerator::State{p1, 0.0, 0.0};
        double x = t / dur;
        double dp = p1 - p0;
        return QuinticSplineGenerator::State{
            p0 + dp * (10.0*std::pow(x,3)-15.0*std::pow(x,4)+6.0*std::pow(x,5)),
            dp * (30.0*std::pow(x,2)-60.0*std::pow(x,3)+30.0*std::pow(x,4))/dur,
            dp * (60.0*x-180.0*std::pow(x,2)+120.0*std::pow(x,3))/(dur*dur)
        };
    };

    for (int i = 0; i < 100; ++i) {
        double t = (static_cast<double>(i) / 100.0) * T;
        auto exp = get_exp_custom(t, 0.0, 100.0, T);
        auto act = rtk::guarded(run, [&] { return k.step(t); });
        rtk::near(run, act.position, exp.position);
        rtk::near(run, act.velocity, exp.velocity);
        rtk::near(run, act.acceleration, exp.acceleration);
    }

    return rtk::report(run);
}
