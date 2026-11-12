# status_line

Implement `StatusLine::format()` in `kernel.hpp`. It returns the text
`id=<id> joint=<joint> pos=<position> vel=<velocity>`, with both doubles in
fixed notation with exactly 6 decimals (as `std::to_chars` with
`std::chars_format::fixed` and precision 6 prints them). The returned view only
needs to stay valid until the next call. `configure(max_length)` is called once
with an upper bound on the line length.

`format()` is called from a real-time control loop for telemetry: it must not
allocate heap memory. `configure()` may.
