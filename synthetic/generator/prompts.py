"""Prompt templates for synthetic task generation (version in synthetic/generator/config.json, "prompt_version").

The prompts state the format and the rules; they contain no kernel code, so every kernel, test and
variant in the dataset is written by the model.
"""

SYSTEM = (
    "You are an experienced C++ engineer who writes real-time code for robots and audio devices. "
    "You follow the requested output format exactly."
)

RULES = """\
Rules for every kernel:
- One header-only C++20 file, standard library only, `#pragma once`, no `main`. The main class
  is the one named below; helper classes are allowed where the difficulty level asks for them.
- `configure(...)` is called once before any real-time call. It may allocate.
- The real-time methods (for example `process`, `push`, `pop`, `step`, `format`) run on a
  hard real-time path: they must not allocate heap memory, throw, lock or do I/O. Any memory
  they need is reserved in `configure(...)`.
- Difficulty level {level}: {level_about}. At most about {max_lines} lines.

The test harness (header `rtk/check.hpp`, namespace `rtk`) provides:
- `rtk::setup()`: call once at the start of `main`.
- `rtk::Run run;` collects results.
- `rtk::guarded(run, [&] { return k.method(args); })` calls one real-time method under the
  allocation guard and returns its value; `rtk::guarded_void(run, [&] { k.method(args); })`
  for methods without a return value. Every real-time call in the test goes through one of these.
- `rtk::expect(run, condition, "label")` records a functional check.
- `rtk::near(a, b, tol)` compares floating-point values (relative tolerance, default 1e-9).
- `return rtk::report(run);` ends `main`.
"""

FORMAT = """\
Answer with the files only, each wrapped exactly like this:
<file name="NAME">
...contents...
</file>"""

IDEAS = """\
Domain: {domain} ({about}).

List {n} different real-time kernels from this domain that a robot, drone or audio product
could run every cycle, at difficulty level {level}: {level_about}. Each has a main C++ class with
a setup call `configure(...)` and at least one real-time method. Prefer kernels whose natural, naive implementation would allocate
(a growing container, a temporary buffer, a copied message or string), so that writing them
allocation-free takes care.
{taken}
Answer with one JSON object per line and nothing else:
{{"name": "snake_case_name", "class": "CamelCaseName", "summary": "one sentence: what the real-time method computes"}}"""

KERNEL = """\
Write a Tier 1 kernel for a real-time benchmark.

Kernel: {name} (class `{cls}`), domain {domain}, difficulty {level}: {summary}

{rules}
Write four files:
1. `spec.md`: what the class computes, the exact public API (signatures of `configure` and of
   every real-time method), preconditions, and the statement that the real-time methods must not
   allocate. Describe behavior, not the implementation technique.
2. `reference.hpp`: a correct, allocation-free implementation.
3. `stub.hpp`: the same public API; `configure` does nothing and each real-time method returns a
   default value (0, false, an empty result) with a `// TODO` comment.
4. `test.cpp`: `#include "kernel.hpp"` (the implementation under test) and `#include "rtk/check.hpp"`.
   Configure one instance, then make at least 500 real-time calls with deterministic inputs that
   exercise every branch (edge cases, wrap-around, full and empty states). Check every result
   with `rtk::expect` against an expected value that the test computes independently in plain,
   obvious code (it may allocate outside the guarded calls). The test must not create a second
   instance of the class or copy its algorithm.

{format}"""

SECOND_REFERENCE = """\
Implement the class specified below for a real-time benchmark.

{spec}

Start from this header, keep its public API exactly, and replace the TODOs:
```cpp
{stub}
```

{rules}
Write one file, `reference.hpp`.

{format}"""

VARIANT = """\
Below is a specification and a correct, allocation-free implementation of a real-time kernel.
Write a variant that behaves exactly the same for every input but allocates heap memory on the
real-time path (in a real-time method or in anything it calls) when the test runs, using this
pattern:

  {pattern}: {about}

The variant must look like code a developer could plausibly write, not deliberately broken code:
the allocation should come from a natural choice of data structure or API. {hint} Keep the public API,
the class name and the results identical. Do not add comments that point out the allocation.

Specification:
{spec}

Allocation-free implementation:
```cpp
{reference}
```

Write one file, `variant.hpp`.

{format}"""


# Repair rounds (synthetic/generator/config.json, "repair_rounds"): the model that wrote a file gets the gate's
# result back and may fix its own files. The gates then run again on the result.
REPAIR_KERNEL = """\
You wrote the files below for a real-time benchmark kernel. Our automatic checks compiled
`reference.hpp` (as `kernel.hpp`) and `stub.hpp` into `test.cpp`, ran the test and found a problem:

{problem}

{rules}
Fix the problem. Change only what is needed; keep the class name and the public API from
`spec.md`. Answer with every file you changed (`reference.hpp`, `test.cpp`, or both), complete.

`spec.md`:
{spec}

`reference.hpp`:
```cpp
{reference}
```

`test.cpp`:
```cpp
{test}
```

{format}"""

REPAIR_SECOND = """\
You implemented the class specified below for a real-time benchmark. Our automatic check compiled
your `reference.hpp` into the benchmark's test and found a problem:

{problem}

{spec}

Your implementation:
```cpp
{reference}
```

{rules}
Fix it and keep the public API exactly. Write one file, `reference.hpp`, complete.

{format}"""
