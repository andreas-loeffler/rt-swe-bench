# LV2 census: heap allocation in run() of audio plugins

The same `malloc-guard` recorder as the controller census (`harness/rtswe_eval`),
armed around `LV2_Descriptor::run()` of every LV2 plugin in the Ubuntu 24.04
archive packages listed in `census/lv2/Dockerfile`. LV2 requires plugins that declare
`lv2:hardRTCapable` not to use `malloc()`, `free()` or other heap functions in
`run()` ([LV2 core](https://lv2plug.in/ns/lv2core#hardRTCapable)). We only
check allocation; `free()` is not flagged.

Method (`census/lv2/lv2_census.cpp`, one process per plugin, `census/lv2/run_census.sh`):
48 kHz, 256-frame blocks, audio and CV inputs carry a 440 Hz sine, control
inputs start at their default, MIDI inputs get a note. 16 unguarded warm-up
runs, then 400 guarded runs (note on at run 20, off at run 200). From run 25,
every 50 runs, every control input jumps to a pseudo-random valid value
(fixed seed), outside the guard. The host's URID map, log, options and worker
never allocate; worker requests run between runs, outside the guard.

Positive control (`census/lv2/control.lv2`): a plugin that declares hardRTCapable and
allocates a `std::vector` every run and `malloc(64)` every 10th run; the census
reports 440 of 440 allocations, attributed to its `run()`.

Files: `lv2_census.jsonl` (with automation), `lv2_census_defaults.jsonl`
(earlier run with default parameters only, no automation), `apt-manifest.txt`.

## Default parameters only

```
plugins: 653  measured: 649  status: {'ok': 649, 'skip': 2, 'error': 2}
declare lv2:hardRTCapable: 520 of 649 measured
allocate in run(): 1 of 649; of the hardRTCapable ones: 0 of 520
patterns (hardRTCapable, allocating): {}
```

## With parameter automation

```
plugins: 653  measured: 650  status: {'ok': 650, 'error': 3}
declare lv2:hardRTCapable: 521 of 650 measured
allocate in run(): 10 of 650; of the hardRTCapable ones: 8 of 521
patterns (hardRTCapable, allocating): {'sporadic': 8}
hardRTCapable allocating plugins taking MIDI: 3

| Package | Measured | hardRT | hardRT allocating | steady | sporadic | once |
|---|---|---|---|---|---|---|
| lsp-plugins-lv2 | 175 | 175 | 0 | 0 | 0 | 0 |
| x42-plugins | 113 | 113 | 0 | 0 | 0 | 0 |
| swh-lv2 | 105 | 0 | 0 | 0 | 0 | 0 |
| guitarix-lv2 | 72 | 72 | 0 | 0 | 0 | 0 |
| calf-plugins | 51 | 50 | 3 | 0 | 3 | 0 |
| mda-lv2 | 36 | 36 | 0 | 0 | 0 | 0 |
| invada-studio-plugins-lv2 | 18 | 0 | 0 | 0 | 0 | 0 |
| fomp | 17 | 17 | 0 | 0 | 0 | 0 |
| eq10q | 17 | 17 | 0 | 0 | 0 | 0 |
| zam-plugins | 17 | 15 | 1 | 0 | 1 | 0 |
| avldrums.lv2, avldrums.lv2-soundfont | 9 | 9 | 0 | 0 | 0 | 0 |
| zynaddsubfx-lv2 | 8 | 8 | 0 | 0 | 0 | 0 |
| dragonfly-reverb-lv2 | 4 | 4 | 4 | 0 | 4 | 0 |
| setbfree | 3 | 3 | 0 | 0 | 0 | 0 |
| geonkick | 2 | 2 | 0 | 0 | 0 | 0 |
| drumgizmo | 1 | 0 | 0 | 0 | 0 | 0 |
| abgate | 1 | 0 | 0 | 0 | 0 | 0 |
| ir.lv2 | 1 | 0 | 0 | 0 | 0 | 0 |

Top allocating hardRTCapable plugins (bytes per guarded run):
    24577.9 B/run      96 allocs in   6 runs  sporadic calf-plugins             Calf Multiband Limiter                   main (/rtswe/lv2/lv2_census.cpp:396) via calf.so
    23714.3 B/run     114 allocs in   6 runs  sporadic calf-plugins             Calf Sidechain Limiter                   main (/rtswe/lv2/lv2_census.cpp:396) via calf.so
     9370.7 B/run     512 allocs in   8 runs  sporadic dragonfly-reverb-lv2     Dragonfly Room Reverb                    main (/rtswe/lv2/lv2_census.cpp:396) via DragonflyRoomReverb_dsp.so
     6618.6 B/run     416 allocs in   8 runs  sporadic dragonfly-reverb-lv2     Dragonfly Hall Reverb                    main (/rtswe/lv2/lv2_census.cpp:396) via DragonflyHallReverb_dsp.so
     3168.4 B/run      18 allocs in   6 runs  sporadic calf-plugins             Calf Limiter                             main (/rtswe/lv2/lv2_census.cpp:396) via calf.so
     2677.4 B/run     100 allocs in   8 runs  sporadic dragonfly-reverb-lv2     Dragonfly Early Reflections              main (/rtswe/lv2/lv2_census.cpp:396) via DragonflyEarlyReflections_dsp
      985.6 B/run      48 allocs in   8 runs  sporadic dragonfly-reverb-lv2     Dragonfly Plate Reverb                   main (/rtswe/lv2/lv2_census.cpp:396) via DragonflyPlateReverb_dsp.so
      426.2 B/run     288 allocs in   8 runs  sporadic zam-plugins              ZamHeadX2                                main (/rtswe/lv2/lv2_census.cpp:396) via libzita-convolver.so.4
```

All eight hardRTCapable plugins that allocate do so in the run right after a
parameter change (runs 25, 75, ...), not in steady state. Errors: two swh
plugins fail to instantiate; LSP Multiband Limiter Mono does not finish under
automation (clean with default parameters). Site names show only the plugin
library ("via calf.so"): the archive packages have no debug symbols.
