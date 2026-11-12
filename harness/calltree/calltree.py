#!/usr/bin/env python3
"""Static allocation call tree for one ros2_controllers package (condition C3).

Compiles the package's library sources with clang to LLVM IR (-O0, line tables
only), builds the call graph over every function defined in those translation
units, including instantiated templates and implicit copy constructors, and
walks it from the controller's real-time entry points:

    update(const rclcpp::Time &, const rclcpp::Duration &)
    update_and_write_commands(...), update_reference_from_subscribers(...)

It reports every call site in the package's own code from which a heap
allocation (operator new, malloc and friends, or logging) is reachable, with
the shortest call chain down to the allocation. std::string internals are
instantiated in the translation unit (-D_GLIBCXX_ASSERTIONS disables libstdc++'s
extern template for basic_string), so string copies show up as well.

The analysis over-approximates: it lists paths that may never run, such as
error branches or growth that only happens on the first cycle. Calls into other
shared libraries and virtual calls are not followed; they are listed separately.

    calltree.py --package diff_drive_controller [--ws /ws] [--json]
"""
from __future__ import annotations

import argparse
import collections
import json
import os
import re
import shlex
import subprocess
import sys
import tempfile
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

ROOT_METHODS = ("update", "update_and_write_commands", "update_reference_from_subscribers")
ROOT_SIG = "(rclcpp::Time const&, rclcpp::Duration const&)"

ALLOC = {
    "_Znwm": "operator new", "_Znam": "operator new[]",
    "_ZnwmSt11align_val_t": "operator new", "_ZnamSt11align_val_t": "operator new[]",
    "_ZnwmRKSt9nothrow_t": "operator new", "_ZnamRKSt9nothrow_t": "operator new[]",
    "_ZnwmSt11align_val_tRKSt9nothrow_t": "operator new",
    "_ZnamSt11align_val_tRKSt9nothrow_t": "operator new[]",
    "malloc": "malloc", "calloc": "calloc", "realloc": "realloc",
    "posix_memalign": "posix_memalign", "aligned_alloc": "aligned_alloc",
    "memalign": "memalign", "valloc": "valloc", "strdup": "strdup", "strndup": "strndup",
}
# Library summaries: external functions known to allocate on every call that reaches them.
SUMMARIES = {
    "rcutils_log": "logging: formats the message and publishes it to /rosout",
    "rcutils_log_internal": "logging: formats the message and publishes it to /rosout",
}
# libstdc++ plumbing left out of displayed paths (kept: the steps that say what is copied or grown)
PLUMBING = re.compile(r"^(std::(_\w+|__\w+|allocator_traits|allocator)\b|__gnu_cxx::)|::(_M_(?!create\b|realloc_|"
                      r"push_back_aux|push_front_aux|reallocate_map|default_append|fill_insert|range_insert)\w*)$")
THROW = re.compile(r"^(__cxa_allocate_exception|_ZSt\d+__throw_\w+)$")

DEFINE = re.compile(r'^define [^@]*@(?:"([^"]+)"|([\w.$-]+))\(.*?(?:!dbg !(\d+))? ?\{$')
# The callee is the first @name( or %reg( after the keyword. Return attributes such as
# dereferenceable(16) contain parentheses, but no @ or %.
CALL = re.compile(r'\b(?:call|invoke)\b.*?(?:(@)(?:"([^"]+)"|([\w.$-]+))|%(?:"[^"]+"|[\w.$-]+))\(')
DBG = re.compile(r"!dbg !(\d+)")
META = re.compile(r"^!(\d+) = (?:distinct )?!(\w+)\((.*)\)$")
FIELD = re.compile(r'(\w+): (?:!(\d+)|"((?:[^"\\]|\\.)*)"|(-?\d+))')


# --------------------------------------------------------------------------- compile

def package_dir(repo: Path, package: str) -> Path:
    for xml in repo.glob("*/package.xml"):
        if re.search(rf"<name>\s*{re.escape(package)}\s*</name>", xml.read_text()):
            return xml.parent
    raise SystemExit(f"error: package {package!r} not found in {repo}")


