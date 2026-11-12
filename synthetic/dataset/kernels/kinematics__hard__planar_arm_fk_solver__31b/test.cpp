#include "kernel.hpp"
#include "rtk/check.hpp"
#include <vector>
#include <cmath>

int main() {
    rtk::setup();
    rtk::Run run;

    PlanarArmKinematics k;
    const size_t DOF = 3;
    double lengths[DOF] = {1.0, 1.0, 1.0};
    k.configure(DOF, lengths);

    for (int i = 0; i < 500; ++i) {
        // Generate deterministic inputs
        double qs[DOF];
        double qdots[DOF];
        for (size_t j = 0; j < DOF; ++j) {
            qs[j] = 0.1 * i * (j + 1);
            qdots[j] = 0.01 * i * (j + 1);
        }

        double pose[3];
        rtk::guarded_void(run, [&] {
            k.solve(qs, pose);
        });

        // Independent verification for solve
        double exp_x = 0, exp_y = 0, exp_phi = 0;
        for (size_t j = 0; j < DOF; ++j) {
            exp_phi += qs[j];
            exp_x += lengths[j] * std::cos(exp_phi);
            exp_y += lengths[j] * std::sin(exp_phi);
        }
        rtk::expect(run, rtk::near(pose[0], exp_x), "pose_x");
        rtk::expect(run, rtk::near(pose[1], exp_y), "pose_y");
        rtk::expect(run, rtk::near(pose[2], exp_phi), "pose_phi");

        double vel[3];
        rtk::guarded_void(run, [&] {
            k.solve_velocity(qs, qdots, vel);
        });

        // Independent verification for solve_velocity
        double exp_vx = 0, exp_vy = 0, exp_omega = 0;
        for (size_t j = 0; j < DOF; ++j) {
            double col_vx = 0, col_vy = 0;
            double temp_phi = 0;
            for (size_t m = 0; m < DOF; ++m) {
                temp_phi += qs[m];
                if (m >= j) {
                    col_vx -= lengths[m] * std::sin(temp_phi);
                    col_vy += lengths[m] * std::cos(temp_phi);
                }
            }
            exp_vx += col_vx * qdots[j];
            exp_vy += col_vy * qdots[j];
            exp_omega += qdots[j];
        }
        rtk::expect(run, rtk::near(vel[0], exp_vx), "vel_x");
        rtk::expect(run, rtk::near(vel[1], exp_vy), "vel_y");
        rtk::expect(run, rtk::near(vel[2], exp_omega), "vel_omega");

        if (i % 100 == 0) {
            rtk::guarded_void(run, [&] {
                k.set_mode(KinematicMode::REDUCED_PRECISION);
            });
        }
    }

    return rtk::report(run);
}
