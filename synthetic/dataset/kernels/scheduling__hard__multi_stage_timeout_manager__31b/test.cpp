#include "kernel.hpp"
#include "rtk/check.hpp"
#include <vector>
#include <map>

int main() {
    rtk::setup();
    rtk::Run run;

    MultiStageTimeoutManager manager;
    std::vector<double> thresholds = {1.0, 2.0, 5.0};
    size_t max_timers = 10;
    
    manager.configure(max_timers, thresholds);

    struct Mirror {
        double time = 0.0;
        bool active = false;
    };
    std::map<uint32_t, Mirror> mirror;

    auto get_expected_stage = [&](uint32_t id) {
        if (mirror.find(id) == mirror.end() || !mirror[id].active) return (uint8_t)0;
        uint8_t stage = 0;
        for (double th : thresholds) {
            if (mirror[id].time >= th) stage++;
            else break;
        }
        return stage;
    };

    // 1. Test sequential updates for one timer
    uint32_t first_id = 100;
    for (int i = 0; i < 100; ++i) {
        double dt = 0.1;
        if (i == 0) {
            rtk::guarded_void(run, [&] { manager.reset(first_id); });
            mirror[first_id].active = true;
            mirror[first_id].time = 0.0;
        }
        rtk::guarded_void(run, [&] { manager.update(first_id, dt); });
        mirror[first_id].time += dt;
        uint8_t actual = rtk::guarded(run, [&] { return manager.get_stage(first_id); });
        rtk::expect(run, actual == get_expected_stage(first_id), "Stage check loop");
    }

    // Clear first timer to make room for the concurrent test to hit exactly max_timers
    // Since there is no 'remove' API, we must account for first_id in our count.
    // We will use IDs 1-9.
    for (uint32_t id = 1; id <= 9; ++id) {
        rtk::guarded_void(run, [&] { manager.reset(id); });
        mirror[id].active = true;
        mirror[id].time = 0.0;
    }

    for (int i = 0; i < 200; ++i) {
        uint32_t id = (i % 9) + 1;
        double dt = 0.2;
        rtk::guarded_void(run, [&] { manager.update(id, dt); });
        mirror[id].time += dt;
        uint8_t actual = rtk::guarded(run, [&] { return manager.get_stage(id); });
        rtk::expect(run, actual == get_expected_stage(id), "Concurrent stage check");
    }

    // 3. Test reset and limit
    // Current active IDs: 100, 1, 2, 3, 4, 5, 6, 7, 8, 9 (Total 10)
    // Timer 11 should fail to be created
    rtk::guarded_void(run, [&] { manager.reset(11); });
    rtk::expect(run, rtk::guarded(run, [&] { return manager.get_stage(11); }) == 0, "Limit check");

    // Reset timer 1
    rtk::guarded_void(run, [&] { manager.reset(1); });
    mirror[1].time = 0.0;
    rtk::expect(run, rtk::guarded(run, [&] { return manager.get_stage(1); }) == 0, "Reset check");

    // Test a huge delta jump
    rtk::guarded_void(run, [&] { manager.update(2, 100.0); });
    mirror[2].time += 100.0;
    rtk::expect(run, rtk::guarded(run, [&] { return manager.get_stage(2); }) == 3, "Jump check");

    return rtk::report(run);
}