def compile_commands(ws: Path, base_ws: Path, package: str, pkg_dir: Path) -> list[tuple[str, list[str]]]:
    """(source, clang argv) for the package's library sources, from the Ninja build
    of this workspace or, if the package was not built here yet, of the base workspace."""
    build = ws / "build" / package
    remap = None
    if not (build / "build.ninja").exists():
        build = base_ws / "build" / package
        if ws != base_ws:
            remap = (f"{base_ws}/src/", f"{ws}/src/")
    p = subprocess.run(["ninja", "-C", str(build), "-t", "compdb"], capture_output=True, text=True)
    if p.returncode != 0:
        raise SystemExit(f"error: no build of {package} found ({p.stderr.strip()[:200]})")
    out = []
    for e in json.loads(p.stdout):
        argv = shlex.split(e.get("command", ""))
        src = e.get("file", "")
        if remap:
            src = src.replace(*remap)
            argv = [a.replace(*remap) for a in argv]
        if not src.endswith((".cpp", ".cc", ".cxx")) or "-c" not in argv:
            continue
        if not src.startswith(str(pkg_dir) + "/") or "/test/" in src[len(str(pkg_dir)):]:
            continue
        if argv and argv[0].endswith("ccache"):
            argv = argv[1:]
        args, skip = [], False
        for a in argv[1:]:
            if skip:
                skip = False
            elif a == "-o":
                skip = True
            elif a == "-c" or a.startswith(("-Werror", "-fdiagnostics-color", "-O", "-g")):
                continue
            else:
                args.append(a)
        out.append((src, e["directory"], args))
    if not out:
        raise SystemExit(f"error: no library sources of {package} in its build")
    seen, uniq = set(), []
    for src, d, args in out:
        if src not in seen:
            seen.add(src)
            uniq.append((src, d, args))
    return uniq


def to_ir(job: tuple[str, str, list[str]], work: Path) -> tuple[str, Path | None, str]:
    src, directory, args = job
    ll = work / (re.sub(r"[^\w.]", "_", src) + ".ll")
    cmd = [os.environ.get("RTSWE_CLANG", "clang++-18")] + args + [
        "-S", "-emit-llvm", "-O0", "-gline-tables-only", "-w", "-D_GLIBCXX_ASSERTIONS", "-o", str(ll)]
    p = subprocess.run(cmd, cwd=directory, capture_output=True, text=True)
    if p.returncode != 0:
        errs = [l for l in p.stderr.splitlines() if "error" in l][:12]
        return src, None, "\n".join(errs) or p.stderr[-1500:]
    return src, ll, ""


# --------------------------------------------------------------------------- IR

class Function:
    __slots__ = ("sym", "file", "line", "calls")

    def __init__(self, sym, file, line):
        self.sym, self.file, self.line = sym, file, line
        # (callee or None if indirect, file, line, builds an exception)
        self.calls: list[tuple[str | None, str, int, bool]] = []


