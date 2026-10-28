# Wolf Prism: project knowledge and recovery chart

Prepared 2026-10-27 from this project's visible conversation, actual source/build work, recorded test outputs and the subsequent Mac recovery check. This covers Wolf Prism; Wolf Cosmos/Wolf Tide are referenced only where they supplied verified components. Their separate latest work is not reconstructed here.

**Graphify status:** list_workspaces returned an empty workspace list on 2026-10-27. No repository destination or memory write was available. This document is a handoff for later ingestion, not evidence that Graphify saved or published these notes.

## 1. Identity and boundaries

| Topic | Confirmed knowledge | Evidence / consequence |
|---|---|---|
| Product | Wolf Prism 0.1.0, independent Chroma Console-inspired audio multi-effect | Actual compiled artifacts and installed version checks |
| Goal | Closely follow the cream-panel, four-colored-column control layout; prioritize sound, predictable behavior, efficiency and a shareable installer | User brief and attached Chroma Console photo |
| Reference | Hologram official Chroma Console manual, firmware v1.042 | https://www.hologramelectronics.com/pages/chroma-console-manual ; linked CC_manual_WEB.pdf |
| Proprietary access | No Hologram firmware decompilation, original DSP extraction, bit-exact reconstruction or hardware A/B was performed | Do not call this an exact clone or claim sonic superiority |
| Existing projects | Real Wolf Cosmos and Wolf Tide source, vendor archive, build tools and validation logs were found on the user's Mac after scratch contained no plugin source | Inspect the actual Mac projects before reimplementing shared components |
| Reused component | Wolf Cosmos Source/BloomDSP.h: safe fractional delay lines and eight-line FDN core | Preserved AGPL attribution and source; other Prism modes independently written |
| Build dependency | JUCE 8.0.12, commit 29396c22c93392d6738e021b83196283d6e4d850 | Vendor/JUCE archive bundled for offline build; AGPL route with corresponding source |
| Identity compatibility | Bundle audio.wolf.prism; manufacturer Wolf; plugin code Wprm; state magic 0x5750524d, version 1 | Preserve identifiers, parameter IDs and state compatibility in future updates |

## 2. Working effect inventory

Each entry is implemented DSP; these names do not establish equivalence with Hologram's internal algorithms.

| Module | Effect | Independent processing | Important limit |
|---|---|---|---|
| Character | Drive | ADAA tanh saturation, sensitivity, tilt, gain compensation | Original transfer curve |
| Character | Sweeten | Envelope compression, gentle saturation, tilt | Original compressor law |
| Character | Fuzz | Higher-gain asymmetric biased ADAA saturation | No transistor circuit model |
| Character | Howl | Resonant state-variable filter feeding saturation | Original tuning; cutoff bounded for sample rate |
| Character | Swell | Envelope threshold with adjustable attack/release | No automatic hardware input calibration |
| Movement | Doubler | Stereo-offset short cubic-interpolated delays with modulation and blend | Original timing/width mapping |
| Movement | Vibrato | Fully delayed modulated read head; drift alters stereo modulation | Original waveform mixture |
| Movement | Phaser | Swept 2–12-stage allpass chain | Original pole mapping |
| Movement | Tremolo | Depth-controlled amplitude modulation, rounded-to-chopped waveform | Simplified instability |
| Movement | Pitch | Two overlapping Hann delay heads, -12 to +12 semitones, bandwidth filtering and drift quantization | Intentional wet delay up to roughly 60 ms; no spectral reconstruction |
| Diffusion | Cascade | Dark filtered nonlinear feedback delay | Gain/time-constrained feedback; hardware runaway not recreated |
| Diffusion | Reels | Brighter saturating delay with slow pitch drift | Shares delay framework with Cascade; no physical tape model |
| Diffusion | Space | Modulated stereo eight-line Hadamard FDN with allpass diffusion, decay/tone/predelay control | One scalable network; not the reference's five morphing reverbs |
| Diffusion | Collage | Dual windowed reads with time-dependent slicing and speed changes | Destructive hardware edits approximated |
| Diffusion | Reverse | Overlapping reverse heads, variable half-to-double speed and wet blend | Original windows/timing |
| Texture | Filter | Tilt, low-pass, high-pass styles | Original split-band/one-pole filters |
| Texture | Squash | Envelope compression plus ADAA saturation | Original dynamics response |
| Texture | Cassette | Wow/flutter delay, saturation and bandwidth reduction | No measured tape model |
| Texture | Broken | Periodic pitch drops, amplitude dips, ring modulation | Deterministic periodic behavior |
| Texture | Interference | Signal-dependent noise, ring modulation, quantization | Original disturbance textures |

## 3. Controls, routing and performance functions

