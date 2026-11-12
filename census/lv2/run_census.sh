#!/usr/bin/env bash
# Build the LV2 census host and run it on every installed plugin, one process
# per plugin (a crash or hang only loses that plugin). Inside the lv2 image:
#   docker run --rm -v "$PWD":/rtswe:ro -v "$PWD/results/lv2":/out rt-swe-bench/lv2:noble \
#     bash /rtswe/census/lv2/run_census.sh
set -u
OUT=/out/lv2_census.jsonl
BIN=/tmp/lv2_census
g++ -std=c++20 -O1 -g /rtswe/census/lv2/lv2_census.cpp -o "$BIN" \
  -I/rtswe/harness/rtswe_eval/include -I/opt/malloc_guard/include \
  $(pkg-config --cflags --libs lilv-0) -L/opt/malloc_guard/lib -lmalloc_guard -ldl || exit 1
cp /opt/apt-manifest.txt /out/apt-manifest.txt
: > "$OUT"
lv2ls | sort | while read -r uri; do
  line=$(timeout 60 "$BIN" "$uri" 2>/dev/null | tail -n 1)
  rc=${PIPESTATUS[0]}
  if [[ "$line" == \{*\} ]]; then
    echo "$line" >> "$OUT"
  else
    printf '{"uri":"%s","status":"error: exit %s"}\n' "$uri" "$rc" >> "$OUT"
  fi
done
# package of each plugin bundle
python3 - "$OUT" <<'PY'
import json, subprocess, sys, urllib.parse
rows = [json.loads(l) for l in open(sys.argv[1])]
for r in rows:
    b = r.get("bundle", "")
    path = urllib.parse.unquote(b[len("file://"):]) if b.startswith("file://") else ""
    pkg = ""
    if path:
        p = subprocess.run(["dpkg", "-S", path.rstrip("/")], capture_output=True, text=True)
        pkg = p.stdout.split(":")[0] if p.returncode == 0 else ""
    r["package"] = pkg
open(sys.argv[1], "w").write("".join(json.dumps(r) + "\n" for r in rows))
PY
wc -l "$OUT"
