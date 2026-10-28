# Wolf Prism 0.1.0 validation

Build: 2026-10-26. Independent Chroma Console-inspired first playable implementation. Reference behavior and intentional omissions are tabulated in REFERENCE-COMPARISON.md. This record distinguishes automated checks from listening and actual DAW use.

## Platforms and native build

- Build/runtime machine: user's Intel MacBook Air, Core i7-1060NG7, macOS 15.7.9, Apple Clang 17 and macOS 15.5 SDK.
- JUCE 8.0.12, pinned source commit and bundled dependency archive; AGPL route, corresponding source included.
- Release universal x86_64/arm64 AU, VST3 and standalone built successfully. Both architecture slices inspected with lipo. Intel native tests ran; Apple Silicon runtime did not.
- AU type aumf, subtype Wprm, manufacturer Wolf. Apple auval passed including MIDI, parameter setting and ramped scheduling. Initial failure to discover the newly copied component was resolved by refreshing AudioComponentRegistrar; it was a discovery-cache failure. The failed and successful logs are retained.
- VST3 build generates moduleinfo.json through JUCE's helper. No independent Steinberg validator or pluginval execution. Ableton Live 10/12 in-app scanning, editing, saving and rendering remain untested.
- Native editor rendered through actual JUCE components in normal and FX Setup views and visually inspected. Knobs, values, labels, effect buttons and expanded controls are readable without clipping. Native control activation, attachment and session tests exercised the processor/editor. VoiceOver, keyboard-only end-to-end, high-DPI/multiple-display and prolonged GUI use remain untested.

## Automated checks passed

- All 20 modes finite and bounded at 44.1, 48, 96 and 192 kHz. Stereo core, mono->mono, mono->stereo and stereo->stereo wrapper layouts.
- Mix zero and all-modules-disabled dry identity; Space Amount 1/global Mix 1 has no immediate direct input.
- Diffusion tails survive ordinary reset, same-rate prepare and bypass-with-trails tests. No-trails/rapid switching stressed. A changed sample rate explicitly reinitializes DSP.
- All 24 routing permutations; 35 seconds of frequent type/order/extreme-control/bypass changes; nonfinite audio/controls; circular buffer addressing.
- Capture duration, record/play, overdub clearing, restore-stopped policy, maximum 30-second auto-stop and multiple forward/reverse wraps.
- Gesture recording becomes available and records expected frame count. Shared loop playback and tempo scaling implemented; no per-knob loop-length matching claim.
- 39 unique automation parameter names, 16 factory program mappings and user slot 80 recall.
- Session and portable-bank round trips include current capture; user slot stores settings/Gesture; pre-initialization state load; saved capture resampled correctly at 96 kHz.
- Truncated state and incompatible version rejected without changing current settings. Import code additionally validates complete/unique parameter IDs, numeric types/ranges, choice integrality, finite bounded loop samples, sample rates/duration, Gesture bounds and trailing data. This is not an exhaustive adversarial parser fuzz campaign.
- MIDI capture actions at offsets 32 and 200 yield the expected 168-frame capture. Other parameter MIDI CC mapping is not advertised. Block sizes 1,7,64,257,1024,4096.
- Core and wrapper heap-allocation hooks report zero allocations over checked processing loops after initialization. Application audio code contains no file I/O or blocking locks. Save/restore intentionally suspend processing outside the callback for consistent snapshots.
- AddressSanitizer + UndefinedBehaviorSanitizer core run passed on Linux x86_64. LeakSanitizer could not operate in the hosted /proc environment; it was disabled for the successful ASan/UBSan run. Memory-leak verification is not claimed.
- CTest on Mac: 2/2 DSP and host checks passed.

## Recorded music analysis

Installed Apple Loops guitar, electric piano, snare and choir recordings were decoded and processed through all 16 factory starting points at 44.1 kHz, with finite bounded output and nonzero energy. Logs include peak/RMS per preset. These are real recordings and automated processing checks, not human listening, perceived-loudness matching, blind preference results or hardware A/B tests. The loops and rendered derivatives are not redistributed. No claim of professional sonic quality or superiority is established by these tests.

## Performance

Measured after compilation finished, using a dedicated native -O3 core benchmark on the Intel Mac. 48 kHz stereo, 256-sample blocks, 2000 measured callbacks per case, four modules enabled, synthetic signal generation and timing included. No GUI or DAW overhead.

| Workload | Mean microseconds/block | P99 | Maximum observed | Real-time ratio |
|---|---:|---:|---:|---:|
| Sweeten / Doubler / Space / Filter | 192.833 | 342.427 | 498.180 | 3.617% |
| Howl / Phaser / Space / Cassette | 218.394 | 365.150 | 763.299 | 4.096% |
| Fuzz / Pitch / Reverse / Interference | 151.078 | 265.179 | 474.092 | 2.834% |

Deadline per 256 samples: 5333.33 microseconds. These are short-run observations on one computer, not worst-case guarantees, cross-architecture results or the DAW's CPU meter. No broad CPU optimization percentage versus a baseline is claimed.

## Packaging and integrity

The normal macOS .pkg contains AU/VST3/standalone plus README, AGPL license and Source.zip with the complete project and JUCE dependency archive. Expanded payload paths, universal slices, bundle identifiers and strict ad-hoc bundle signatures were checked. Source.zip CRC and required source/dependency entries checked. Package is unsigned and unnotarized; bundle ad-hoc signatures are not Developer ID signatures. The root installation wizard/privileged install was not executed. AU runtime testing used a per-user verification copy. Installer upgrade hook creates recoverable backups of previous global Wolf Prism bundles; rollback path documented. Package SHA-256 is checked again after transfer before delivery.

## Material limits

- Independently implemented approximations; no firmware extraction, decompilation or measured hardware reconstruction.
- Space is one scalable FDN, not the reference's five morphing reverb designs; feedback is constrained for stability rather than hardware runaway behavior.
- Shared eight-knob Gesture, simplified per-effect drift, sensitivity without automatic calibration, limited MIDI, explicit software capture controls and all-overdubs undo.
- No quantized capture; overlap crossfade slightly shortens loop playback period. See README.
- Presets have not been human-listened or loudness matched. Pitch/reverse wet latency is intentional; zero reported latency describes the immediate dry path.
- Tail report is 240 seconds for finite diffusion; active capture is indefinite and needs an explicit export range. A host that stops callbacks cannot render tails.
- State/bank operations suspend audio briefly. No background snapshot implementation.
