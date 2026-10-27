# Wolf Tide project memory

Snapshot: 2026-10-27. This repository contains a source/documentation snapshot of Tide 1.1.0 for code indexing. It is not the installer or a complete vendored build distribution: JUCE vendor files, binary assets, demos, and build outputs were not uploaded.

## Implemented in the checked source
- Stereo delay and reverb, independent on/off and presets, combined presets and routing.
- Delay models: Digital, Tape, Bucket, Multitap, Reverse, Pitch.
- Reverb models: Room, Plate, Hall, Bloom. There is no dedicated Cloud model.
- Pitch modes: Climbing Repeats shifts inside feedback; Fixed Harmony shifts the return after unshifted feedback. Windowed shifting adds wet latency and can smear transients. Climbing changes effective repeat spacing.
- Shared shimmer processing surrounds the selected reverb; dedicated shimmer architectures are not implemented.
- Activity-aware DSP sleeping, cached coefficients, cubic pitch interpolation, pitch-dependent anti-alias filtering, Hann lookup tables and efficient oscillators are present.
- Grey delay and blue reverb interface, cached boutique artwork, Static/Activity/Motion graphics, mix and engine locks, A/B snapshots and scoped portable presets.

## Confirmed source issue, not yet fixed
PluginProcessor.cpp applyFactory sets pitchMode=Fixed Harmony for Vocal / Air Throw, then the defaults loop resets it to Climbing. Correct initialization order and validate factory preset intentions.

## Proposed next update, not implemented
1. Measure and improve pitch accuracy, high-interval aliasing, transient quality and repeat timing; preserve legacy sessions. Evaluate Signalsmith Stretch as an explicit Studio option only after latency/CPU comparison.
2. Dedicated original reverbs: Halo (coupled allpass shimmer tank), Prism (spectral pitch plus dedicated FDN), Glint (bloom envelope/diffusion/modulated tank), Cloud Ensemble (dense diffuse tank and ensemble modulation).
3. Delay expansion: Diffusion and Pattern first; Granular and Resonator after performance/stability validation.
4. Preset browser: Delay/Reverb/Combined scopes, direct Factory/User browsing, previous/next, Favorites/Recent, instrument tags, accurate Edited and partial-recall indicators, routing lock, Undo Last Load distinct from A/B.
5. Profile grouped coefficient updates, safe activity horizons, buffer reuse, unnecessary per-sample work, bounded transition tails and UI polling. Do not lose long buffered repeats, freeze or audible tails.
6. Preserve parameter IDs and old algorithm mappings. Test choice-parameter migrations before extending lists.
7. Validate audio, stability and CPU, then build a versioned AU/VST3 Mac installer with rollback support.

## Research and evidence boundaries
- Official BigSky plugin and firmware are research targets. Strymon's public preset librarian and preset-byte maps do not expose reverb DSP.
- Supported Eventide H9000 VSig graphs expose module connections; modules remain precompiled and some algorithms are locked. Do not assume H90 source access.
- Hologram offers Microcosm SysEx firmware; availability does not prove unencrypted executable DSP.
- We have not recovered or verified proprietary Strymon, Eventide or Hologram algorithms. Online one-shot decompilation claims are unverified leads.
- Decompilation produces reconstructed pseudocode, not original source. Validate against known-source binaries, executable references where available, and controlled audio. Track redistribution permissions separately.
- Research leads: https://github.com/Signalsmith-Audio/signalsmith-stretch ; https://github.com/GhostNoteAudio/CloudSeedCore ; https://github.com/roblg/ghidra-sharc ; https://github.com/strymon/spl ; https://www.hologramelectronics.com/pages/microcosm-firmware ; https://cdn.eventideaudio.com/manuals/h9000/2.2.11/content/development/vsig3.html

## Engineering requirements
No allocation, locking, blocking I/O or logging in audio callbacks. Profile before optimizing; report typical and peak DSP independently of UI and DAW CPU. Keep one dry path, coherent stereo, safe feedback, smooth automation and intentional tails. Transport-stop tails depend on the host continuing callbacks. Preserve old sessions and distinguish testing actually performed from planned checks. Keep maintainable code and avoid speculative abstractions. Use Graphify only after confirming repository access and index readiness; never claim memory was saved without acknowledgement.