def parse_ir(ll: Path) -> dict[str, Function]:
    text = ll.read_text(errors="replace").splitlines()
    meta = {}
    for line in reversed(text):
        if not line.startswith("!"):
            break
        m = META.match(line)
        if m:
            meta[m.group(1)] = (m.group(2), m.group(3))

    fields_cache: dict[str, dict] = {}

    def fields(mid: str) -> dict:
        if mid not in fields_cache:
            kind, body = meta.get(mid, ("", ""))
            d = {"_kind": kind}
            for f in FIELD.finditer(body):
                d[f.group(1)] = f.group(2) or f.group(3) or f.group(4)
            fields_cache[mid] = d
        return fields_cache[mid]

    file_cache: dict[str, str] = {}

    def file_of(scope: str | None) -> str:
        hops = 0
        while scope and hops < 50:
            if scope in file_cache:
                return file_cache[scope]
            f = fields(scope)
            if "file" in f:
                df = fields(f["file"])
                name, d = df.get("filename", ""), df.get("directory", "")
                path = os.path.normpath(name if name.startswith("/") else os.path.join(d, name))
                file_cache[scope] = path
                return path
            scope, hops = f.get("scope"), hops + 1
        return ""

    def loc(mid: str | None) -> tuple[str, int]:
        if not mid:
            return "", 0
        f = fields(mid)
        if f["_kind"] == "DILocation":
            return file_of(f.get("scope")), int(f.get("line") or 0)
        if f["_kind"] == "DISubprogram":
            return file_of(mid), int(f.get("line") or 0)
        return "", 0

    funcs: dict[str, Function] = {}
    cur: Function | None = None
    in_throw = False  # between __cxa_allocate_exception and __cxa_throw: building an exception
    i = 0
    while i < len(text):
        line = text[i]
        if cur is None:
            if line.startswith("define "):
                m = DEFINE.match(line)
                if m:
                    sym = m.group(1) or m.group(2)
                    file, ln = loc(m.group(3))
                    cur = Function(sym, file, ln)
                    in_throw = False
        elif line == "}":
            funcs.setdefault(cur.sym, cur)
            cur = None
        elif " call " in line or " invoke " in line or line.lstrip().startswith(("call ", "invoke ")):
            m = CALL.search(line)
            if m and " asm " not in line[:m.start() + 40]:
                callee = (m.group(2) or m.group(3)) if m.group(1) else None
                if callee is None or not callee.startswith("llvm."):
                    d = DBG.search(line)
                    if not d and " invoke " in line and i + 1 < len(text):
                        d = DBG.search(text[i + 1])
                    file, ln = loc(d.group(1) if d else None)
                    if callee == "__cxa_allocate_exception":
                        in_throw = True
                    cur.calls.append((callee, file, ln, in_throw))
                    if callee == "__cxa_throw":
                        in_throw = False
        i += 1
    return funcs


def demangle(symbols: list[str]) -> dict[str, str]:
    # LLVM's demangler knows clang 18's newer manglings (e.g. "Tn" constraints); binutils' does not
    tool = "llvm-cxxfilt-18" if subprocess.run(["which", "llvm-cxxfilt-18"], capture_output=True).returncode == 0 \
        else "c++filt"
    p = subprocess.run([tool], input="\n".join(symbols), capture_output=True, text=True)
    out = p.stdout.splitlines()
    return dict(zip(symbols, out)) if len(out) == len(symbols) else {s: s for s in symbols}


# --------------------------------------------------------------------------- names

def _match(s: str, i: int, open_: str, close: str) -> int:
    depth = 0
    for j in range(i, len(s)):
        if s[j] == open_:
            depth += 1
        elif s[j] == close:
            depth -= 1
            if depth == 0:
                return j
    return len(s) - 1


def _collapse(s: str, keep_depth: int = 1) -> str:
    """Drop template arguments nested deeper than keep_depth."""
    out, depth = [], 0
    for ch in s:
        if ch == "<":
            depth += 1
            if depth == keep_depth + 1:
                out.append("<…>")
            if depth > keep_depth:
                continue
        elif ch == ">":
            depth -= 1
            if depth >= keep_depth:
                continue
        elif depth > keep_depth:
            continue
        out.append(ch)
    return "".join(out)