| Area | Implemented behavior | Preserve / clarify |
|---|---|---|
| Main panel | Tilt, Rate, Time, Mix; four Amount knobs; individual module enable and effect selection | All visible main controls are connected |
| Secondary | Sensitivity, Movement Drift, Diffusion Drift, Output dB and four module volumes | Diffusion volume affects the wet signal |
| Routing | All 24 serial module permutations | C=Character, M=Movement, D=Diffusion, T=Texture |
| Automation | 39 unique named host parameters | Preserve IDs; do not renumber casually |
| Channel layouts | Mono->mono, mono->stereo, stereo->stereo | Tested wrapper layout acceptance |
| Dry/wet | Global Mix 0 is unchanged dry at unity output; Mix 1 is the module-chain output | Some delay modes retain direct audio inside that chain; Space Amount 1 + Mix 1 is a true reverb return |
| Latency | Immediate dry path, reported added latency 0 | Wet pitch, reverse and time effects intentionally delay audio; never call the whole effect zero latency |
| Tempo | Tempo Sync selects musical timing; Host Tempo separately enables following DAW BPM | Tap selects manual BPM, enables sync and disables Host Tempo |
| Gesture | All eight primary controls sampled at 100 Hz, maximum 30 s shared loop, tempo-scaled playback | No separate per-knob lengths or touch-to-delete; display shows base values |
| Capture | Up to 30 s record/play/stop/erase; overdub; reverse; half/normal/double speed; pre/post FX and level | Live input continues; explicit buttons replace hold-switch behavior |
| Undo | Removes all overdubs since original capture, retaining original layer | Not multi-step undo |
| Loop boundary | Overlapping crossfade up to 8 ms or one eighth of loop | Effective playback period is shortened by overlap; recorded duration display remains original |
| Dual Bypass | Module mask: Character 1, Movement 2, Diffusion 4, Texture 8; add bits | Separate global bypass always addresses entire plugin |
| Trails/reset | Diffusion receives no new audio during bypass but can drain; ordinary reset and same-rate prepare preserve tails | Changed sample rate reinitializes runtime DSP; host must continue callbacks |
| Tail report | 240 s finite diffusion tail | Capture playback can be indefinite; export needs explicit range |
| MIDI | CC22 high record / low play; CC23 high stop; CC24 high erase, processed at sample offsets | Prism-specific assignments, not full Hologram chart; no PC, MIDI clock/thru/channel filtering or physical expression mapping |

## 4. State and preset contract

| Area | Confirmed behavior / lesson |
|---|---|
| Factory presets | 16 instrument/purpose starting points; not human-listened or perceived-loudness matched |
| User bank | 80 in-session slots, saved in DAW state/export; no global cross-instance shared user library |
| User slot content | Parameters and Gesture; Capture audio excluded, consistent with ephemeral hardware capture policy |
| Session/export content | Current settings, current Capture, Gesture and used user slots; .wolfprism portable bank |
| Audio restoration | Resamples saved capture to current sample rate, restores stopped; Play resumes |
| Input validation | Version, file size, parameter completeness/uniqueness, numeric types/ranges, choice integrality, loop rate/duration/finite samples, Gesture bounds and trailing bytes |
| Atomic commit | Parse and validate before replacing settings; tested truncated and incompatible imports leave settings unchanged |
| Size | Import ceiling 64 MiB; full high-rate Capture plus used Gesture slots can approach 55 MB |
| Consistent saving | Save/restore briefly suspend processor and acquire callback lock outside processing | 
| Save limitation | Interruption is disclosed; uninterrupted large-loop saving needs a background snapshot design |
| Compatibility | Extend with explicit state migration; do not silently invalidate existing sessions |

## 5. Fixes and engineering lessons from this implementation

