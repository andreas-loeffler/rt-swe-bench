#include "kernel.hpp"
#include "rtk/check.hpp"
#include <vector>

int main() {
    rtk::setup();
    rtk::Run run;

    StateSequencer k;
    
    // Test Case: Simple sequence
    // State 0: 2 cycles, State 1: 1 cycle, State 2: 3 cycles
    std::vector<uint32_t> config = {2, 1, 3};
    k.configure(config);

    // We expect the sequence to be:
    // cycle 1: state 0 (count 1/2)
    // cycle 2: state 0 (count 2/2) -> transition
    // cycle 3: state 1 (count 1/1) -> transition
    // cycle 4: state 2 (count 1/3)
    // cycle 5: state 2 (count 2/3)
    // cycle 6: state 2 (count 3/3) -> transition
    // cycle 7: state 0 ...
    
    // The transition happens AFTER the duration is met. 
    // Based on the reference: 
    // Call 1: cycles=1, dur[0]=2 -> return 0
    // Call 2: cycles=2, dur[0]=2 -> cycles=0, state=1, return 1
    // Call 3: cycles=1, dur[1]=1 -> cycles=0, state=2, return 2
    // Call 4: cycles=1, dur[2]=3 -> return 2
    // Call 5: cycles=2, dur[2]=3 -> return 2
    // Call 6: cycles=3, dur[2]=3 -> cycles=0, state=0, return 0
    
    // Correct expected pattern for {2, 1, 3}: 0, 1, 2, 2, 2, 0, ...
    // Wait, let's re-verify the logic:
    // step() increments counter. If counter >= duration, reset and move.
    // Config {2, 1, 3}:
    // 1. counter 0->1. 1 < 2. return 0.
    // 2. counter 1->2. 2 >= 2. counter=0, state=1. return 1.
    // 3. counter 0->1. 1 >= 1. counter=0, state=2. return 2.
    // 4. counter 0->1. 1 < 3. return 2.
    // 5. counter 1->2. 2 < 3. return 2.
    // 6. counter 2->3. 3 >= 3. counter=0, state=0. return 0.
    
    // Total period = 2+1+3 = 6.
    // Sequence: 0, 1, 2, 2, 2, 0 | 0, 1, 2, 2, 2, 0 ... 
    // No, that's not right. Let's trace:
    // state 0: duration 2. Calls: 1(ret 0), 2(ret 1). 
    // state 1: duration 1. Call: 3(ret 2).
    // state 2: duration 3. Call: 4(ret 2), 5(ret 2), 6(ret 0).
    
    // Actually, the simplest way to test is to mirror the logic in the test loop.
    
    uint32_t expected_state = 0;
    uint32_t expected_cycles = 0;
    
    for (int i = 0; i < 600; ++i) {
        // Calculate expected
        expected_cycles++;
        if (expected_cycles >= config[expected_state]) {
            expected_cycles = 0;
            expected_state = (expected_state + 1) % config.size();
        }
        uint32_t current_expected = expected_state;

        uint32_t result = rtk::guarded(run, [&] { return k.step(); });
        rtk::expect(run, result == current_expected, "Cycle " + std::to_string(i));
    }

    // Test Edge Case: Single state
    StateSequencer k2;
    k2.configure({5});
    for (int i = 0; i < 100; ++i) {
        uint32_t result = rtk::guarded(run, [&] { return k2.step(); });
        rtk::expect(run, result == 0, "Single state test");
    }

    // Test Edge Case: Empty
    StateSequencer k3;
    k3.configure({});
    for (int i = 0; i < 100; ++i) {
        uint32_t result = rtk::guarded(run, [&] { return k3.step(); });
        rtk::expect(run, result == 0, "Empty config test");
    }

    return rtk::report(run);
}
