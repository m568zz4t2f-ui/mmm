# Tide 1.2.0: effect behavior audit and reference designs

Date: October 27, 2026. This is an audit of the Tide 1.2.0 source, not Cosmos or Prism. Source/TideDSP.h matches the GitHub tide-1.2.0 branch at blob `3d471f04b6bab805c86aca83d8126f7af49b854c`; the release baseline is commit `e72b71042d57e22a334ef17379ec1b9030ab1896`. No plug-in source or installed bundle was changed during this audit. Diagnostic programs and results are included alongside this report.

## Assessment

There are reproducible functional problems. Passing build, AU validation, finite-output and nonzero-impulse tests did not establish that every control works or that the algorithms meet their musical descriptions. The 1.2 tests missed several behavioral failures described below.

I reviewed all nine delay choices and eleven reverb choices, the parameter-to-DSP mapping, contextual UI enablement, presets and relevant shared processing. The two control matrices alone use 356 controlled renders. Additional comparisons cover mono ping-pong, hidden state, model overlap, swell, new-note bloom behavior, heap allocation and decay/energy. These are source/offline tests, not listening judgments against physical Strymon/Line 6/Eventide units.

## Confirmed high-priority behavior problems

| Finding | Reproduction/evidence | Change needed |
| --- | --- | --- |
| Mono Ping-pong does not alternate in the conventional modes | At 48 kHz, a duplicated mono impulse, 100 ms, feedback .5 and modulation off produced exactly identical L/R output for Digital, Tape, Analog, Pattern, Reverse, Pitch, Granular and Lo-Fi. Diffusion produced stereo differences through its unequal allpasses; this alone does not prove alternating repeats. | Give mono ping-pong a deliberate launch/panning policy and crossed return path; verify alternating impulse arrivals and preserve stereo-input behavior. Swapping identical channel histories cannot create alternation. |
| Halo and Studio Plate Size/Modulation are ineffective | Extreme control changes produced exactly identical audio in both models. Independently reproduced at 48 and 192 kHz. BoutiqueTank uses Plate for these models, while size/modulation only affect its unused FDN branch. | Implement plate-tank size/modulation safely or mark unavailable controls honestly. The planned plate controls require an actual internal path. |
| Glint/Studio Bloom suppress an existing tail on a new note | After a gap, a fresh note resets a gain envelope multiplying the entire tank output. During the first 16 samples at 8 kHz, wet RMS was about .0038 of the otherwise identical unretriggered tail: roughly -48 dB. | Build up the new contribution/density while leaving stored late energy continuous. Simply moving the same envelope to the input would lose short impulses; use a buffered/diffused buildup path and measure its impulse response. |
| Independent B-time is ignored by Pattern and Reverse | Changing B time from 200 to 400 ms changed Digital, but produced identical Pattern and Reverse renders. Those readers use the A time/linked ratio instead. | Support independent A/B read structures in these modes or disable the incompatible option. Test manual and synced B timing separately. |
| Pattern and Reverse bypass the advertised time crossfade | Source tracing: modern Pattern overwrites the previously crossfaded main tap; Reverse reads directly from newTime. Their complete read paths do not blend old/new timing. | Crossfade complete tap/head sets, retaining independent reverse trajectories. A passing Digital timing regression does not cover these readers. Audible severity under automation still needs a listening/step test. |
| Reverb Diffusion controls the Diffusion delay | Processor maps only rDiffusion into p.diffusion; Echo's diffusion branch reads that field. A 0-to-1 sweep substantially changes delay output (relative RMS difference 1.67 in this fixture). | Append an independent delay diffusion parameter and migrate the existing setting deliberately. Loading a reverb preset must not change a locked delay's texture. |
| Extension delay tone depends on a hidden legacy choice | At an 800 ms setting, changing raw dType from Digital to Bucket while retaining the same active extension changed Diffusion/Granular/Lo-Fi output by about 4.9/8.5/8.2% relative RMS. Echo::set colors the filter from dType rather than the resolved active model. The UI leaves that underlying value behind when selecting an extension. | Resolve the active algorithm once and derive its coloration and capabilities from that value. History must not secretly voice the same visible model differently. |
| Swell is an input-dependent attenuator, not an attack ramp | For a held sine, Swell=1 reduced wet RMS from .0141 early to .00817 late; with Swell=0 it stayed approximately .0168/.0169. The formula reduces input gain as input level grows. | Implement an onset/attack envelope with hysteresis and a defined retrigger policy. Do not rename a second ducking function as swell. |
| Contextual UI reads stale engine information | Reverb Modulation enablement checks the old rType even when rModel selects a new engine. It can disable a working Cloud modulation control after Room, or enable an ineffective Halo control after Hall. Pattern/Reverse Motion and unrelated advanced controls are also displayed despite no effect. | Define capabilities using the effective model; hide/disable unavailable controls and show model-specific labels. |