| Finding | Resolution / lesson |
|---|---|
| Space initially leaked direct audio at full wet | FDN had been initialized at its default mix before smoothing toward 1; initialize embedded reverb at Mix 1. Dedicated full-wet test then passed |
| Saturator could raise effective feedback despite nominal coefficient <1 | Normalize saturation stage gain and constrain feedback according to delay duration; nominal feedback alone is insufficient stability evidence |
| Callback parameter lookups risk temporary String allocations | Cache all parameter atomic pointers used by processing; wrapper allocation hook passed afterward |
| MIDI parameter notifications risk callback work | Retained direct sample-offset Capture commands; use host automation for other knobs instead of notifying host from sample loop |
| Tap tempo could be overridden immediately by host BPM | Separate Host Tempo from tempo subdivision sync; Tap explicitly switches to manual tempo |
| Ordinary host reset/prepare could cut tails | No-op reset, preserve same-rate prepare; distinguish changed-rate initialization |
| Clearing large audio buffers can stall callback | Reused logical-generation delay clearing; bounded page initialization for overdub layer |
| Overdub undo must avoid full-loop copies in callback | Separate overdub layer with generation stamps; undo invalidates layer rather than copying entire recording |
| Preset browser could show factory name over recalled user state | Update browser/user selection from actual current program |
| Native compile error from mixed float/double min arguments | Replaced integer-power expression with explicit float multiplication for cutoff math |
| New AU appeared missing to auval | Refresh AudioComponentRegistrar and retry; subsequent full validation passed. Do not confuse registration cache failure with DSP failure |
| Generic scratch history was incomplete | Real source, build logs and installed artifacts on Mac provided stronger evidence than chat summaries |
| Synthetic tests do not establish musical quality | Added processing of real instrument recordings, retained explicit no-human-listening/no-hardware-A/B limitation |
| Optimized code needs comparable measurements | Benchmark only after compilation ends; separate core DSP from GUI/DAW overhead |
| Remote large-file reads can return running call_id | Retrieve with get_tool_result; do not blindly retry original call. Transfer ordered chunks and verify full hash |
| Deliverable links alone do not preserve artifacts | Save installer/source/guide/validation persistently; keep exact returned file IDs |

## 6. Validation evidence

| Check | Recorded outcome | Scope limit |
|---|---|---|
| Core modes/rates | All 20 modes finite/bounded at 44.1, 48, 96, 192 kHz | Not hardware sound matching |
| Dry/Space | Mix-zero/all-disabled identity and Space full-wet test passed | Not an exhaustive spectral fidelity analysis |
| Switching/routing | All 24 routes, 35 s rapid type/order/extreme-control/bypass stress passed | Not a multi-hour soak |
| Capture | Duration, overdub clearing, stopped restore, 30 s auto-stop, multiple forward/reverse wraps passed | No quantized capture implemented |
| Gesture | Recording availability and expected frame count checked | No per-knob independent cycle matching |
| State | Session/bank/slot80/pre-init recall, capture sample-rate migration, malformed import rejection passed | No exhaustive adversarial parser fuzz campaign |
| MIDI/buffers | Capture events at samples 32/200 produced 168-frame recording; block sizes 1,7,64,257,1024,4096 checked | No full MIDI chart |
| Allocations | Core and wrapper allocation hooks reported zero allocations over checked warmed-up processing loops | Host/OS behavior outside these tests not certified |
| Sanitizers | Linux x86_64 ASan and UBSan passed | LeakSanitizer disabled after hosted /proc failure; leak verification not claimed |
| Native tests | Mac CTest 2/2 passed, actual JUCE normal/setup UI snapshots inspected | VoiceOver, keyboard-only flow, varied DPI/displays and prolonged GUI remain untested |
| Real recordings | Apple Loops guitar, electric piano, snare and choir processed through 16 starting presets at 44.1 kHz; finite output/nonzero energy | Automated metering only; no redistribution of recordings; no human listening or loudness preference claim |
| AU | Installed aumf/Wprm/Wolf passed Apple auval including MIDI and scheduling after registrar refresh | Intel Mac runtime only |
| VST3 | Built, signed, moduleinfo generated by JUCE helper | Independent Steinberg validator/pluginval and Ableton sessions not run |
| Packaging | Expanded payload, paths, identities, both CPU slices, strict ad-hoc bundle signatures, Source.zip CRC and vendor archive checked | No Developer ID signature/notarization |

### Recorded core benchmark

Intel MacBook Air Core i7-1060NG7, macOS 15.7.9, Apple Clang 17, native -O3, stereo 48 kHz / 256 samples, 2000 measured blocks, four modules enabled. Includes synthetic input generation/timing. Block deadline 5333.33 microseconds.

| Workload | Mean microseconds | P99 | Max observed | Real-time CPU ratio |
|---|---:|---:|---:|---:|
| Sweeten/Doubler/Space/Filter | 192.833 | 342.427 | 498.180 | 3.617% |
| Howl/Phaser/Space/Cassette | 218.394 | 365.150 | 763.299 | 4.096% |
| Fuzz/Pitch/Reverse/Interference | 151.078 | 265.179 | 474.092 | 2.834% |

Short runs on one Intel machine; these are not a DAW CPU meter, architectural comparison or worst-case real-time guarantee.

## 7. Build, delivery, installation and recovery

