#include "kernel.hpp" // Testing against reference implementation
#include "rtk/check.hpp"
#include <vector>
#include <numeric>

int main() {
    rtk::setup();
    rtk::Run run;

    PacketBurstAggregator k;
    const size_t BURST_SIZE = 1024;
    k.configure(BURST_SIZE);

    uint8_t out_buf[2048];
    uint8_t dummy_data[2048];
    std::iota(std::begin(dummy_data), std::end(dummy_data), 0);

    // --- Edge Case: Nulls and Zeros ---
    // These specifically catch changes to the boundary checks in reference.hpp
    size_t null_push = rtk::guarded(run, [&] { 
        return k.push(nullptr, 10); 
    });
    rtk::expect(run, null_push == 0, "Push nullptr rejected");

    size_t zero_push = rtk::guarded(run, [&] { 
        return k.push(dummy_data, 0); 
    });
    rtk::expect(run, zero_push == 0, "Push zero length rejected");

    size_t null_pop = rtk::guarded(run, [&] { 
        return k.pop(nullptr, 100); 
    });
    rtk::expect(run, null_pop == 0, "Pop nullptr rejected");

    size_t zero_pop = rtk::guarded(run, [&] { 
        return k.pop(out_buf, 0); 
    });
    rtk::expect(run, zero_pop == 0, "Pop zero capacity rejected");

    // --- Data Integrity Test ---
    uint8_t pattern1[] = { 0xAA, 0xBB, 0xCC };
    uint8_t pattern2[] = { 0x11, 0x22, 0x33 };
    
    rtk::guarded_void(run, [&] { k.push(pattern1, 3); });
    rtk::guarded_void(run, [&] { k.push(pattern2, 3); });
    
    size_t popped_len = rtk::guarded(run, [&] { 
        return k.pop(out_buf, 2048); 
    });
    rtk::expect(run, popped_len == 6, "Total bytes popped");
    rtk::expect(run, out_buf[0] == 0xAA && out_buf[3] == 0x11, "Data content match");

    // --- Fill and pop loop ---
    size_t internal_shadow_size = 0;
    for (int i = 0; i < 500; ++i) {
        size_t chunk_size = (i % 10) + 1;
        
        size_t pushed = rtk::guarded(run, [&] { 
            return k.push(dummy_data + (i % 256), chunk_size); 
        });

        if (pushed == chunk_size) {
            internal_shadow_size += pushed;
        } else {
            size_t popped = rtk::guarded(run, [&] { 
                return k.pop(out_buf, 2048); 
            });
            rtk::expect(run, popped == internal_shadow_size, "Pop before overflow");
            internal_shadow_size = 0; 
            pushed = rtk::guarded(run, [&] { 
                return k.push(dummy_data + (i % 256), chunk_size); 
            });
            internal_shadow_size += pushed;
        }
        rtk::expect(run, pushed == chunk_size, "Push success");

        if (i % 20 == 0) {
            size_t popped = rtk::guarded(run, [&] { 
                return k.pop(out_buf, 2048); 
            });
            rtk::expect(run, popped == internal_shadow_size, "Pop size match");
            internal_shadow_size = 0;
        }
    }

    // --- Overflow and Boundary cases ---
    rtk::guarded_void(run, [&] { k.pop(out_buf, 2048); });
    
    size_t fill_amount = BURST_SIZE - 5;
    rtk::guarded_void(run, [&] { k.push(dummy_data, fill_amount); });
    
    size_t failed_push = rtk::guarded(run, [&] { 
        return k.push(dummy_data, 6); 
    });
    rtk::expect(run, failed_push == 0, "Overflow rejected");

    size_t success_push = rtk::guarded(run, [&] { 
        return k.push(dummy_data, 5); 
    });
    rtk::expect(run, success_push == 5, "Exact fit accepted");

    // --- Partial Pop Logic ---
    rtk::guarded_void(run, [&] { k.pop(out_buf, 2048); }); 
    rtk::guarded_void(run, [&] { k.push(dummy_data, 100); });
    
    size_t partial_pop = rtk::guarded(run, [&] { 
        return k.pop(out_buf, 50); 
    });
    rtk::expect(run, partial_pop == 50, "Partial pop capacity limit");
    
    size_t next_pop = rtk::guarded(run, [&] { 
        return k.pop(out_buf, 2048); 
    });
    rtk::expect(run, next_pop == 0, "Buffer cleared after any pop");

    return rtk::report(run);
}
