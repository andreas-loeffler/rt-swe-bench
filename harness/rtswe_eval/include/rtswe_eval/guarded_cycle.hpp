// Copyright 2026 Andreas Loeffler
// SPDX-License-Identifier: Apache-2.0
//
// Runs one callable (one controller update() cycle) with malloc-guard armed on
// the calling thread and returns every allocation it saw, each with a raw
// backtrace. Symbolization happens after the guard is disarmed, so it may
// allocate freely.

#ifndef RTSWE_EVAL__GUARDED_CYCLE_HPP_
#define RTSWE_EVAL__GUARDED_CYCLE_HPP_

#include <dlfcn.h>
#include <execinfo.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <map>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "intrinsic/malloc_guard.h"

namespace rtswe_eval
{

struct Alloc
{
  std::size_t bytes = 0;
  intrinsic::StackTrace trace;
};

struct CycleReport
{
  std::size_t violations = 0;  // all allocations in the cycle
  std::size_t bytes = 0;
  std::vector<Alloc> allocs;   // the first kMaxRecorded of them, with traces
};

namespace detail
{
constexpr std::size_t kMaxRecorded = 64;

struct Recorder
{
  std::array<Alloc, kMaxRecorded> allocs{};
  std::size_t n = 0;
  std::size_t total = 0;
  std::size_t bytes = 0;
};

inline thread_local Recorder rec;

// Runs inside malloc-guard's hook (its reentrancy flag is set), so the
// allocation backtrace() may do on first use is not counted. Fixed storage
// only; nothing here allocates.
inline void on_alloc(std::size_t bytes)
{
  Recorder & r = rec;
  ++r.total;
  r.bytes += bytes;
  if (r.n < kMaxRecorded)
  {
    Alloc & a = r.allocs[r.n++];
    a.bytes = bytes;
    a.trace.num_frames = static_cast<std::size_t>(
      backtrace(a.trace.stack_trace, static_cast<int>(intrinsic::kMaxMallocGuardStackFrames)));
  }
}
}  // namespace detail

// Process-wide hook installation; call once (e.g. in SetUpTestSuite), outside
// any guarded region. Also warms up backtrace() and the thread-local recorder.
inline intrinsic::ScopedMallocGuardHook & install_hooks()
{
  static intrinsic::ScopedMallocGuardHook hook;
  void * warm[4];
  backtrace(warm, 4);
  detail::rec.n = 0;
  return hook;
}

template <typename F>
auto guarded(F && f, CycleReport & report)
{
  intrinsic::ScopedThreadLocalReaction reaction(intrinsic::MallocGuardReaction::kCustomCallback);
  intrinsic::SetThreadLocalMallocGuardCallback(&detail::on_alloc);
  detail::rec.n = 0;
  detail::rec.total = 0;
  detail::rec.bytes = 0;
  struct Collect
  {
    CycleReport & r;
    ~Collect()
    {
      const auto & d = detail::rec;
      r.violations = d.total;
      r.bytes = d.bytes;
      r.allocs.assign(d.allocs.begin(), d.allocs.begin() + static_cast<std::ptrdiff_t>(d.n));
      intrinsic::SetThreadLocalMallocGuardCallback(nullptr);
    }
  };
  Collect collect{report};  // destroyed after the guard below: reads stay unguarded
  intrinsic::MallocGuard guard;
  return std::forward<F>(f)();
}

inline std::string hex(std::uintptr_t v)
{
  std::ostringstream o;
  o << std::hex << v;
  return o.str();
}

inline std::string chomp(std::string s)
{
  while (!s.empty() && (s.back() == '\n' || s.back() == '\r')) {s.pop_back();}
  return s;
}

struct Frame
{
  std::string object;    // shared object or executable
  std::uintptr_t offset = 0;
  std::string function;  // outermost function at this address (inline chain collapsed)
  std::string location;  // file:line as addr2line reports it
};

// One frame through dladdr + addr2line. Cached per address.
inline Frame resolve(void * addr)
{
  static std::map<void *, Frame> cache;
  if (auto it = cache.find(addr); it != cache.end()) {return it->second;}
  Frame fr;
  Dl_info info{};
  if (dladdr(addr, &info) == 0 || info.dli_fname == nullptr)
  {
    fr.function = "??";
    return cache[addr] = fr;
  }
  fr.object = info.dli_fname;
  // Return addresses point after the call; step back one byte for the call line.
  fr.offset = reinterpret_cast<std::uintptr_t>(addr) -
              reinterpret_cast<std::uintptr_t>(info.dli_fbase) - 1;
  std::ostringstream cmd;
  cmd << "addr2line -f -C -i -e '" << fr.object << "' 0x" << std::hex << fr.offset;
  if (FILE * pipe = popen(cmd.str().c_str(), "r"))
  {
    std::array<char, 2048> fn{};
    std::array<char, 2048> loc{};
    // With -i the pairs run from the innermost inlined function outwards; the
    // last pair is the real function and the line in it that made the call.
    while (fgets(fn.data(), fn.size(), pipe) && fgets(loc.data(), loc.size(), pipe))
    {
      fr.function = chomp(fn.data());
      fr.location = chomp(loc.data());
    }
    pclose(pipe);
  }
  if (fr.function.empty() || fr.function == "??")
  {
    fr.function = info.dli_sname ? info.dli_sname : "??";
  }
  return cache[addr] = fr;
}

// Frames that belong to the allocator machinery rather than to the code that
// asked for memory: the recorder in this binary, malloc-guard, libc, libstdc++.
inline bool is_allocator_frame(const Frame & fr)
{
  const auto & o = fr.object;
  return o.find("malloc_guard") != std::string::npos || o.find("libc.so") != std::string::npos ||
         o.find("libstdc++") != std::string::npos ||
         fr.function.find("rtswe_eval::detail::on_alloc") != std::string::npos ||
         fr.function.find("HandleMallocGuardViolation") != std::string::npos;
}

inline std::string basename(const std::string & path)
{
  auto pos = path.rfind('/');
  return pos == std::string::npos ? path : path.substr(pos + 1);
}

// Function name without its parameter list ("ns::Class::f(int) const" -> "ns::Class::f").
inline std::string short_name(const std::string & fn)
{
  std::size_t depth = 0;
  for (std::size_t i = 0; i < fn.size(); ++i)
  {
    if (fn[i] == '<') {++depth;}
    if (fn[i] == '>' && depth > 0) {--depth;}
    if (fn[i] == '(' && depth == 0 && fn.compare(i, 11, "(anonymous ") != 0) {return fn.substr(0, i);}
  }
  return fn;
}

// Library and message code that allocates on behalf of a caller.
inline bool is_helper_frame(const Frame & fr)
{
  const auto & f = fr.function;
  return f.rfind("std::", 0) == 0 || f.rfind("void std::", 0) == 0 ||
         f.rfind("__gnu_cxx::", 0) == 0 || f.find("::msg::") != std::string::npos ||
         f.find("_<std::allocator<void> >") != std::string::npos;
}

// Site key: the first frame above the allocator and the std/message helpers,
// with its source line, e.g. "pkg::C::update (src/c.cpp:163)".
inline std::string site_of(const intrinsic::StackTrace & trace)
{
  std::string fallback;
  bool exception = false;  // allocation made by `throw` (__cxa_allocate_exception)
  std::string via;         // first library without debug info on the way up
  for (std::size_t i = 0; i < trace.num_frames; ++i)
  {
    Frame fr = resolve(trace.stack_trace[i]);
    if (fr.function.find("__cxa_allocate_exception") != std::string::npos) {exception = true;}
    if (is_allocator_frame(fr)) {continue;}
    // Frames in libraries without debug info often symbolize to an unrelated
    // exported name; skip to the first frame with a source line and keep the
    // library as "via <lib>".
    if (fr.location.rfind("??", 0) == 0 && !fr.object.empty())
    {
      if (via.empty()) {via = basename(fr.object);}
      continue;
    }
    std::string loc = fr.location;
    if (auto pos = loc.find("/src/"); pos != std::string::npos) {loc = loc.substr(pos + 5);}
    std::string key = (exception ? "throw in " : "") + short_name(fr.function) + " (" + loc + ")" +
                      (via.empty() ? "" : " via " + via);
    if (fallback.empty()) {fallback = key;}
    if (!is_helper_frame(fr)) {return key;}
  }
  return fallback.empty() ? "??" : fallback;
}

// Full symbolized backtrace, one frame per line (for logs and agent feedback).
inline std::string symbolize(const intrinsic::StackTrace & trace)
{
  std::ostringstream out;
  for (std::size_t i = 0; i < trace.num_frames; ++i)
  {
    Frame fr = resolve(trace.stack_trace[i]);
    out << "  #" << i << " " << fr.function;
    if (!fr.location.empty()) {out << " at " << fr.location;}
    if (!fr.object.empty()) {out << " [" << basename(fr.object) << "+0x" << hex(fr.offset) << "]";}
    out << "\n";
  }
  return out.str();
}

}  // namespace rtswe_eval

#endif  // RTSWE_EVAL__GUARDED_CYCLE_HPP_
