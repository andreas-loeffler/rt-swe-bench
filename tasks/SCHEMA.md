# Task format

One directory per task, `tasks/<id>/`:

| File | Content |
|---|---|
| `task.json` | metadata, see below |
| `issue.md` | the issue text the agent sees |
| `reference.diff` | reference patch: passes the tests and leaves `update()` allocation-free |
| `naive.diff` | a plausible patch that passes the tests but allocates (for bug fixes: often the merged upstream fix) |
| `base.diff` | bug fixes only: reverts the upstream fix, applied before the run |
| `test.diff` | bug fixes only: the upstream tests, hidden from the agent and restored for scoring |
| `oracle.txt` | allocation removals only: the malloc-guard report of the unpatched code (shown to the agent in C4) |
| `validation.jsonl` | recorded verdicts of the unpatched code, the reference and the naive patch (`verify.sh` compares against it) |

`task.json`:

```json
{
  "id": "rt-006",
  "track": 1,
  "source": "upstream fix reverted: ros2_controllers#2624 (backport #2639, 92d148a8)",
  "package": "mecanum_drive_controller",
  "probe": "task_rt_006_mecanum_zero",
  "issue": "issue.md",
  "base_patch": "base.diff",
  "test_patch": "test.diff",
  "files": ["mecanum_drive_controller/src/mecanum_drive_controller.cpp"],
  "warmup_cycles": 3,
  "notes": "naive.diff is the merged upstream fix; it resets the command history with a new std::queue(std::deque{...}) in update()."
}
```

- `track`: 1 is a bug fix (the tests fail on the unpatched code), 2 an allocation removal (the tests pass,
  `update()` allocates).
- `package`: the colcon package under test; `probe`: the gtest target in `harness/rtswe_eval` that configures and
  activates the controller, drives `update()` with malloc-guard armed and compares the outputs with recorded values.
- `files`: the files the reference touches (documentation only; the agent may edit anything except the package's
  test files, whose edits are dropped before scoring).
- `warmup_cycles`: unguarded `update()` calls before the guarded ones.

A patch is scored by `scripts/evaluate.sh tasks/<id> <patch.diff>`: F = 1 if the package tests (with `test.diff`
restored) and the probe pass, R = 1 if malloc-guard records no allocation in the guarded cycles. Every task is
validated: the unpatched code fails (bug fixes: F = 0; allocation removals: R = 0), the reference passes both
checks and the naive patch passes only the tests.
