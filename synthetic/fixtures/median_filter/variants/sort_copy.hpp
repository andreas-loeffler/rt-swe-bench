// Allocating variant: copies the window and partially sorts the copy per call.
#pragma once
#include <algorithm>
#include <cstddef>
#include <vector>

class MedianFilter
{
public:
  void configure(std::size_t window)
  {
    window_ = window;
    ring_.assign(window, 0.0);
    head_ = count_ = 0;
  }

  double process(double x)
  {
    ring_[head_] = x;
    head_ = (head_ + 1) % window_;
    if (count_ < window_) {++count_;}
    std::vector<double> w;
    if (count_ < window_) {w.assign(ring_.begin(), ring_.begin() + static_cast<long>(count_));}
    else {w = ring_;}
    auto mid = w.begin() + static_cast<long>((w.size() - 1) / 2);
    std::nth_element(w.begin(), mid, w.end());
    return *mid;
  }

private:
  std::vector<double> ring_;
  std::size_t window_ = 1, head_ = 0, count_ = 0;
};
