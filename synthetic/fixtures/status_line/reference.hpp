// Reference: formats into a char buffer sized in configure(), with std::to_chars.
#pragma once
#include <charconv>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string_view>
#include <vector>

class StatusLine
{
public:
  // Setup, not real-time: may allocate.
  void configure(std::size_t max_length) { buf_.assign(max_length, '\0'); }

  // Real-time: "id=<id> joint=<joint> pos=<position> vel=<velocity>", numbers in
  // fixed notation with 6 decimals. The view stays valid until the next call.
  std::string_view format(std::uint32_t id, std::string_view joint, double position, double velocity)
  {
    char * p = buf_.data();
    char * end = buf_.data() + buf_.size();
    auto put = [&](std::string_view s) {
      const std::size_t n = std::min<std::size_t>(s.size(), static_cast<std::size_t>(end - p));
      std::memcpy(p, s.data(), n);
      p += n;
    };
    put("id=");
    p = std::to_chars(p, end, id).ptr;
    put(" joint=");
    put(joint);
    put(" pos=");
    p = std::to_chars(p, end, position, std::chars_format::fixed, 6).ptr;
    put(" vel=");
    p = std::to_chars(p, end, velocity, std::chars_format::fixed, 6).ptr;
    return {buf_.data(), static_cast<std::size_t>(p - buf_.data())};
  }

private:
  std::vector<char> buf_;
};
