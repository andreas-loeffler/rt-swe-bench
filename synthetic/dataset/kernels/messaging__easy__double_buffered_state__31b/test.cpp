#include "kernel.hpp"
#include "rtk/check.hpp"
#include <vector>
#include <numeric>

int main() {
    rtk::setup();
    rtk::Run run;

    DoubleBufferedState k;
    const size_t size = 10;
    k.configure(size);

    std::vector<double> current_write(size);
    std::vector<double> expected_read(size, 0.0);
    std::vector<double> actual_read(size);

    for (int i = 0; i < 1000; ++i) {
        // 1. Write new data
        for (size_t j = 0; j < size; ++j) {
            current_write[j] = static_cast<double>(i) + j;
        }

        rtk::guarded_void(run, [&] {
            k.write(current_write.data());
        });

        // Before swap, reader should still see the previous snapshot
        rtk::guarded_void(run, [&] {
            k.read(actual_read.data());
        });
        for (size_t j = 0; j < size; ++j) {
            rtk::expect(run, rtk::near(actual_read[j], expected_read[j]), "Read pre-swap");
        }

        // 2. Swap
        rtk::guarded_void(run, [&] {
            k.swap();
        });

        // Now, current_write becomes the expected_read
        expected_read = current_write;

        // 3. Verify read after swap
        rtk::guarded_void(run, [&] {
            k.read(actual_read.data());
        });
        for (size_t j = 0; j < size; ++j) {
            rtk::expect(run, rtk::near(actual_read[j], expected_read[j]), "Read post-swap");
        }
    }

    return rtk::report(run);
}
