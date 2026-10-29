# Tide Deep Current — rendering and CPU constraints

**Goal:** visually expressive, smooth, original displays without unacceptable GUI CPU/GPU/memory regression. Motion is allowed and encouraged where useful.

## Baseline
Read [UI-REFRESH-1.0.md](../UI-REFRESH-1.0.md) before profiling. That build used shared JUCE theme, cached static surfaces, bounded knob imagery, nine preallocated reverb curves, 24 Hz active and 12 Hz idle display cadence. Those were measured design choices, not arbitrary targets. Prior cloud forced-paint stress measurements rose at several 1/4/8-window scenarios, but were noisy and are NOT normal DAW paint figures.

## Implementation requirements
- Never add rendering, allocations, file reads, texture creation, locks or new analysis to the audio callback.
- Decode source art at load/init only; use optimized resolution-appropriate, immutable, shared/cached textures.
- Separate static chassis/panels/bezel from live controls and live parameter/value arcs.
- Avoid full-widget repeated repaint where subregions can be invalidated; stop hidden editor timers where supported.
- Use existing delay/reverb state for visualization; do not add FFT, new DSP, decorative audio buffering or unsafe cross-thread reads.
- Prefer simple cached geometry, low-cost alpha compositing, steady readable curves, and static highlights to per-frame blurs/particles.
- Do not automatically reduce animation quality or frame rate without profiling and user-visible review.

## Measure
Compare old baseline at `464e0c3f7047c2a50198a824b1cbaf61caed4bee` vs new branch, same machine, OS, compiler, host, sample rate, buffer, and display scale. Test editors closed/open at 1/4/8/16 instances. Measure GUI CPU, process CPU, audio callback duration, peak frame time, missed frames, timer lateness, memory and optional GPU. Reproduce with the repo's `tide_ui_refresh_checks` and bench script where available, and separately perform *native* Ableton/Retina profiling.

Do not claim any native performance acceptance from synthetic Linux/Xvfb screenshots. Log exact numbers, environment and any regressions.