def short(name: str) -> str:
    s = name
    while "> >" in s:
        s = s.replace("> >", ">>")
    s = s.replace("std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char>>", "std::string")
    s = s.replace("std::__cxx11::", "std::")
    s = re.sub(r"(\w+)_<std::allocator<void> ?>", r"\1", s)        # ROS message types
    for _ in range(4):                                               # default allocators
        s = re.sub(r", std::allocator<[^<>]*(?:<[^<>]*(?:<[^<>]*>)*[^<>]*>)*[^<>]*>", "", s)
    s = re.sub(r"\(anonymous namespace\)", "(anon)", s)
    # parameters of the function itself: the last top-level (...)
    end = s.rstrip()
    for q in (" const", " &&", " &", " volatile", " noexcept"):
        while end.endswith(q):
            end = end[: -len(q)]
    if end.endswith(")"):
        depth = 0
        for j in range(len(end) - 1, -1, -1):
            if end[j] == ")":
                depth += 1
            elif end[j] == "(":
                depth -= 1
                if depth == 0:
                    end = end[:j] if not end[:j].endswith("operator") else end[:j] + "()"
                    break
    # enclosing function parameters of lambdas and local classes
    while True:
        m = re.search(r"\((?!\))[^()]*(?:\([^()]*\)[^()]*)*\)(?=::)", end)
        if not m:
            break
        end = end[: m.start()] + "()" + end[m.end():]
    # return type of template functions: keep the last top-level token
    depth, cut = 0, 0
    for j, ch in enumerate(end):
        if ch in "<(":
            depth += 1
        elif ch in ">)":
            depth -= 1
        elif ch == " " and depth == 0 and not end[:j].endswith("operator") and end[j + 1: j + 9] != "operator":
            cut = j + 1
    end = end[cut:]
    end = end.replace("$_", "lambda#").replace("{lambda", "{lambda")
    if len(end) > 110:
        end = _collapse(end, 1)
    if len(end) > 110:
        end = _collapse(end, 0)
    return end


def hint(chain: list[str]) -> str:
    j = " ".join(chain)
    if re.search(r"::operator=\((?!.*&&)", j) and re.search(r"_M_allocate_and_copy|::_M_create\(|_M_assign", j):
        return ("assignment: reuses the destination's buffer and allocates only if it is too small, "
                "typically on the first cycle")
    if re.search(r"basic_string<.*?>::(_M_create|_S_allocate)\(", j) and "__new_allocator<char>::allocate" in j:
        return "allocates if the string is longer than 15 characters (libstdc++ small-string buffer)"
    if re.search(r"vector<.*::(_M_realloc_append|_M_realloc_insert|_M_default_append|_M_fill_insert|reserve)", j):
        return "allocates when the vector grows beyond its capacity"
    if "deque" in j and re.search(r"_M_push_(back|front)_aux|_M_reallocate_map|_M_new_elements", j):
        return "allocates when the deque needs a new block (every few hundred bytes of pushes)"
    if re.search(r"_Hashtable|_Rb_tree", j):
        return "map/set insertion allocates a node"
    if "make_shared" in j or "__shared_count" in j:
        return "creates a shared_ptr control block"
    return ""


# --------------------------------------------------------------------------- analysis

