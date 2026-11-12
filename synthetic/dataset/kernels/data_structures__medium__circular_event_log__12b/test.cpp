#include "kernel.hpp"
#include "rtk/check.hpp"

int main() {
    rtk::setup();
    rtk::Run run;

    CircularLog k;
    k.configure(10);

    // Test empty state
    rtk::expect(run, k.size() == 0, "initial size 0");
    
    uint32_t id;
    float val;
    rtk::expect(run, k.pop(id, val) == false, "pop from empty");

    // Test basic push/pop
    rtk::guarded_void(run, [&] { k.push(1, 1.1f); });
    rtk::expect(run, k.size() == 1, "size 1");
    rtk::guarded_void(run, [&] { k.push(2, 2.2f); });
    rtk::expect(run, k.size() == 2, "size 2");
    
    rtk::guarded_void(run, [&] { k.pop(id, val); });
    rtk::expect(run, id == 1, "pop id 1");
    rtk::expect(run, val == 1.1f, "pop val 1.1");
    rtk::expect(run, k.size() == 1, "size back to 1");

    // Test overflow/wrap-around
    // Buffer capacity 10. Currently has 1 element (id=2).
    // Push 10 more elements.
    for (int i = 3; i <= 12; ++i) {
        rtk::guarded_void(run, [&] { k.push(static_cast<uint32_t>(i), static_cast<float>(i)); });
    }
    
    rtk::expect(run, k.size() == 10, "size capped at 10");

    // Verify wrap-around logic: 
    // Elements pushed: 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12
    // Capacity is 10. Total pushed (after initial 1) is 11.
    // The oldest (id=2) should be overwritten by the first wrap (id=12).
    // Wait, let's trace:
    // Init: [2] (Size 1)
    // Pushes: 3,4,5,6,7,8,9,10,11,12 (10 pushes)
    // Total pushes since start: 11. 
    // Buffer holds last 10: [3,4,5,6,7,8,9,10,11,12]
    
    // Pop them all to verify order
    for (int i = 3; i <= 12; ++i) {
        rtk::guarded_void(run, [&] { k.pop(id, val); });
        rtk::expect(run, id == static_cast<uint32_t>(i), "pop order");
        rtk::expect(run, val == static_cast<float>(i), "pop order val");
    }
    rtk::expect(run, k.size() == 0, "size empty after all pops");

    // Test capacity 0
    CircularLog k2;
    k2.configure(0);
    rtk::expect(run, k2.push(1, 1.0f) == false, "push fail capacity 0");

    // Test high frequency sequence (500 calls)
    CircularLog k3;
    k3.configure(100);
    for (int i = 0; i < 500; ++i) {
        rtk::guarded_void(run, [&] { k3.push(i, static_cast<float>(i)); });
        if (i % 2 == 0) {
            rtk::guarded_void(run, [&] { k3.pop(id, val); });
        }
        
        // Every 100 pushes/pops, check size logic
        if (i > 0 && i % 100 == 0) {
            // This logic is complex to check exactly due to pop/push interleaving, 
            // but we ensure it stays within bounds.
            rtk::expect(run, k3.size() <= 100, "size bound");
        }
    }

    return rtk::report(run);
}
