#include "kernel.hpp"
#include "rtk/check.hpp"
#include <vector>

struct Event {
    int id;
    float val;
    bool operator==(const Event& other) const { return id == other.id && val == other.val; }
};

int main() {
    rtk::setup();
    rtk::Run run;

    BoundedEventLog<Event> log;
    const size_t capacity = 100;
    log.configure(capacity);

    // Test 1: Fill the buffer
    for (int i = 0; i < 600; ++i) {
        Event e{i, static_cast<float>(i) * 1.1f};
        rtk::guarded_void(run, [&]() {
            log.push(e);
        });
    }

    // After 600 pushes to a 100-capacity buffer, 
    // it should be full, and the oldest 500 were overwritten.
    // Remaining should be IDs 500 to 599.
    rtk::guarded(run, [&]() {
        return log.size();
    });
    
    // we need to check the value here, so we use a variable
    size_t s = 0;
    rtk::guarded_void(run, [&]() { s = log.size(); });
    rtk::expect(run, s == capacity, "Buffer should be full after wrap-around");

    // Test 2: Pop and verify ordering (FIFO)
    // Expected IDs: 500, 501, ... 599
    for (int i = 500; i < 600; ++i) {
        Event out{0, 0.0f};
        bool popped = false;
        rtk::guarded_void(run, [&]() {
            popped = log.pop(out);
        });
        rtk::expect(run, popped == true, "Should pop event " + std::to_string(i));
        rtk::expect(run, out.id == i, "Popped ID should be " + std::to_string(i));
        rtk::near(run, out.val, static_cast<float>(i) * 1.1f, 1e-5f);
    }

    // Test 3: Empty state
    Event out_empty{0, 0.0f};
    bool popped_empty = true;
    rtk::guarded_void(run, [&]() {
        popped_empty = log.pop(out_empty);
    });
    rtk::expect(run, popped_empty == false, "Pop from empty should return false");

    size_t s_empty = 1;
    rtk::guarded_void(run, [&]() { s_empty = log.size(); });
    rtk::expect(run, s_empty == 0, "Size of empty buffer should be 0");

    // Test 4: Partial fill and pop
    for (int i = 0; i < 50; ++i) {
        Event e{i, 0.0f};
        rtk::guarded_void(run, [&]() { log.push(e); });
    }
    
    size_t s_partial = 0;
    rtk::guarded_void(run, [&]() { s_partial = log.size(); });
    rtk::expect(run, s_partial == 50, "Size should be 50");

    Event out_partial{0, 0.0f};
    bool popped_partial = false;
    rtk::guarded_void(run, [&]() {
        popped_partial = log.pop(out_partial);
    });
    rtk::expect(run, popped_partial == true && out_partial.id == 0, "Partial pop should get ID 0");

    return rtk::report(run);
}