def analyze(funcs: dict[str, Function], names: dict[str, str], pkg_dir: str):
    in_pkg = lambda f: f.startswith(pkg_dir + "/")
    callers = collections.defaultdict(set)        # every call edge
    callers_normal = collections.defaultdict(set)  # without exception construction
    for f in funcs.values():
        for c, _, _, thr in f.calls:
            if c is not None:
                callers[c].add(f.sym)
                if not thr:
                    callers_normal[c].add(f.sym)

    def reverse_bfs(sinks: set[str], edges) -> dict[str, str | None]:
        nxt = {s: None for s in sinks}
        q = collections.deque(sorted(sinks))
        while q:
            c = q.popleft()
            for f in sorted(edges.get(c, ())):
                if f not in nxt:
                    nxt[f] = c
                    q.append(f)
        return nxt

    called = set(callers)
    alloc_sinks = {s for s in called if s in ALLOC or s in SUMMARIES}
    throw_sinks = {s for s in called if THROW.match(s)}
    to_alloc = reverse_bfs(alloc_sinks, callers_normal)
    to_throw = reverse_bfs(throw_sinks | alloc_sinks, callers)

    def chain_from(c: str) -> list[str]:
        chain, x = [c], to_alloc[c]
        while x is not None:
            chain.append(x)
            x = to_alloc.get(x)
        return chain

    def branches(c: str, k: int = 3) -> list[list[str]]:
        """Up to k allocation paths from c that differ in their first step."""
        if c in ALLOC or c in SUMMARIES:
            return [[c]]
        out, seen = [], set()
        kids = [(d, thr) for d, _, _, thr in funcs[c].calls] if c in funcs else []
        for d, thr in sorted(kids, key=lambda x: len(chain_from(x[0])) if x[0] in to_alloc else 99):
            if d is None or thr or d in seen or d not in to_alloc:
                continue
            seen.add(d)
            out.append([c] + chain_from(d))
            if len(out) == k:
                break
        return out or [chain_from(c)]

    roots = sorted(s for s, f in funcs.items() if in_pkg(f.file) and any(
        names[s].endswith(f"::{m}{ROOT_SIG}") for m in ROOT_METHODS))
    parent = {r: None for r in roots}
    depth = {r: 0 for r in roots}
    q = collections.deque(roots)
    while q:
        f = q.popleft()
        for c, _, _, _ in funcs[f].calls:
            if c in funcs and c not in parent:
                parent[c] = f
                depth[c] = depth[f] + 1
                q.append(c)
    analyze.depth = depth

    sites, throws, indirect, external = [], [], [], collections.OrderedDict()
    for fsym in parent:
        fn = funcs[fsym]
        for c, file, line, thr in fn.calls:
            if not in_pkg(file):
                continue
            if c is None:
                indirect.append((file, line, fsym))
                continue
            if c in funcs and in_pkg(funcs[c].file):
                continue  # package code: its own call sites are reported
            if c in to_alloc and not thr:
                paths = [b for b in branches(c)
                         if not any(s in funcs and in_pkg(funcs[s].file) for s in b[1:])]
                if paths:  # otherwise the path goes back into package code, reported there
                    sites.append((file, line, fsym, paths))
            elif c in to_throw:
                throws.append((file, line, fsym))
            elif c not in funcs:
                n = names.get(c, c)
                if "::" in n and not n.startswith(("std::", "__gnu_cxx::", "__cxxabiv1::")) \
                        or c.startswith(("rcl", "rmw")):
                    external.setdefault(short(n), (file, line))
    return roots, parent, sites, throws, indirect, external


def tail(name: str, k: int = 2) -> str:
    """The last k components of a qualified name, splitting only on top-level ::."""
    parts, depth, start = [], 0, 0
    for j, ch in enumerate(name):
        if ch in "<(":
            depth += 1
        elif ch in ">)":
            depth -= 1
        elif depth == 0 and name.startswith("::", j):
            parts.append(name[start:j])
            start = j + 2
    parts.append(name[start:])
    return "::".join(p for p in parts[-k:] if p)


def via(fsym: str, parent: dict, names: dict, funcs: dict, pkg_dir: str) -> list[str]:
    chain = []
    while fsym is not None:
        if funcs[fsym].file.startswith(pkg_dir + "/"):
            chain.append(tail(short(names[fsym])))
        fsym = parent[fsym]
    return chain[::-1]


def _lines_by_file(locs, rel, width: int = 0) -> str:
    by = collections.OrderedDict()
    for f, l in sorted(locs, key=lambda x: (rel(x[0]), x[1])):
        by.setdefault(rel(f), []).append(str(l))
    text = "; ".join(f"{f}:{','.join(dict.fromkeys(ls))}" for f, ls in by.items())
    return text if not width or len(text) <= width else text[: width - 4] + " ..."


