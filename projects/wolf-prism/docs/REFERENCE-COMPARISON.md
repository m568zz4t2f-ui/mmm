# Reference and implementation comparison

Reference: Hologram Chroma Console manual, firmware v1.042, fetched from its official support page on 2026-10-26: https://www.hologramelectronics.com/pages/chroma-console-manual and linked CC_manual_WEB.pdf. Specifications describe observable behavior; internal structures below are independent engineering choices.

| Reference feature | Wolf Prism 0.1.0 implementation | Difference / evidence limit |
|---|---|---|
| Drive | ADAA tanh saturation, sensitivity, tilt and gain compensation | Original transfer/gain curve; no hardware match measured |
| Sweeten | Envelope compression, mild saturation and tilt | Original compressor behavior |
| Fuzz | Asymmetric biased ADAA saturator with higher gain | No physical transistor circuit model |
| Howl | Resonant state-variable filter into saturation | Original resonance/fuzz tuning |
| Swell | Envelope threshold and variable attack/release | Original triggering; not calibrated to hardware |
| Doubler | Stereo offset short cubic delay, modulation and direct blend | Original timing/width mapping |
| Vibrato | Fully delayed modulated read head; drift changes stereo modulation | Original LFO waveform mix |
| Phaser | 2 to 12 allpass stages with swept frequency | Original pole/feedback tuning |
| Tremolo | Variable depth and rounded-to-chopped waveform | Original waveform; no full random motor model |
| Pitch | Two overlapping Hann delay heads, -12 to +12 semitones, low-pass bandwidth control and drift quantization | Time-domain wet latency; not spectral/polyphonic hardware reconstruction |
| Cascade | Dark low-pass/saturating feedback delay | Feedback bounded below unity, so hardware runaway self-oscillation is intentionally absent |
| Reels | Brighter nonlinear feedback delay and slow pitch drift | Shares the delay framework with Cascade but changes filtering/saturation; no physical tape model |
| Space | Stereo modulated eight-line Hadamard FDN with allpass diffusion and variable decay/tone/predelay | One original scalable network; the manual's five morphing reverb sounds are not recreated |
| Collage | Dual windowed delay reads with time-dependent slicing and drift-triggered speed changes | Destructive hardware edits are approximated; not sample-matched |
| Reverse | Dual overlapping reverse heads, half to double playback speed, Amount wet blend | Original window length and feedback behavior |
| Filter | Tilt, low-pass and high-pass modes | Original one-pole/split-band implementation; tilt can color gain |
| Squash | Envelope gain compression plus ADAA saturation | Original attack/ratio law |
| Cassette | Wow/flutter variable delay, saturation and bandwidth reduction | No measured tape model |
| Broken | Periodic pitch drops, amplitude dips and ring modulation | Deterministic periodic instability rather than exact reference randomness |
| Interference | Signal-dependent noise, ring modulation, quantization | Original disturbance textures |
| Four modules / arbitrary routing | All 24 serial permutations; individual module toggles | Serial routing only, as mapped from the manual |
| Secondary controls | Sensitivity, Drift x2, output and per-module dB controls | Original ranges |
| Global Mix | Linear dry/chain blend | Full chain can include internal direct signal by effect design |
| Gesture | 30-second eight-knob automation recording, 100 Hz, tempo-scaled playback | Shared loop; no per-knob independent lengths or touch-to-delete |
| Capture | 30-second record/play/stop/erase, pre/post routing; added overdub/undo/reverse/speed | Explicit buttons replace hold-foot-switch behavior; no capture quantization |
| 80 user presets | 80 in-session memories, exported banks, 16 factory starting points | Portable banks are software-only; slots are saved with session/export, not globally shared across instances |
| Capture in presets | Excluded from user memories; current capture saved with session/bank | Session persistence is an added software behavior |
| Dual bypass | Module bitmask and separate global bypass | Explicit UI replaces hardware double-tap gesture |
| Trails | Diffusion receives zero new input while bypassed and drains | Fast type/routing switches use a short fade and at most one outgoing diffusion tail |
| Tap/MIDI/host tempo | Manual tap, tempo subdivision and optional host BPM; capture CC22-24 | Full hardware CC chart, MIDI clock/thru and expression mapping absent |
| Automatic input calibration | Sensitivity knob only | Four-level automatic calibration is not implemented |
| Hardware visual reference | Cream panel, four colored columns, same primary/secondary grouping | Independently drawn graphics/name; no Hologram logo or product artwork |

No hardware A/B, impulse-response reconstruction, firmware decompilation, or original source recovery was performed.

Engineering sources consulted:
- Julius O. Smith, Physical Audio Signal Processing: time-varying delay effects, interpolation, Schroeder reverberators and feedback delay networks. https://www.dsprelated.com/freebooks/pasp/
- JUCE 8.0.12 bundled documentation, CMake API and LICENSE.md. https://github.com/juce-framework/JUCE/tree/8.0.12
- Existing Wolf Cosmos source/build/validation files on the user's Mac, particularly BloomDSP.h and its safe fractional-delay/FDN implementation.

No external DSP implementation other than the user's Bloom core was incorporated; other algorithms were independently written here. JUCE dependency source and notices are bundled under its AGPL route.