| Item | Exact reference / status |
|---|---|
| Mac project | /Users/isaiahwersky/Downloads/WolfPrism |
| Main DSP | Source/PrismDSP.h ; reused Source/BloomDSP.h |
| Wrapper/UI | Source/PluginProcessor.h/.cpp ; Source/PluginEditor.h/.cpp |
| Tests | tests/dsp_tests.cpp, host_tests.cpp, stress.cpp, music_render.cpp, benchmark.cpp |
| Build | CMakeLists.txt; scripts/build-mac.command; scripts/package-mac.sh; scripts/source-archive.py |
| Mac tools | Existing CMake under /Users/isaiahwersky/Downloads/WolfTide-1.0.0/.build-tools/bin/ ; Xcode command-line tools |
| Formats | Universal x86_64 + arm64 AU/VST3/standalone, macOS 11 minimum |
| AU category | MusicEffect aumf because MIDI capture input is supported |
| Installer | dist/Wolf-Prism-0.1.0-unsigned.pkg ; 43,454,860 bytes |
| Installer SHA-256 | 5e66c059042ac12f4cfe3539bb548ee182cd2a7eda6b9a729318246c930503a3 |
| Complete source ZIP SHA-256 | 2370b6350e3d36e44d8f15822de03605e83b3ef7591380534a9dad0fed0712e2 |
| Signing | Installer unsigned/unnotarized; bundles ad-hoc signed and strictly verified |
| Distribution install | Package targets /Library/Audio/Plug-Ins/Components and VST3, /Applications/Wolf Audio, /Library/Application Support/Wolf Audio/Wolf Prism; upgrade hook backs up prior global bundles |
| Actual installed AU | /Users/isaiahwersky/Library/Audio/Plug-Ins/Components/Wolf Prism.component |
| Actual installed VST3 | /Users/isaiahwersky/Library/Audio/Plug-Ins/VST3/Wolf Prism.vst3 |
| Actual standalone | /Users/isaiahwersky/Applications/Wolf Audio/Wolf Prism.app |
| Actual installed docs | /Users/isaiahwersky/Library/Application Support/Wolf Audio/Wolf Prism |
| Actual installation method | Verified final package payload expanded and installed per-user, no privileged installer wizard; binary hashes/signatures and both slices checked |
| Installation evidence | docs/mac-install-final.json ; docs/mac-auval-installed-refreshed.log |
| Recovery evidence | Later read-only Mac check found source, installer/source ZIP, installation receipt and all three installed v0.1.0 bundles; installer hash matched |
| DAW usage | Rescan VST3 plugins and load Wolf Prism; do not restart/modify an unsaved Live session automatically |

### Persistent deliverable identities

| Artifact | Library file ID |
|---|---|
| Wolf-Prism-0.1.0-unsigned.pkg | libfile_12eb8236799c8191b45a085b3440ef74 |
| Wolf-Prism-0.1.0-Source.zip | libfile_1bd369ae8d2c8191a561178d4828f337 |
| Wolf-Prism-Getting-Started.md | libfile_fe6556f79d0881919a3640a3fa7a065e |
| Wolf-Prism-Validation.md | libfile_303707ddfc1481919ac80d0c48cc0d1e |

These IDs came from successful saves in this conversation; their contents were not refreshed during creation of this retrospective chart. Earlier scratch files may have been pruned by automatic maintenance. Prefer saved artifacts or the verified Mac project for recovery.

## 8. Open work and next-step boundaries

| Priority | Open item | Evidence needed before claiming completion |
|---|---|---|
| High | Musical audition and perceived-loudness matching | Human listening on guitar/chords/transients/voice/keys at matched levels |
| High | Ableton actual host behavior | Explicitly requested DAW test on saved/temporary set; scan/load, automation, state and render results |
| High | Apple Silicon runtime | Real arm64 machine testing; universal compilation alone is insufficient |
| High | More faithful Space and reference behavior | Distinct measured/musically justified architectures; hardware/reference comparison |
| Medium | Background loop/bank snapshots | Demonstrate consistent saved audio without suspending processing |
| Medium | Clearer preset workflow | Named/shared user library, favorites/search or scoped browsing only as useful; retain existing IDs/state |
| Medium | Gesture parity and readable live modulation | Per-knob behavior and base-versus-modulated-value UI validation |
| Medium | Capture timing | Exact/quantized loop boundary design without undocumented timing shifts; loop/export checks |
| Medium | Wider DSP validation | Filter variations, pitch artifact/alias measurements, extreme automation, long soak and parser fuzzing beyond current evidence |
| Medium | Release engineering | Independent VST3 validation, accessibility, high-DPI, signed/notarized installer if credentials become available |
| Pending access | Graphify ingestion | Accessible workspace + correct repository; save separate self-contained facts with file/symbol references; verify recall |

Future Graphify ingestion should distinguish completed fixes, test observations, implementation choices and proposed work. The remember API's intake acknowledgement is not publication: extracted candidates may need acceptance in Memory > Needs review. Do not mark notes published or indexed merely because intake succeeded.