def render(package, repo, pkg_dir, n_tu, roots, parent, sites, throws, indirect, external, names, funcs,
           limit=30, budget=3900, width=260, function: str | None = None) -> str:
    rel = lambda p: os.path.relpath(p, repo)
    depth = getattr(analyze, "depth", {})
    root_names = sorted({short(names[r]).split("::")[-1] + "()" for r in roots})
    out = [f"Static call tree of {package}: {n_tu} source file(s) compiled, entry points "
           f"{', '.join(root_names) or '(none found)'}."]
    if not roots:
        out.append("No update(), update_and_write_commands() or update_reference_from_subscribers() "
                   "defined in this package.")
        return "\n".join(out)
    logging = [(f, l) for f, l, _, paths in sites if all(p[-1] in SUMMARIES for p in paths)]
    code = [s for s in sites if not all(p[-1] in SUMMARIES for p in s[3])]
    if function:
        keep = lambda fsym: any(function in v for v in via(fsym, parent, names, funcs, pkg_dir))
        code = [s for s in code if keep(s[2])]
        logging = [(f, l) for f, l, fsym, paths in sites if (f, l) in set(logging) and keep(fsym)]
        out.append(f"Only call sites in functions matching {function!r} and the package functions they call.")
    if not code:
        out.append("\nNo call site in this package reaches a heap allocation through code "
                   "(see logging and the calls not followed below).")
    else:
        grouped = collections.OrderedDict()
        for file, line, fsym, paths in sorted(code, key=lambda s: (depth.get(s[2], 0), rel(s[0]), s[1])):
            grouped.setdefault((file, line, fsym), []).extend(paths)
        by_fn = collections.OrderedDict()
        for (file, line, fsym) in grouped:
            by_fn.setdefault(fsym, []).append((file, line))
        out.append(f"\n{len(grouped)} call site(s) in {len(by_fn)} function(s) of this package can reach "
                   f"a heap allocation.")
        if len(by_fn) > 1:
            out.append("Functions (closest to the entry points first):")
            for fsym, locs in by_fn.items():
                out.append(f"  {' -> '.join(via(fsym, parent, names, funcs, pkg_dir))}: "
                           f"{_lines_by_file(locs, rel, 160)}")
            out.append("To see only one function and what it calls, pass its name as \"function\".")
        out.append("\nDetails:")
        for k, ((file, line, fsym), chains) in enumerate(grouped.items()):
            if k == limit or sum(len(x) + 1 for x in out) > budget:
                out.append(f"... {len(grouped) - k} more call sites not shown")
                break
            out.append(f"{rel(file)}:{line}  in {' -> '.join(via(fsym, parent, names, funcs, pkg_dir))}")
            by_callee = collections.OrderedDict()
            for chain in chains:
                by_callee.setdefault(chain[0], []).append(chain)
            for callee, cs in list(by_callee.items())[:3]:
                out.append(f"  calls {short(names.get(callee, callee))}")
                for chain in cs[:2]:
                    full = [short(names.get(s, s)) for s in chain[1:-1]]
                    shown = [n for n in full if not PLUMBING.search(n)]
                    sink = chain[-1]
                    if sink in SUMMARIES:
                        shown.append(names.get(sink, sink))
                    else:
                        elem = re.search(r"__new_allocator<(.+)>::allocate$", full[-1]) if full else None
                        shown.append(ALLOC[sink] + (f" ({elem.group(1)} storage)" if elem else ""))
                    if len(shown) > 6:
                        shown = shown[:3] + ["…"] + shown[-2:]
                    h = SUMMARIES.get(sink) or hint([names.get(s, s) for s in chain])
                    text = " -> ".join(shown)
                    if len(text) > width:
                        text = text[: width - 1] + "…"
                    out.append("    -> " + text + (f"\n       [{h}]" if h else ""))
    if logging:
        out.append(f"\nLogging (RCLCPP_* macros) allocates when it runs, because it formats the message "
                   f"and publishes it to /rosout: {_lines_by_file(set(logging), rel, 300)}")
    site_locs = {(f, l) for f, l, _, _ in sites}
    throws = {(f, l) for f, l, _ in throws} - site_locs
    if throws:
        out.append(f"\n{len(throws)} more call site(s) allocate only when they throw an exception "
                   f"(error paths): {_lines_by_file(throws, rel, 200)}")
    if indirect:
        locs = sorted({f"{rel(f)}:{l}" for f, l, _ in indirect})
        out.append(f"\nNot followed, virtual or indirect calls: {', '.join(locs[:8])}"
                   f"{' ...' if len(locs) > 8 else ''}")
    if external:
        items = [f"{n} ({rel(f)}:{l})" for n, (f, l) in list(external.items())[:6]]
        out.append("\nNot followed, calls into other libraries:\n  " + "\n  ".join(items)
                   + (f"\n  ... {len(external) - 6} more" if len(external) > 6 else ""))
    out.append("\nThis is static analysis: it lists every path that can allocate, including "
               "branches that may never run in practice.")
    return "\n".join(out)


