#include "kernel.hpp"
#include "rtk/check.hpp"
#include <vector>
#include <cstring>
#include <string>

int main() {
    rtk::setup();
    rtk::Run run;
    ZeroCopyPacketSerializer k;

    const size_t BUF_SIZE = 64;
    k.configure(BUF_SIZE);

    // We will run 600 iterations to exercise the buffer and reset logic
    for (int i = 0; i < 600; ++i) {
        // Every 10 iterations, we reset to simulate packet frames
        if (i % 10 == 0) {
            rtk::guarded_void(run, [&] { k.reset(); });
            
            // Check reset state
            size_t bytes = rtk::guarded(run, [&] { return k.get_written_bytes(); });
            rtk::expect(run, bytes == 0, "Reset check " + std::to_string(i));
        }

        // Try to write a float
        float val_f = static_cast<float>(i) * 1.1f;
        bool ok_f = rtk::guarded(run, [&] { return k.serialize_float(val_f); });

        // Try to write a uint32
        uint32_t val_u = static_cast<uint32_t>(i) + 100;
        bool ok_u = rtk::guarded(run, [&] { return k.serialize_uint32(val_u); });

        size_t written = rtk::guarded(run, [&] { return k.get_written_bytes(); });
        const uint8_t* ptr = rtk::guarded(run, [&] { return k.data(); });

        // Calculate expected behavior independently
        static size_t expected_offset = 0;
        if (i % 10 == 0) expected_offset = 0;

        bool expected_ok_f = false;
        if (expected_offset + sizeof(float) <= BUF_SIZE) {
            expected_ok_f = true;
            expected_offset += sizeof(float);
        }

        bool expected_ok_u = false;
        if (expected_offset + sizeof(uint32_t) <= BUF_SIZE) {
            expected_ok_u = true;
            expected_offset += sizeof(uint32_t);
        }

        rtk::expect(run, ok_f == expected_ok_f, "Float bound check " + std::to_string(i));
        rtk::expect(run, ok_u == expected_ok_u, "Uint bound check " + std::to_string(i));
        rtk::expect(run, written == expected_offset, "Offset check " + std::to_string(i));

        // Verify data integrity for the first successful write of the packet
        if (i % 10 == 0 && ok_f) {
            float actual_f;
            std::memcpy(&actual_f, ptr, sizeof(float));
            rtk::near(run, actual_f, val_f, 1e-7);
        }
    }

    return rtk::report(run);
}
