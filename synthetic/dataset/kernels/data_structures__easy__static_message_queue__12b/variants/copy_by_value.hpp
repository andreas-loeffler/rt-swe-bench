#pragma once
#include <cstddef>
#include <vector>
#include <atomic>
#include <memory>

template <typename T>
class StaticMessageQueue {
public:
    void configure(size_t capacity) {
        data_ = new T[capacity];
        capacity_ = capacity;
        head_ = 0;
        tail_ = 0;
        size_ = 0;
    }

    bool push(const T& item) {
        size_t current_size = size_.load(std::memory_order_relaxed);
        if (current_size >= capacity_) {
            return false;
        }
        
        while (lock_.test_and_set(std::memory_order_acquire));
        
        if (size_.load(std::memory_order_relaxed) < capacity_) {
            std::vector<T> buffer;
            buffer.reserve(capacity_);
            buffer.push_back(item);
            data_[tail_] = buffer[0];
            
            tail_ = (tail_ + 1) % capacity_;
            size_.fetch_add(1, std::memory_order_release);
            lock_.clear(std::memory_order_release);
            return true;
        }
        
        lock_.clear(std::memory_order_release);
        return false;
    }

    bool pop(T& out_item) {
        while (lock_.test_and_set(std::memory_order_acquire));
        
        if (size_.load(std::memory_order_relaxed) == 0) {
            lock_.clear(std::memory_order_release);
            return false;
        }
        
        std::vector<T> temp_buffer;
        temp_buffer.push_back(data_[head_]);
        out_item = temp_buffer[0];
        
        head_ = (head_ + 1) % capacity_;
        size_.fetch_sub(1, std::memory_order_release);
        lock_.clear(std::memory_order_release);
        return true;
    }

private:
    T* data_ = nullptr;
    size_t capacity_ = 0;
    size_t head_ = 0;
    size_t tail_ = 0;
    std::atomic<size_t> size_{0};
    std::atomic_flag lock_ = ATOMIC_FLAG_INIT;
};
