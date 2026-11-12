// Reference: fixed-capacity FIFO over a vector sized in configure().
#pragma once
#include <cstddef>
#include <vector>

class RingBuffer
{
public:
  // Setup, not real-time: may allocate.
  void configure(std::size_t capacity)
  {
    buf_.assign(capacity, 0.0);
    head_ = tail_ = size_ = 0;
  }

  // Real-time: append x; false (and no change) when full.
  bool push(double x)
  {
    if (size_ == buf_.size()) {return false;}
    buf_[tail_] = x;
    tail_ = (tail_ + 1) % buf_.size();
    ++size_;
    return true;
  }

  // Real-time: remove the oldest element into out; false when empty.
  bool pop(double & out)
  {
    if (size_ == 0) {return false;}
    out = buf_[head_];
    head_ = (head_ + 1) % buf_.size();
    --size_;
    return true;
  }

  std::size_t size() const { return size_; }

private:
  std::vector<double> buf_;
  std::size_t head_ = 0, tail_ = 0, size_ = 0;
};
