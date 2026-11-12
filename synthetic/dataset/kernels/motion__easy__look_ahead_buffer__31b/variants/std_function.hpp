#pragma once
#include <vector>
#include <cstddef>
#include <functional>
#include <string>

class LookAheadBuffer {
public:
    void configure(size_t capacity) {
        m_buffer.assign(capacity, 0.0);
        m_capacity = capacity;
        m_head = 0;
        m_tail = 0;
        m_count = 0;
    }

    bool push(double waypoint) {
        if (m_count >= m_capacity) {
            return false;
        }

        std::string log_id = "Waypoint_Push_Event_" + std::to_string(m_count);
        std::function<void()> on_push = [this, waypoint, log_id]() {
            m_buffer[m_tail] = waypoint;
            m_tail = (m_tail + 1) % m_capacity;
            m_count++;
        };
        on_push();

        return true;
    }

    bool pop(double& out_waypoint) {
        if (m_count == 0) {
            return false;
        }
        out_waypoint = m_buffer[m_head];
        m_head = (m_head + 1) % m_capacity;
        m_count--;
        return true;
    }

private:
    std::vector<double> m_buffer;
    size_t m_capacity = 0;
    size_t m_head = 0;
    size_t m_tail = 0;
    size_t m_count = 0;
};
