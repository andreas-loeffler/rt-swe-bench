#include "kernel.hpp" 
#include "rtk/check.hpp"
#include <vector>

int main() {
    rtk::setup();
    rtk::Run run;

    FixedCapacityCommandQueue q;
    const size_t CAP = 16;
    q.configure(CAP);

    // 1. Test Empty state
    uint32_t out_id = 0xdeadbeef;
    float out_val = -1.0f;
    bool popped_empty = rtk::guarded(run, [&] { return q.pop(out_id, out_val); });
    rtk::expect(run, popped_empty == false, "initial_pop_fail");

    // 2. Strict capacity and saturation test
    // Fill exactly to the power-of-two capacity (16)
    int pushed_count = 0;
    for (int i = 0; i < 32; ++i) {
        if (rtk::guarded(run, [&] { return q.push(i, (float)i); })) {
            pushed_count++;
        }
    }
    rtk::expect(run, pushed_count == 16, "capacity_is_exactly_16");

    // Verify FIFO and content
    for (int i = 0; i < 16; ++i) {
        uint32_t id;
        float val;
        bool popped = rtk::guarded(run, [&] { return q.pop(id, val); });
        rtk::expect(run, popped, "should_pop_16_items");
        rtk::expect(run, id == (uint32_t)i, "fifo_id_order");
        rtk::near(run, val, (float)i, 1e-5f);
    }

    // 3. Verify it is empty again
    popped_empty = rtk::guarded(run, [&] { return q.pop(out_id, out_val); });
    rtk::expect(run, popped_empty == false, "empty_after_flush");

    // 4. Massive wrap-around test
    // This catches errors in index increments (e.g., h + 0 instead of h + 1)
    // or incorrect masking logic.
    for (int i = 0; i < 1000; ++i) {
        bool pushed = rtk::guarded(run, [&] { return q.push(i, (float)i); });
        rtk::expect(run, pushed, "wrap_push_success");
        
        uint32_t id;
        float val;
        bool popped = rtk::guarded(run, [&] { return q.pop(id, val); });
        rtk::expect(run, popped, "wrap_pop_success");
        rtk::expect(run, id == (uint32_t)i, "wrap_id_match");
        rtk::near(run, val, (float)i, 1e-5f);
    }

    return rtk::report(run);
}
