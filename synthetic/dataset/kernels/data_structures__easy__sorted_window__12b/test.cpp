#include "kernel.hpp"
#include "rtk/check.hpp"
#include <vector>
#include <algorithm>
#include <numeric>

int main() {
    rtk::setup();
    rtk::Run run;

    const size_t capacity = 10;
    SortedWindow k;
    k.configure(capacity);

    // Reference implementation for verification
    std::vector<double> ref_buffer;
    std::vector<double> ref_history;
    auto get_median = [&](const std::vector<double>& v) {
        size_t n = v.size();
        if (n % 2 == 1) return v[n / 2];
        return (v[n / 2 - 1] + v[n / 2]) / 2.0;
    };

    for (int i = 0; i < 1000; ++i) {
        double val = static_cast<double>(i % 100);
        
        double result = rtk::guarded(run, [&] { 
            return k.process(val); 
        });

        // Update reference
        if (ref_history.size() < capacity) {
            ref_buffer.insert(std::lower_bound(ref_buffer.begin(), ref_buffer.end(), val), val);
            ref_history.push_back(val);
        } else {
            double oldest = ref_history[0];
            ref_history.erase(ref_history.begin());
            ref_history.push_back(val);
            
            auto it = std::lower_bound(ref_buffer.begin(), ref_buffer.end(), oldest);
            if (it != ref_buffer.end() && *it == oldest) ref_buffer.erase(it);
            ref_buffer.insert(std::lower_bound(ref_buffer.begin(), ref_buffer.end(), val), val);
        }

        rtk::near(result, get_median(ref_buffer), 1e-9, "Median check");
    }

    return rtk::report(run);
}
