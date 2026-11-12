#include "kernel.hpp"
#include "rtk/check.hpp"
#include <vector>

int main() {
    rtk::setup();
    rtk::Run run;

    LookAheadBuffer k;
    const size_t capacity = 10;
    k.configure(capacity);

    // Corrected loop for precise tracking
    std::vector<double> expected_queue;
    k.configure(capacity); // Reset

    for (int i = 0; i < 1000; ++i) {
        // Logic: Push until full, then pop one, then push one.
        if (expected_queue.size() < capacity) {
            double val = (double)i;
            bool ok = rtk::guarded(run, [&] { return k.push(val); });
            rtk::expect(run, ok == true, "Push success");
            expected_queue.push_back(val);
        } else {
            // Buffer is full, push should fail
            bool ok = rtk::guarded(run, [&] { return k.push(123.45); });
            rtk::expect(run, ok == false, "Push overflow");
        }

        // Pop every 3rd iteration
        if (i % 3 == 0 && !expected_queue.empty()) {
            double internal_out = 0.0;
            bool res = rtk::guarded(run, [&] { 
                return k.pop(internal_out); 
            });
            
            double expected_val = expected_queue[0];
            rtk::expect(run, res == true, "Pop success");
            rtk::near(run, internal_out, expected_val, 1e-9);
            expected_queue.erase(expected_queue.begin());
        }
    }

    // Final drain
    while (!expected_queue.empty()) {
        double internal_out = 0.0;
        bool res = rtk::guarded(run, [&] { 
            return k.pop(internal_out); 
        });
        rtk::expect(run, res == true, "Drain success");
        rtk::near(run, internal_out, expected_queue[0], 1e-9);
        expected_queue.erase(expected_queue.begin());
    }

    // Ensure finally empty
    double final_dummy;
    bool final_res = rtk::guarded(run, [&] { return k.pop(final_dummy); });
    rtk::expect(run, final_res == false, "Final empty state");

    return rtk::report(run);
}