def main() -> None:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--package", required=True)
    ap.add_argument("--ws", default=os.environ.get("RTSWE_WS", "/ws"))
    ap.add_argument("--base-ws", default="/ws", help="workspace whose build to use if --ws has none")
    ap.add_argument("--json", action="store_true", help="machine-readable sites (validation)")
    ap.add_argument("--limit", type=int, default=30)
    ap.add_argument("--function", help="only call sites in functions matching this name and below")
    a = ap.parse_args()

    ws, base_ws = Path(a.ws), Path(a.base_ws)
    repo = ws / "src/ros2_controllers"
    pkg_dir = package_dir(repo, a.package)
    jobs = compile_commands(ws, base_ws, a.package, pkg_dir)
    with tempfile.TemporaryDirectory(prefix="rtswe_ct_") as tmp:
        with ThreadPoolExecutor(max_workers=min(4, len(jobs))) as ex:
            results = list(ex.map(lambda j: to_ir(j, Path(tmp)), jobs))
        errors = [(s, e) for s, ll, e in results if ll is None]
        if errors:
            print("The package does not compile, so no call tree:")
            for s, e in errors:
                print(f"{os.path.relpath(s, repo)}:\n{e}")
            sys.exit(1)
        funcs: dict[str, Function] = {}
        for _, ll, _ in results:
            for sym, f in parse_ir(ll).items():
                funcs.setdefault(sym, f)
    syms = sorted(set(funcs) | {c for f in funcs.values() for c, _, _, _ in f.calls if c})
    names = demangle(syms)
    roots, parent, sites, throws, indirect, external = analyze(funcs, names, str(pkg_dir))
    if a.json:
        # Every package line on an allocation path: the sites, and the calls in package code
        # into package functions from which a site is reachable.
        in_pkg = lambda f: f.startswith(str(pkg_dir) + "/")
        hot = {fsym for _, _, fsym, p in sites if not all(c[-1] in SUMMARIES for c in p)}
        grew = True
        while grew:
            grew = False
            for fsym in parent:
                if fsym not in hot and any(c in hot for c, *_ in funcs[fsym].calls):
                    hot.add(fsym)
                    grew = True
        lines = {(os.path.relpath(f, repo), l) for f, l, _, p in sites
                 if not all(c[-1] in SUMMARIES for c in p)}
        lines |= {(os.path.relpath(fl, repo), ln) for fsym in parent for c, fl, ln, thr in funcs[fsym].calls
                  if c in hot and c != fsym and in_pkg(fl) and in_pkg(funcs[c].file) and not thr}
        print(json.dumps({
            "lines": sorted(f"{f}:{l}" for f, l in lines),
            "logging": sorted({f"{os.path.relpath(f, repo)}:{l}" for f, l, _, p in sites
                               if all(c[-1] in SUMMARIES for c in p)}),
            "package": a.package, "roots": [names[r] for r in roots], "functions": len(funcs),
            "reachable": len(parent),
            "sites": [{"file": os.path.relpath(f, repo), "line": l, "in": short(names[s]),
                       "callee": short(names.get(p[0][0], p[0][0])),
                       "paths": [[short(names.get(x, x)) for x in c] for c in p]}
                      for f, l, s, p in sites],
            "throw_sites": len({(f, l) for f, l, _ in throws}),
            "indirect": len({(f, l) for f, l, _ in indirect}), "external": list(external),
        }, indent=1))
        return
    print(render(a.package, repo, str(pkg_dir), len(jobs), roots, parent, sites, throws, indirect,
                 external, names, funcs, a.limit, function=a.function))


if __name__ == "__main__":
    main()
