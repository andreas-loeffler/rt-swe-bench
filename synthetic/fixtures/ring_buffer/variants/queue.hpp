// Allocating variant: std::queue over std::deque; the deque allocates a new
// block whenever the back crosses a block boundary.
#pragma once
#include <cstddef>
#include <queue>

class RingBuffer
{
public:
  void configure(std::size_t capacity)
  {
    capacity_ = capacity;
    q_ = {};
  }

  bool push(double x)
  {
    if (q_.size() == capacity_) {return false;}
    q_.push(x);
    return true;
  }

  bool pop(double & out)
  {
    if (q_.empty()) {return false;}
    out = q_.front();
    q_.pop();
    return true;
  }

  std::size_t size() const { return q_.size(); }

private:
  std::queue<double> q_;
  std::size_t capacity_ = 0;
};