These findings are about behavior or misleading availability. They are not all numerical instability, and there is no claim that every effect is broken.

## Model distinction and sound-design gaps

- Granular Pitch at +24 is effectively the same as the modern Climbing Pitch path in the tested configuration: relative difference about 6.2e-8. Its second voice is clamped to +24 and becomes a duplicate. It is a two-shifter/four-read-head texture, without an independently controlled grain scheduler, density, spray, envelopes or onset capture. Rename accurately or implement those behaviors before presenting it as a broad Microcosm-style granular engine.
- Studio Plate at zero input diffusion is exactly the Classic Plate in the rendered comparison. It adds input diffusion around the same tank, not a new modulated plate core. Sharing code is sensible; describe the variation accurately and make promised tank controls functional.
- Glint with shimmer zero and Studio Bloom with matching settings produced identical output. A shared foundation is fine, but the two models presently overlap completely under this condition.
- Legacy Bloom's new Bloom Attack control is ineffective. Its actual bloom character comes from its expanded diffusion/FDN geometry, not that control. Do not confuse the bottom performance macro with the newer attack parameter.
- Tape and Analog are functioning stylized delays, not validated tape transport/hysteresis or clocked BBD simulations. Drive's change was small at the low level used here; that is a level-dependent nonlinearity rather than a disconnected control.
- Lo-Fi is a fixed four-sample hold and approximately 12-bit quantization. Bit depth, reduction amount and alias policy are not independent options. Its present aliases may be intentional texture, but it is a limited model.

## Every delay reviewed

Control tests here use three-second renders at 16 kHz. Zero means identical output for the tested extreme settings, not necessarily that the control should exist on every model.

| Delay | Working path | Issues / useful improvement |
| --- | --- | --- |
| Digital | Time, feedback, tone, linked ratio and independent B-time respond | Repair mono ping-pong; complete timing/level automation and stereo-input tests. Clean defaults should stay unmodulated. |
| Tape | Moving-head time, modulation, feedback, tone and drive respond | Mono ping-pong fails; current drive is subtle at low level. Study transport coloration/hysteresis selectively rather than importing a complete tape plug-in. |
| Analog / Bucket | Time-dependent darkening, extra pole, drive and modulation respond | Mono ping-pong fails; actual BBD clocking/companding absent. B-side bandwidth currently follows shared/A-side design. |
| Pattern / Multitap | 2/4/8 taps, time, feedback, tone and linked ratio respond | Independent B time and Motion have no effect in modern Pattern; timing changes bypass complete crossfades; no true per-tap level/pan editing. |
| Reverse | Dual-window reverse reading, time, feedback and tone respond | Ratio, independent B-time and Motion have no effect. Reverse time changes jump the head geometry. Define slice/onset timing explicitly. |
| Pitch | Interval, time, feedback and Fixed/Climbing placement respond | Mono ping-pong fails; window latency/transient smearing remain. Measure interval automation and anti-aliasing during transitions. |
| Diffusion | Three allpasses per channel produce a real diffuse texture | Wrong-side Diffusion ownership, hidden dType coloration, and incomplete contextual controls. Stereo difference here is not an alternating ping-pong validation. |
| Granular Pitch | Two pitch voices respond to interval and feedback | Hidden coloration, +24 voice duplication, and limited granular behavior. Preserve independent stereo data when adopting a scheduler. |
| Lo-Fi | Sample hold and quantization change repeats | Hidden coloration, mono ping-pong failure, fixed amount/bit depth and no explicit alias-quality option. |

