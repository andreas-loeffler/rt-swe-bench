"""Deterministic first-order mutants of a C++ header, to check that a test is strong enough (gate G2).

Operators: relational (<, <=, >, >=, ==, !=), arithmetic (+, -, *, /), increment and decrement,
logical (&&, ||), boolean literals and numeric literals. Only the bodies of the real-time methods
are mutated: not configure(), constructors or member declarations, whose mutants are mostly
equivalent (a default value that configure() overwrites). Comments, string and character
literals, preprocessor lines and template argument brackets are left alone. Mutants that do not
compile are discarded by the caller.
"""
import random
import re

SWAP = {"<=": "<", "<": "<=", ">=": ">", ">": ">=", "==": "!=", "!=": "==",
        "+": "-", "-": "+", "*": "/", "/": "*", "++": "--", "--": "++",
        "&&": "||", "||": "&&", "true": "false", "false": "true",
        "+=": "-=", "-=": "+=", "*=": "/=", "/=": "*="}
NUM = r"(?<![\w.])(?:\d+\.\d*|\.\d+|\d+)(?:[eE][+-]?\d+)?[fFuUlL]*(?![\w.])"
TOKEN = re.compile(r"\+=|-=|\*=|/=|\+\+|--|&&|\|\||<=|>=|==|!=|(?<![<>=!+\-*/&|])[<>+\-*/](?![<>=+\-*/&|])|\btrue\b|\bfalse\b|" + NUM)
SETUP = {"configure"}  # setup methods whose bodies are not mutated (constructors are added per class)


def _code_mask(src: str) -> list[bool]:
    """True for characters outside comments, string/char literals and preprocessor lines."""
    mask = [True] * len(src)
    i, n = 0, len(src)
    line_start = True
    while i < n:
        c = src[i]
        if line_start and src[i:].lstrip(" \t").startswith("#"):
            j = src.find("\n", i)
            j = n if j < 0 else j
            for k in range(i, j):
                mask[k] = False
            i = j
            continue
        line_start = c == "\n"
        if src.startswith("//", i):
            j = src.find("\n", i)
            j = n if j < 0 else j
            for k in range(i, j):
                mask[k] = False
            i = j
        elif src.startswith("/*", i):
            j = src.find("*/", i + 2)
            j = n if j < 0 else j + 2
            for k in range(i, j):
                mask[k] = False
            i = j
        elif c in "\"'":
            j = i + 1
            while j < n and src[j] != c:
                j += 2 if src[j] == "\\" else 1
            for k in range(i, min(j + 1, n)):
                mask[k] = False
            i = j + 1
        else:
            i += 1
    return mask


def _is_template_bracket(src: str, pos: int, tok: str) -> bool:
    if tok == "<":
        return bool(re.search(r"[A-Za-z_:]\s*$", src[max(0, pos - 40):pos])) and \
            bool(re.match(r"\s*[A-Za-z_:]", src[pos + 1:pos + 40]))
    if tok == ">":
        return bool(re.search(r"[A-Za-z_:>\d]\s*$", src[max(0, pos - 40):pos])) and \
            bool(re.match(r"\s*[\s;,>(){&*:]", src[pos + 1:pos + 3]))
    return False


def _rt_bodies(src: str, mask: list[bool]) -> list[bool]:
    """True for characters inside the body of a member function other than setup and constructors."""
    classes = set(re.findall(r"\b(?:class|struct)\s+([A-Za-z_]\w*)", src))
    inside = [False] * len(src)
    depth, cls_depth, body_start, body_depth, rt = 0, None, None, None, False
    for i, c in enumerate(src):
        if not mask[i]:
            continue
        if c == "{":
            depth += 1
            head = src[max(0, i - 300):i]
            if cls_depth is None and re.search(r"\b(class|struct)\s+\w+[^;{()]*$", head):
                cls_depth = depth
            elif cls_depth is not None and depth == cls_depth + 1 and body_start is None and ")" in head[-200:]:
                m = re.search(r"([~A-Za-z_]\w*)\s*\([^()]*(?:\([^()]*\)[^()]*)*\)[^;{}()]*$", head)
                name = m.group(1).lstrip("~") if m else ""
                rt = bool(m) and name not in SETUP and name not in classes
                body_start, body_depth = i, depth
        elif c == "}":
            if body_start is not None and depth == body_depth:
                if rt:
                    for k in range(body_start, i):
                        inside[k] = True
                body_start = body_depth = None
            if cls_depth is not None and depth == cls_depth:
                cls_depth = None
            depth -= 1
    return inside


def _num_mutation(tok: str) -> list[str]:
    m = re.match(r"(\d+\.\d*|\.\d+|\d+)(.*)", tok)
    if not m:
        return []
    num, suffix = m.groups()
    if "." in num:
        v = float(num)
        return [repr(v + 1.0) + suffix] if v == 0 else [repr(v + 1.0) + suffix, repr(-v) + suffix]
    v = int(num)
    return [str(v + 1) + suffix] + ([str(v - 1) + suffix] if v else [])


def sites(src: str) -> list[tuple[int, str, str]]:
    """All mutation sites as (position, original token, replacement)."""
    mask = _code_mask(src)
    body = _rt_bodies(src, mask)
    out = []
    for m in TOKEN.finditer(src):
        pos, tok = m.start(), m.group()
        if not all(mask[pos:m.end()]) or not body[pos]:
            continue
        if tok in ("<", ">") and _is_template_bracket(src, pos, tok):
            continue
        if tok[0].isdigit() or tok[0] == ".":
            out.extend((pos, tok, r) for r in _num_mutation(tok))
        elif tok in SWAP:
            out.append((pos, tok, SWAP[tok]))
    return out


def mutants(src: str, k: int, seed: int) -> list[tuple[str, str]]:
    """Up to k mutants as (description, source), sampled with a fixed seed."""
    s = sites(src)
    random.Random(seed).shuffle(s)
    res = []
    for pos, tok, rep in s[:k]:
        line = src.count("\n", 0, pos) + 1
        res.append((f"line {line}: {tok} -> {rep}", src[:pos] + rep + src[pos + len(tok):]))
    return res
