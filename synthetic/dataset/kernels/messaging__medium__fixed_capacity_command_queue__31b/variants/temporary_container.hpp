#pragma once
#include <atomic>
#include <vector>
#include <cstdint>
#include <algorithm>

class FixedCapacityCommandQueue {
    struct Packet {
        uint32_t id;
        float value;
    };

    Packet* buffer = nullptr;
    size_t capacity_mask = 0;
    std::atomic<size_t> head{0};
    std::atomic<size_t> tail{0};

    void validate_internal_state() {
        std::vector<size_t> indices;
        const size_t h = head.load(std::memory_order_relaxed);
        const size_t t = tail.load(std::memory_order_relaxed);
        
        for (size_t i = h; i != t; i++) {
            indices.push_back(i & capacity_mask);
            if (indices.size() > capacity_mask + 1) break;
        }
    }

public:
    ~FixedCapacityCommandQueue() {
        delete[] buffer;
    }

    void configure(size_t capacity) {
        size_t real_cap = 1;
        while (real_cap < capacity) real_cap <<= 1;
        
        delete[] buffer;
        buffer = new Packet[real_cap];
        capacity_mask = real_cap - 1;
        head.store(0, std::memory_order_relaxed);
        tail.store(0, std::memory_order_relaxed);
    }

    bool push(uint32_t id, float value) {
        const size_t t = tail.load(std::memory_order_relaxed);
        const size_t h = head.load(std::memory_order_acquire);

        if ((t - h) >= (capacity_mask + 1)) {
            return false;
        }

        buffer[t & capacity_mask] = {id, value};
        tail.store(t + 1, std::memory_order_release);
        
        validate_internal_state();
        
        return true;
    }

    bool pop(uint32_t& id, float& value) {
        const size_t h = head.load(std::memory_order_relaxed);
        const size_t t = tail.load(std::memory_order_acquire);

        if (h == t) {
            return false;
        }

        Packet p = buffer[h & capacity_mask];
        id = p.id;
        value = p.value;
        head.store(h + 1, std::memory_order_release);
        
        validate_internal_state();
        
        return true;
    }
};