## Every reverb reviewed

Decay and Tone altered every model. Shimmer altered every model in the control fixture: shared wrapper for most, local branch for Halo/Glint. Pitch Regeneration changed only Halo/Glint, as expected from the actual code. The current drawer does not consistently communicate those distinctions.

| Reverb | Responding model-specific controls | Issues / useful improvement |
| --- | --- | --- |
| Room | Decay/Tone; shared shimmer | Motion, Size, Diffusion, Attack and Regeneration are not implemented here. Improve reflection/late-field balance and short-room coloration; keep controls honest. |
| Classic Plate | Decay/Tone; shared shimmer | No tank modulation or size control. Use a modulated coupled/allpass design and retain this legacy sound. |
| Classic Hall | Motion plus decay/tone | Size/Diffusion/Attack/Regeneration do not operate here. Consider calibrated low/mid/high decay and early/late balance. |
| Classic Bloom | Expanded diffusion/FDN and Motion | Attack control does not operate; preserve its actual existing topology and repair the UI mapping. |
| Chamber | Motion, Size and Diffusion | Attack/Regeneration unused. Size changes are abrupt; early/late balance and decay need calibration. |
| Cloud Ensemble | Motion, Size and Diffusion | A denser FDN variant; early/late shaping and smoother size automation would improve it. Its controls do work apart from unavailable generic controls. |
| Halo Shimmer | Diffusion, local shimmer and regeneration | Size/Modulation do nothing; pitch mix/regen and automated changes need smoothing/calibration. Retain independent local feedback instead of doubling the global shimmer. |
| Glint | Motion, Size, Diffusion, Attack and local pitch/regen | New notes suppress the whole tail; with shimmer zero it overlaps Studio Bloom. Rework buildup and distinguish its voicing. |
| Studio Plate | Input Diffusion; Decay/Tone | Same core tank as Classic Plate. Size/Modulation do nothing. Actual in-tank modulation/size is the useful upgrade. |
| Studio Hall | Motion, Size and Diffusion | Useful FD network/early reflections, but no Attack/Regeneration control. Smooth geometry and calibrate frequency-dependent decay. |
| Studio Bloom | Motion, Size, Diffusion and Attack | New-note tail suppression; overlaps unpitched Glint. Use real density growth and continuous late energy. |

## Loudness, decay, stability and cost

A stereo impulse comparison at 48 kHz used Mix=1, Amount=.3, nominal Decay=4 s, Tone=12 kHz, zero predelay/duck/shimmer and Bass Weight=1. T20 was fitted to the -5 to -25 dB Schroeder energy-decay interval and extrapolated to T60. These are synthetic broadband estimates, not calibrated physical-room or hardware measurements.

- Estimated T60 ranged from about 2.95 s (Chamber) to 4.25 s (Classic Bloom). Frequency-dependent losses and diffusion explain part of this; calibrate by band rather than assuming the displayed value is measured T60.
- Integrated IR energy gain ranged from about -9.41 dB (Classic Hall) to -18.38 dB (Glint/Studio Bloom): roughly 9 dB variation with equal return controls. This can contribute to some modes seeming weak. It is not a perceived-loudness ranking. Calibrate fixed post-tank gains/presets outside feedback; do not blindly normalize every running tail.
- Twelve 30-second Halo/Glint renders at 16 kHz covered all six intervals with maximum regeneration .7, shimmer 1, decay 20 and high modulation. Outputs remained finite; measured peaks stayed below .047 for the chosen .03-amplitude stimulus. No runaway was reproduced in this bounded test. This does not establish all-input stability.
- Preallocated heap payload rose from 12.94 to 17.49 MiB at 48 kHz, and 51.76 to 69.91 MiB at 192 kHz: about 35% more than 1.1. The measured one-second Digital processing path allocated zero heap objects in both versions. This is payload requested by new, not resident memory or a full audio-thread audit.
- Inactive models are already conditionally processed, which is good. However, seven new full tank instances and pitch storage are always prepared. A fixed pool of prepared transition slots may reduce memory without audio-thread allocation.
- Modern Pitch recalculates filter coefficients every 32 samples even when the target is settled. Stop rebuilding settled coefficients and benchmark the result. Preserve the current useful UI artwork cache and hidden-editor timer suspension. Existing meters count only the shared shimmer path, not all internal Halo/Glint pitch work.

