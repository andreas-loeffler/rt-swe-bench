#include "kernel.hpp"
#include "rtk/check.hpp"
#include <vector>
#include <string>

int main() {
    rtk::setup();
    rtk::Run run;

    RateDivider k;
    
    // Case 1: Integer division (48000 / 1000 = 48 samples per trigger)
    double m_rate = 48000.0;
    double t_rate = 1000.0;
    k.configure(m_rate, t_rate);

    double expected_phase = 0.0;
    double increment = t_rate / m_rate;

    for (int i = 0; i < 1000; ++i) {
        bool triggered = rtk::guarded(run, [&] { 
            return k.process(); 
        });
        
        expected_phase += increment;
        bool expected_trigger = false;
        if (expected_phase >= 1.0) {
            expected_trigger = true;
            expected_phase -= 1.0;
        }

        rtk::expect(run, triggered == expected_trigger, "IntDiv_Trigger_" + std::to_string(i));
        
        double actual_phase = rtk::guarded(run, [&] { 
            return k.get_phase(); 
        });
        rtk::near(actual_phase, expected_phase);
    }

    // Case 2: Non-integer division (Irregular intervals)
    // 44100 Hz master, 123.45 Hz target
    double m_rate2 = 44100.0;
    double t_rate2 = 123.45;
    k.configure(m_rate2, t_rate2);
    
    expected_phase = 0.0;
    double increment2 = t_rate2 / m_rate2;

    for (int i = 0; i < 1000; ++i) {
        bool triggered = rtk::guarded(run, [&] { 
            return k.process(); 
        });
        
        expected_phase += increment2;
        bool expected_trigger = false;
        if (expected_phase >= 1.0) {
            expected_trigger = true;
            expected_phase -= 1.0;
        }

        rtk::expect(run, triggered == expected_trigger, "FloatDiv_Trigger_" + std::to_string(i));
        
        double actual_phase = rtk::guarded(run, [&] { 
            return k.get_phase(); 
        });
        rtk::near(actual_phase, expected_phase);
    }

    // Case 3: Maximum frequency (target == master)
    k.configure(48000.0, 48000.0);
    for (int i = 0; i < 100; ++i) {
        bool triggered = rtk::guarded(run, [&] { 
            return k.process(); 
        });
        rtk::expect(run, triggered == true, "MaxFreq_Trigger_" + std::to_string(i));
    }

    return rtk::report(run);
}
