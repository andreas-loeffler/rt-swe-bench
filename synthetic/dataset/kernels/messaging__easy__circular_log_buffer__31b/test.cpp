#include "kernel.hpp"
#include "rtk/check.hpp"
#include <vector>
#include <string>
#include <cstdio>
#include <cstring>

int main() {
    rtk::setup();
    rtk::Run run;

    CircularLogBuffer k;
    const size_t MAX_E = 10;
    const size_t MAX_S = 16;
    k.configure(MAX_E, MAX_S);

    // Test 1: Fill and wrap-around
    // We push exactly MAX_E + 5 to ensure we wrap around.
    for (int i = 0; i < (int)MAX_E + 5; ++i) {
        double ts = static_cast<double>(i);
        char msg[32];
        std::snprintf(msg, sizeof(msg), "Msg %d", i);

        rtk::guarded_void(run, [&] {
            k.push(ts, msg);
        });
    }

    // After MAX_E + 5 pushes, size should be exactly MAX_E
    rtk::expect(run, k.size() == MAX_E, "Size should be capped at MAX_E");

    // Check every single element in the buffer for correctness
    // This catches errors in indexing, wrapping, and offset calculation.
    for (size_t i = 0; i < MAX_E; ++i) {
        double read_ts;
        char read_msg[MAX_S + 1];
        bool success = rtk::guarded(run, [&] {
            return k.read(i, read_ts, read_msg, sizeof(read_msg));
        });

        // Expected: oldest is index 5, newest is index 14 (if MAX_E=10, pushed 0..14)
        double expected_ts = static_cast<double>(5 + i);
        char expected_msg[MAX_S + 1];
        std::snprintf(expected_msg, sizeof(expected_msg), "Msg %d", (int)expected_ts);

        rtk::expect(run, success == true, "Read success " + std::to_string(i));
        rtk::near(run, read_ts, expected_ts, 1e-9);
        rtk::expect(run, std::strcmp(read_msg, expected_msg) == 0, "Content match " + std::to_string(i));
    }

    // Test 2: Precise Truncation and Boundary
    // Push a string that is exactly MAX_S - 1 characters long.
    char exact_msg[MAX_S + 1];
    std::memset(exact_msg, 'A', MAX_S - 1);
    exact_msg[MAX_S - 1] = '\0';
    
    rtk::guarded_void(run, [&] {
        k.push(100.0, exact_msg);
    });

    double ts_final;
    char msg_final[MAX_S + 1];
    rtk::guarded_void(run, [&] {
        k.read(k.size() - 1, ts_final, msg_final, sizeof(msg_final));
    });
    rtk::expect(run, std::strcmp(msg_final, exact_msg) == 0, "Exact length string match");

    // Push a string longer than MAX_S - 1
    const char* long_msg = "12345678901234567890"; // 20 chars > 15
    rtk::guarded_void(run, [&] {
        k.push(200.0, long_msg);
    });

    char msg_trunc[MAX_S + 1];
    rtk::guarded_void(run, [&] {
        k.read(k.size() - 1, ts_final, msg_trunc, sizeof(msg_trunc));
    });
    // Should be truncated to MAX_S - 1 (15 chars)
    rtk::expect(run, std::strlen(msg_trunc) == MAX_S - 1, "Truncation length check");
    rtk::expect(run, std::strncmp(msg_trunc, long_msg, MAX_S - 1) == 0, "Truncation content check");

    // Test 3: Strict OOB and boundary checks
    double dummy_ts;
    char dummy_msg[MAX_S + 1];
    
    bool res_oob = rtk::guarded(run, [&] {
        return k.read(MAX_E, dummy_ts, dummy_msg, sizeof(dummy_msg));
    });
    rtk::expect(run, res_oob == false, "Read at index == size() must fail");

    bool res_valid = rtk::guarded(run, [&] {
        return k.read(MAX_E - 1, dummy_ts, dummy_msg, sizeof(dummy_msg));
    });
    rtk::expect(run, res_valid == true, "Read at index == size()-1 must succeed");

    return rtk::report(run);
}