## Useful existing work: what to adapt, and what not to copy wholesale

| Reference | Actual source inspected / useful part | Application to Tide / caveat |
| --- | --- | --- |
| [Valley Plateau / Dattorro](https://github.com/ValleyAudio/ValleyRackFree) | src/Plateau/Dattorro.cpp at 86f02e4: modulated allpasses, tank size scaling, distributed taps, filters and freeze transition. GPL-3.0-or-later LICENSE.md checked. | Strong architectural reference for a functioning Plate/Halo Size and Modulation path. Preserve Tide's stereo-excitation policy; do not import its GUI or assume its input summing matches Tide. |
| [RSAlgorithmicVerb](https://github.com/reillypascal/RSAlgorithmicVerb) | Source/DattorroVerb.cpp at a13e0db and README: published Dattorro plate/hall, Gardner rooms and FDN models. GPL-3.0 project. | Compare topologies and impulse responses. This particular plate file allocates two AudioBuffers in processBlock and has an empty reset; do not transplant its lifecycle/processing loop uncritically. |
| [CloudSeedCore](https://github.com/GhostNoteAudio/CloudSeedCore) | DSP/ReverbChannel.h at deb21de: distinct pre/early diffusion/late lines, per-line modulation and EQ, decay tied to length, 1/sqrt(lineCount) output scaling. MIT notice checked. | Best direct integration/prototype candidate for a richer Cloud model; begin with a bounded configuration, compare gain/density/cost and retain attribution. Its core being open does not make the commercial UI open. |
| [ChowMatrix](https://github.com/Chowdhury-DSP/ChowMatrix) | VariableDelay.cpp at 40d8e0e: interpolation variants, state transfer, time smoothing and BBD makeup gain; independent node controls in README. BSD-3-Clause main project. | Reference for independent A/B/tap routing, interpolation choices and calibrated output. Audit submodules independently. |
| [chowdsp_utils BBD](https://github.com/Chowdhury-DSP/chowdsp_utils) | BBDDelayLine.h at e97b826: clocked stage buffer and input/output filter banks. Its DSP module header declares GPLv3. | Optional higher-fidelity Analog backend, subject to clock/short-delay CPU benchmarks. This component is not automatically covered by ChowMatrix's root license. |
| [CHOW Tape Model](https://github.com/jatinchowdhury18/AnalogTapeModel) | Primary project describes its published physical tape model; GPLv3 main project. | Study saturation/transport/bandwidth principles and benchmark small relevant stages. A complete tape machine is unnecessary default overhead for every echo. DSP internals were not fully audited here. |
| [DaisySeedProjects granular delay](https://github.com/bkshepherd/DaisySeedProjects) | granulardelay_module.cpp at 8e9f468: grain size, speed, pitch, spread, envelope and hold controls. | A concrete granular behavior/reference implementation, not evidence of Microcosm code. Its ProcessStereo calls the mono path using left input only; that wrapper would discard Tide's right input. Verify the exact component/dependency licenses before reuse. |
| [Signalsmith Stretch](https://github.com/Signalsmith-Audio/signalsmith-stretch) | README: polyphonic pitch/time processing, explicit input/output latency and optional splitComputation to spread peaks. MIT. | Compare as an optional clean pitch backend. Test chords, transients, latency and worst blocks. Do not introduce it as an always-on dependency for ordinary delays. |
| [Faust JPverb / Greyhole](https://faustlibraries.grame.fr/libs/reverbs/) | Official reverb library documentation and source listing: modulated diffusion, size/damping and long feedback-delay structures. | Useful ambient/diffuse-delay reference. Upstream Greyhole is a diffusion-plus-modulated-delay design; do not repeat third-party claims that this alone means a spectral shimmer/pitch shifter. Check the chosen function and generated dependencies' notices. |
| [Phantasmagoria](https://github.com/FuzzyLotus/Phantasmagoria) | Current phantasmagoria.cpp: author documents previously inaudible Smear/Erosion controls and moves their processing into audible wet paths. | Useful lesson for perceptual control tests. Current code uses the left input and duplicates output; the spectral label does not establish FFT processing or fidelity to a commercial unit. Use as a concept/reference lead, not a drop-in stereo engine. |

I did not verify an exact open-source reconstruction of BigSky Cloud/Shimmer, Line 6 Glitz, TimeLine, H90 or Microcosm DSP. Public preset/control mappings and firmware availability do not prove access to those algorithms. Published designs, genuine open cores and matched audio references provide a more concrete improvement path than an unverified clone label.

## Repair order and acceptance gates

1. Correct basic behavior first: mono ping-pong; ineffective/mis-scoped controls; resolved-model dispatch; independent B and complete time transitions; bloom tail continuity; true swell. Preserve existing parameter IDs/order/domains, migrate added controls explicitly, and keep rollback.
2. Define one capability/control map for the effective models so the UI cannot enable a control that the selected DSP ignores. Add tests that each advertised control changes the intended path, and that loading either side leaves the other side invariant.
3. Make Plate/Halo real modulated, size-adjustable tanks. Prototype CloudSeedCore separately from Tide's existing Cloud, compare matching settings and CPU, then choose the better implementation rather than combining both blindly. Build up Glint/Bloom through density/new excitation, preserving old tails.
4. Improve the pitch primitive and actual granular behaviors. Fix duplicate upper voices and settled-coefficient CPU first; use Cosmos trajectory/voice-pool lessons and the Daisy granular control model while retaining stereo. Benchmark an optional Signalsmith backend before adding it.
5. Calibrate model return levels and frequency-dependent decay; retune the 32 musical presets using guitar/vocal/piano recordings and matched listening levels. Names and counts are not evidence of optimized voicing.
6. Add impulse, phrase-retrigger, mono/stereo, old-state, automation, alias and block-peak tests around the corrected behavior. Then rerun the full plug-in/host/installer checks. A minimum/maximum knob matrix finds disconnections but does not replace listening or a full dynamic sweep.

## Reproduce

Run from the source root with a C++17 compiler. Each program prints measurements rather than a generic PASS. These diagnostics are intentionally not added as passing release gates for known-broken behavior.

```sh
c++ -std=c++17 -O2 -I Source tests/effect_audit.cpp -o /tmp/tide-effect-audit
/tmp/tide-effect-audit
c++ -std=c++17 -O2 -I Source tests/delay_audit.cpp -o /tmp/tide-delay-audit
/tmp/tide-delay-audit
c++ -std=c++17 -O2 -I Source tests/pingpong_audit.cpp -o /tmp/tide-pingpong-audit
/tmp/tide-pingpong-audit
c++ -std=c++17 -O2 -I Source tests/target_spots_audit.cpp -o /tmp/tide-target-spots
/tmp/tide-target-spots
c++ -std=c++17 -O2 -I Source tests/shimmer_stress_audit.cpp -o /tmp/tide-shimmer-stress
/tmp/tide-shimmer-stress
```

Additional programs: tests/memory_audit.cpp (compile against both version headers for comparison) and tests/ir_audit.cpp (writes stereo float raw IRs in /tmp); tests/analyse_ir_audit.py reproduces the T20/energy CSV using NumPy. Measured results are in docs/test-logs/*audit-2026-10-27*. Input signals, durations, levels and rates are defined in the programs. Dynamic size/level automation, denormal stress, independent third-party plug-in validation, matched hardware measurements and instrument listening remain follow-up validation; they have not been silently counted as completed.
