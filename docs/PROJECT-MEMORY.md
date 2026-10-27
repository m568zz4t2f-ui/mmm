# Wolf Tide project memory

Updated October 27, 2026. The default branch now consolidates **Tide 1.2.0**, its effect audit and available project assets. Historical 1.1/planning claims are superseded by the release source and audit. Tide is separate from Cosmos and Prism.

## Verified state

- Mac universal arm64/x86_64 AU, VST3 and Standalone 1.2.0 built and installed with rollback backups; four CTest suites and AU validation passed. See [validation](VALIDATION-1.2.0.md) and [raw logs](test-logs). These checks missed functional defects subsequently reproduced.
- Nine delay choices: Digital, Tape, Bucket, Pattern/Multitap, Reverse, Pitch, Diffusion, Granular Pitch and Lo-Fi.
- Eleven reverb choices: Room, Classic Plate, Classic Hall, Classic Bloom, Chamber, Cloud Ensemble, Halo Shimmer, Glint, Studio Plate, Studio Hall and Studio Bloom. A dedicated Cloud choice now exists; do not repeat the 1.1 “no Cloud” statement.
- 60 parameter IDs: original 44 retained, 16 appended. Preserve IDs, order, selector domains and normalized mappings. Appending choices to old selectors can break automation; use explicit extensions/migration.
- 52 combined factory programs and 40 side starters are exported in Factory-Presets. They are designed starting points, not instrument-validated optimized presets.
- Grey/blue boutique UI, scoped browser, locks, Undo Last Load and cached artwork are implemented. Hidden-editor timers suspend. Capability/UI defects remain.
- Coalesced delay-time requests during transitions and same-sample-rate preparation tails were repaired in 1.2; remaining Pattern/Reverse transitions are separate issues.
- Air Throw Fixed Harmony reset was a false earlier finding: defaults iterate floats, pitchMode is a choice. Keep intent-based preset validation, not a fix to a nonexistent reset.

## Current defects and repair order

[Full effect audit](TIDE-1.2.0-EFFECT-AUDIT.md) is the source of truth, with 356 controlled control-matrix renders plus special diagnostics.

1. Mono ping-pong swaps identical histories and does not alternate in eight tested conventional modes.
2. Halo/Studio Plate Size and Modulation do nothing: they configure an unused FDN rather than the active plate.
3. Glint/Studio Bloom reset a gain envelope across the whole existing tail on a new note, causing a major dip.
4. Pattern/Reverse ignore independent B-time and bypass the complete old/new read crossfade.
5. Reverb Diffusion also controls Diffusion delay; append an independent delay control and migrate deliberately.
6. Extension delay filtering depends on hidden legacy dType; derive behavior from the effective model.
7. Swell attenuates sustained input instead of providing an onset/attack ramp.
8. UI enables controls using legacy selector state rather than the active engine. Centralize actual capabilities.
9. Granular's upper voices duplicate at +24; Studio Plate shares the Classic tank; unpitched Glint overlaps Studio Bloom. Improve actual models before expanding names.
10. Calibrate fixed post-tank gains and band-dependent decay; improve pitch/coefficient work and preallocation only against measured quality/cost.

No audit repair has been applied to the installed plug-in. Preserve compatibility while adding behavioral tests for every advertised control, stereo routing, independent sides, tail continuity, automation and presets. Then repeat host and installer validation.

## References and boundaries

The audit records exact inspected revisions and reuse caveats for Plateau/Dattorro, RSAlgorithmicVerb, CloudSeedCore, ChowMatrix, chowdsp BBD, CHOW Tape, Daisy granular delay, Signalsmith Stretch, Faust JPverb/Greyhole and Phantasmagoria. CloudSeedCore and Signalsmith are MIT candidates; licensing differs per repository/component. Do not transplant mono-only wrappers or audio-thread allocations.

No verified exact reconstruction of BigSky/TimeLine, H90, Microcosm or Line 6 Glitz DSP has been recovered. Firmware, preset bytes, VSig graphs and decompilation claims do not establish recovered source or redistribution rights. Validate original published/open implementations with audio evidence.

## Engineering and product direction

Preserve stereo input, one dry path, independent side presets/bypass, coherent delay-to-reverb integration, safe feedback and continuous tails. Provide clean everyday delays plus distinct plate/hall/cloud/shimmer/granular algorithms. Keep grey delay/blue reverb, a cleaner boutique appearance and easy instrument presets with advanced controls contextual.

No allocation, locking, blocking I/O or logging in audio callbacks. Profile typical and peak blocks, UI separately, and inactive engines. Preserve late buffered repeats/freeze. Transport-stop tails require the DAW to continue callbacks. Meaningful listening on vocals/guitar/keys and matched hardware comparisons remain outstanding.

## Recovery and memory

See [backup index](BACKUP-INDEX.md) for branches, original release baseline, installer location and checksums. Source on the build Mac matched all 21 checked Source/tests/scripts blobs in the saved 1.2 audit baseline before this consolidation. Research diagnostics are measurements, not passing release gates.

Graphify notes acknowledge durable intake and require Memory > Needs review acceptance before publication. Its code graph is an indexed snapshot; verify its commit before trusting it for current source. At the start of this upload it indexed d38085d (old 1.1 source), despite 1.2 and audit branches already existing.
