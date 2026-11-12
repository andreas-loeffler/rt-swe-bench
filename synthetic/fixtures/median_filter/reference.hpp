// Reference: ring buffer plus a sorted copy of the window, both sized in
// configure(); each call removes the outgoing sample and inserts the new one.
#pragma once
#include <algorithm>
#include <cstddef>
#include <vector>

class MedianFilter
{
public:
  // Setup, not real-time: may allocate.
  void configure(std::size_t window)
  {
    window_ = window;
    ring_.assign(window, 0.0);
    sorted_.assign(window, 0.0);
    head_ = count_ = 0;
  }

  // Real-time: lower median of the last `window` samples (of all so far before
  // the window is full): element (n - 1) / 2 of the n samples in sorted order.
  double process(double x)
  {
    if (count_ == window_)
    {
      const double old = ring_[head_];
      auto it = std::lower_bound(sorted_.begin(), sorted_.begin() + static_cast<long>(count_), old);
      std::copy(it + 1, sorted_.begin() + static_cast<long>(count_), it);
      --count_;
    }
    auto pos = std::upper_bound(sorted_.begin(), sorted_.begin() + static_cast<long>(count_), x);
    std::copy_backward(pos, sorted_.begin() + static_cast<long>(count_), sorted_.begin() + static_cast<long>(count_) + 1);
    *pos = x;
    ++count_;
    ring_[head_] = x;
    head_ = (head_ + 1) % window_;
    return sorted_[(count_ - 1) / 2];
  }

private:
  std::vector<double> ring_, sorted_;
  std::size_t window_ = 1, head_ = 0, count_ = 0;
};
