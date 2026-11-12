#pragma once

#include <vector>
#include <cstdint>
#include <algorithm>

/**
 * Helper class to manage a single timer's state and stage transitions.
 */
class TimerState {
    double elapsed_time = 0.0;
    uint32_t id = 0;
    bool active = false;

public:
    void activate(uint32_t timer_id) {
        id = timer_id;
        elapsed_time = 0.0;
        active = true;
    }

    void add_time(double dt) {
        elapsed_time += dt;
    }

    void reset() {
        elapsed_time = 0.0;
    }

    double get_time() const { return elapsed_time; }
    uint32_t get_id() const { return id; }
    bool is_active() const { return active; }
    void set_inactive() { active = false; }
};

class MultiStageTimeoutManager {
    TimerState* timers = nullptr;
    double* thresholds = nullptr;
    size_t num_timers = 0;
    size_t num_thresholds = 0;

public:
    void configure(size_t max_timers, const std::vector<double>& stages) {
        num_timers = max_timers;
        num_thresholds = stages.size();
        
        timers = new TimerState[max_timers];
        thresholds = new double[num_thresholds];
        
        for (size_t i = 0; i < num_thresholds; ++i) {
            thresholds[i] = stages[i];
        }
    }

    ~MultiStageTimeoutManager() {
        delete[] timers;
        delete[] thresholds;
    }

    void reset(uint32_t id) {
        for (size_t i = 0; i < num_timers; ++i) {
            if (timers[i].is_active() && timers[i].get_id() == id) {
                timers[i].reset();
                return;
            }
        }

        for (size_t i = 0; i < num_timers; ++i) {
            if (!timers[i].is_active()) {
                timers[i].activate(id);
                return;
            }
        }
    }

    void update(uint32_t id, double delta_time) {
        for (size_t i = 0; i < num_timers; ++i) {
            if (timers[i].is_active() && timers[i].get_id() == id) {
                timers[i].add_time(delta_time);
                return;
            }
        }
    }

    uint8_t get_stage(uint32_t id) const {
        for (size_t i = 0; i < num_timers; ++i) {
            if (timers[i].is_active() && timers[i].get_id() == id) {
                double t = timers[i].get_time();
                uint8_t stage = 0;
                while (stage < num_thresholds && t >= thresholds[stage]) {
                    stage++;
                }
                return stage;
            }
        }
        return 0;
    }

    bool is_critical(uint32_t id) const {
        return get_stage(id) == num_thresholds;
    }
};
