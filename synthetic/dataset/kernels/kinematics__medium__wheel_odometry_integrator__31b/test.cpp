#include "kernel.hpp"
#include "rtk/check.hpp"
#include <cmath>
#include <vector>
#include <string>

int main() {
    rtk::setup();
    rtk::Run run;

    WheelOdometryIntegrator k;
    const double base = 0.5;
    const double tpm = 1000.0;
    const size_t win = 5;
    
    k.configure(base, tpm, win);

    struct RefState {
        double x = 0.0, y = 0.0, theta = 0.0;
        std::vector<double> v_hist, w_hist;
        size_t head = 0;
        size_t count = 0;
        double base = 0.5;
        double tpm = 1000.0;
        size_t win = 5;

        void push(int32_t lt, int32_t rt, double dt) {
            if (dt <= 0.0) return;
            double dl = static_cast<double>(lt) / tpm;
            double dr = static_cast<double>(rt) / tpm;
            double v = (dl + dr) / (2.0 * dt);
            double w = (dr - dl) / (base * dt);
            
            v_hist[head] = v;
            w_hist[head] = w;
            head = (head + 1) % win;
            if (count < win) count++;
            
            double sv = 0, sw = 0;
            for(size_t i=0; i<count; ++i) { sv += v_hist[i]; sw += w_hist[i]; }
            double av = sv / count;
            double aw = sw / count;
            
            double dt_theta = aw * dt;
            x += av * std::cos(theta + dt_theta * 0.5) * dt;
            y += av * std::sin(theta + dt_theta * 0.5) * dt;
            theta += dt_theta;
            theta = std::atan2(std::sin(theta), std::cos(theta));
        }
    } ref;
    ref.v_hist.assign(win, 0.0);
    ref.w_hist.assign(win, 0.0);

    for (int i = 0; i < 200; ++i) {
        int32_t ticks = 10;
        double dt = 0.01;
        rtk::guarded_void(run, [&] { k.push_ticks(ticks, ticks, dt); });
        ref.push(ticks, ticks, dt);
        
        Pose p = rtk::guarded(run, [&] { return k.get_pose(); });
        rtk::expect(run, rtk::near(p.x, ref.x), "straight_x_" + std::to_string(i));
        rtk::expect(run, rtk::near(p.y, ref.y), "straight_y_" + std::to_string(i));
        rtk::expect(run, rtk::near(p.theta, ref.theta), "straight_theta_" + std::to_string(i));
    }

    for (int i = 0; i < 200; ++i) {
        int32_t lt = 5;
        int32_t rt = 15;
        double dt = 0.01;
        rtk::guarded_void(run, [&] { k.push_ticks(lt, rt, dt); });
        ref.push(lt, rt, dt);
        
        Pose p = rtk::guarded(run, [&] { return k.get_pose(); });
        rtk::expect(run, rtk::near(p.x, ref.x), "circle_x_" + std::to_string(i));
        rtk::expect(run, rtk::near(p.y, ref.y), "circle_y_" + std::to_string(i));
        rtk::expect(run, rtk::near(p.theta, ref.theta), "circle_theta_" + std::to_string(i));
    }

    for (int i = 0; i < 100; ++i) {
        int32_t lt = (i % 2 == 0) ? 100 : -100;
        int32_t rt = (i % 2 == 0) ? -100 : 100;
        double dt = (i % 10 == 0) ? 0.0 : 0.01;
        
        rtk::guarded_void(run, [&] { k.push_ticks(lt, rt, dt); });
        ref.push(lt, rt, dt);
        
        Pose p = rtk::guarded(run, [&] { return k.get_pose(); });
        rtk::expect(run, rtk::near(p.x, ref.x), "jitter_x_" + std::to_string(i));
        rtk::expect(run, rtk::near(p.y, ref.y), "jitter_y_" + std::to_string(i));
        rtk::expect(run, rtk::near(p.theta, ref.theta), "jitter_theta_" + std::to_string(i));
    }

    return rtk::report(run);
}
