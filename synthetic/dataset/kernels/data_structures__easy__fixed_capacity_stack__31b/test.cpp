#include "kernel.hpp" // Using reference as the kernel for testing
#include "rtk/check.hpp"
#include <vector>

int main() {
    rtk::setup();
    rtk::Run run;

    FixedCapacityStack stack;
    const size_t capacity = 100;
    stack.configure(capacity);

    // Test sequence: Fill to capacity, then empty, repeated multiple times
    // to ensure 500+ real-time calls.
    std::vector<int> shadow_stack;
    
    for (int iteration = 0; iteration < 5; ++iteration) {
        // 1. Fill to capacity
        for (int i = 0; i < (int)capacity; ++i) {
            int val = iteration * 1000 + i;
            bool result = rtk::guarded(run, [&] { return stack.push(val); });
            rtk::expect(run, result == true, "Push success within capacity");
            shadow_stack.push_back(val);
        }

        // 2. Test overflow
        bool overflow = rtk::guarded(run, [&] { return stack.push(999); });
        rtk::expect(run, overflow == false, "Push fail when full");

        // 3. Check size
        size_t s = rtk::guarded(run, [&] { return stack.size(); });
        rtk::expect(run, s == capacity, "Size matches capacity");

        // 4. Empty the stack
        for (int i = 0; i < (int)capacity; ++i) {
            int popped_val = 0;
            bool result = rtk::guarded(run, [&] { 
                bool r = stack.pop(popped_val); 
                return r; 
            });
            
            int expected = shadow_stack.back();
            shadow_stack.pop_back();

            rtk::expect(run, result == true, "Pop success");
            rtk::expect(run, popped_val == expected, "Popped value matches LIFO");
        }

        // 5. Test underflow
        int underflow_val = 0;
        bool underflow = rtk::guarded(run, [&] { 
            return stack.pop(underflow_val); 
        });
        rtk::expect(run, underflow == false, "Pop fail when empty");

        // 6. Final size check
        size_t s_empty = rtk::guarded(run, [&] { return stack.size(); });
        rtk::expect(run, s_empty == 0, "Size is zero");
    }

    return rtk::report(run);
}
