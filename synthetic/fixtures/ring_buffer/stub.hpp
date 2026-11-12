#pragma once
#include <cstddef>

class RingBuffer
{
public:
  // Setup, not real-time: may allocate.
  void configure(std::size_t capacity) { (void)capacity; }

  // Real-time: append x; false (and no change) when full.
  bool push(double x) { (void)x; return false; }  // TODO

  // Real-time: remove the oldest element into out; false when empty.
  bool pop(double & out) { (void)out; return false; }  // TODO

  std::size_t size() const { return 0; }
};
