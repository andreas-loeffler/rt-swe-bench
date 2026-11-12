#pragma once
#include <cstddef>
#include <cstdint>
#include <string_view>

class StatusLine
{
public:
  // Setup, not real-time: may allocate.
  void configure(std::size_t max_length) { (void)max_length; }

  // Real-time: "id=<id> joint=<joint> pos=<position> vel=<velocity>", numbers in
  // fixed notation with 6 decimals. The view stays valid until the next call.
  std::string_view format(std::uint32_t id, std::string_view joint, double position, double velocity)
  {
    (void)id; (void)joint; (void)position; (void)velocity;
    return {};  // TODO
  }
};
