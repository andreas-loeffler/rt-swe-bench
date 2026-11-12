#include "kernel.hpp"
#include "rtk/check.hpp"
#include <string>
#include <vector>

int main() {
    rtk::setup();
    rtk::Run run;

    const size_t capacity = 64;
    FixedStringBuilder builder;
    builder.configure(capacity);

    // Test 1: Basic appends
    {
        builder.clear();
        rtk::guarded_void(run, [&] { builder.append("Hello"); });
        rtk::guarded_void(run, [&] { builder.append(" World"); });
        rtk::expect(run, builder.view() == "Hello World", "Basic append");
    }

    // Test 2: Saturation and overflow
    {
        builder.clear();
        std::string long_str(100, 'a');
        // Should only append 63 chars (64 - 1 for null)
        size_t count = rtk::guarded(run, [&] { return builder.append(long_str); });
        rtk::expect(run, count == 63, "Overflow count");
        rtk::expect(run, builder.view().size() == 63, "Overflow view size");
        rtk::expect(run, builder.view()[62] == 'a', "Overflow last char");
    }

    // Test 3: Empty appends and clear
    {
        builder.clear();
        rtk::guarded_void(run, [&] { builder.append(""); });
        rtk::expect(run, builder.view().empty(), "Empty append");
        
        builder.append("Data");
        builder.clear();
        rtk::expect(run, builder.view().empty(), "Clear after data");
    }

    // Test 4: Stress test with 500 iterations
    {
        // We use a loop to simulate many real-time updates
        for (int i = 0; i < 500; ++i) {
            builder.clear();
            std::string msg = "ID:" + std::to_string(i % 100);
            // This will stay well within the 64 byte limit
            rtk::guarded_void(run, [&] { builder.append(msg); });
            
            std::string expected = "ID:" + std::to_string(i % 100);
            rtk::expect(run, builder.view() == expected, "Iteration " + std::to_string(i));
        }
    }

    // Test 5: Exact fill
    {
        builder.clear();
        // Capacity is 64. We want to fill 63 chars + 1 null.
        std::string exact(63, 'x');
        rtk::guarded_void(run, [&] { builder.append(exact); });
        rtk::expect(run, builder.view().size() == 63, "Exact fill");
        
        // Next append should return 0
        size_t count = rtk::guarded(run, [&] { return builder.append("!"); });
        rtk::expect(run, count == 0, "Full buffer append");
    }

    // Test 6: Strict Boundary and Off-by-one checks
    {
        builder.clear();
        // Check behavior when capacity is 1 (only room for null terminator)
        FixedStringBuilder small_builder;
        small_builder.configure(1);
        rtk::guarded_void(run, [&] { small_builder.append("a"); });
        rtk::expect(run, small_builder.view().empty(), "Capacity 1 should be empty");
        size_t count_small = rtk::guarded(run, [&] { return small_builder.append("a"); });
        rtk::expect(run, count_small == 0, "Capacity 1 append should return 0");

        // Check behavior when capacity is 2 (room for 1 char + null)
        builder.clear();
        builder.configure(2); // Need to re-configure or assume capacity was 2
        // Wait, we can't re-configure without re-allocating, let's use a new instance
        FixedStringBuilder b2;
        b2.configure(2);
        rtk::guarded_void(run, [&] { b2.append("a"); });
        rtk::expect(run, b2.view() == "a", "Capacity 2 should hold 1 char");
        size_t count_b2 = rtk::guarded(run, [&] { return b2.append("b"); });
        rtk::expect(run, count_b2 == 0, "Capacity 2 full append should return 0");
    }

    return rtk::report(run);
}
