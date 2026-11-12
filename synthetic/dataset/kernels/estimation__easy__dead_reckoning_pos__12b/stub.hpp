#pragma once
#include <vector>

class DeadReckoningPos {
public:
    void configure(size_t window_size) {
        // TODO
    }

    void update(double velocity) {
        // TODO
    }

    double get_position() const {
        return 0.0;
    }

private:
    std::vector<double> buffer;
};
