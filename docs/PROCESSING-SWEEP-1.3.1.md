# Tide 1.3.1 processing candidate — individual effect audit

Date: October 30, 2026 (Pacific).

Base: `m568zz4t2f-ui/mmm`, `ui/deep-current-complete`, commit `f4e4123a6a2c4bba3ed4b29712bce1c7ac55e81c`.

## What this delivers

An individual code audit of all twelve Tide 1.3 delays and thirteen Tide 1.3 reverbs, applied processing corrections, distinct signal-flow descriptions, regression tests, and baseline/candidate measurement tools. This is an audio-changing **candidate**, not a certified release or a claim that every possible defect has been eliminated.

The UI, artwork, parameter IDs, preset indices and schema are unchanged. CMake's version is advanced to 1.3.1 to distinguish this processor build. The frozen compatibility code in `TideDSP.h` and `BloomDSP.h` is unchanged. Saved sessions selecting the existing **Tide 1.3** processing path WILL sound different with this candidate; there is no new switch preserving the old 1.3 algorithm. Use the original build for exact restoration of those sounds until a release migration policy is chosen. Legacy processing remains the compatibility option it was before this work.

Shared buffers, filters and utility code are appropriate. Each named effect must have a distinct combination of topology, read behavior, nonlinear processing, scheduling or envelope behavior. Different knob defaults alone do not establish an independent algorithm. The table below states the actual differences rather than claiming twenty-five unrelated codebases.

Status vocabulary:

- **Applied**: implemented in this candidate.
- **Verified**: checked by a named numerical test or measurement.
- **Open**: a known limitation or proposed experiment requiring more work; not silently treated as solved.
- Numerical response differences do not establish a perceptually better sound.

## Individual algorithm identity

| ID | Effect | Actual processing identity in the candidate |
| --- | --- | --- |
| D01 | Digital | Forward cubic stereo reads, crossfaded time changes, linear normal-range feedback, tone/low-cut loop filtering. |
| D02 | Tape | Moving read time, wow/flutter/irregular drift, age-related bandwidth loss, saturating repeats. |
| D03 | Bucket | Clocked held/companded recording with clock-related filtering and nonlinear expansion. |
| D04 | Multitap | Four independently weighted/panned/time-scaled reads, separately weighted and now filtered feedback summation. |
| D05 | Reverse | Two overlapping backward phrase readers with normalized per-phrase attack windows. |
| D06 | Pitch | Forward variable-speed reads, waveform-matched short splices, continuously filtered bandwidth histories, fixed/climbing routing. |
| D07 | Diffusion | A forward echo feeding a variable blend of up to six allpass stages in the repeat path. |
| D08 | Granular Pitch | Twelve scheduled voices with independent retained read trajectories, captured history, interval/spread and writer freeze. |
| D09 | Lo-Fi | Quantization plus reduced-rate sample hold, independent noise samples and separate reconstruction filtering. |
| D10 | Oil Can | Two playback distances, dispersive modulated allpasses, irregular rotor drift, dark age response and residual regeneration. |
| D11 | Spectral | Six rate-aware bandpass/timing paths; a frequency-band delay, not an FFT processor. |
| D12 | Magnetic Drum | Four fixed rotary-head ratios, independent playback-head losses, magnetic-style saturation and filtered feedback. |
| R01 | Room | Four damped parallel combs, output allpass diffusion and a short early-reflection field. |
| R02 | Plate | Two orthogonally coupled branches with three dispersive allpasses per branch and distributed output taps. |
| R03 | Hall | Eight-line Householder feedback network, two input diffusion stages and separate early reflections. |
| R04 | Bloom | Eight-line pair-rotation/permutation feedback network, three input allpasses and a squared attack envelope. |
| R05 | Chamber | Six cross-coupled damped combs, simultaneous neighbor-state reads, output diffusion and tight early reflections. |
| R06 | Cloud Ensemble | Sixteen-line Householder network, longer independent modulation and six additional output diffusion stages per channel. |
| R07 | Halo Shimmer | Twelve-line pair-rotation/permutation tank, four input dispersion stages and shifted late-field regeneration/layering. |
| R08 | Glint | Short twelve-line permuted Householder tank, early-reflection pitch layer and three short input dispersion stages. |
| R09 | Studio Plate | Four-branch Hadamard scattering with four separately stored allpass stages per branch and distributed taps. |
| R10 | Studio Hall | Sixteen-line normalized Hadamard feedback network with input diffusion and early reflections; distinct from Cloud's Householder/output-diffusion path. |
| R11 | Studio Bloom | Twelve-line pair rotation with a different permutation, five input and three output dispersion stages, smoothstep attack envelope. |
| R12 | Nonlinear | An eight-line late field with onset-timed finite gate, adjustable release and rising envelope shape. |
| R13 | Resonator | Eight harmonically related feedback combs with low-pass phase compensation and out-of-band partial suppression. |

## Shared corrections and remaining concerns

**C01 — Applied: changed-value coefficient updates.** Delay and reverb settings have explicit value comparisons, including every tap. Unchanged blocks no longer rebuild all the machine coefficients. Spectral band coefficients are calculated only for the Spectral model and refreshed after sample-rate preparation. This avoids speculative threading or added dependencies.

**C02 — Applied: normal-range clean feedback.** Clean delay families no longer pass every return through tanh. The analog-character families retain nonlinear feedback; clean paths retain an emergency bound outside the normal operating range. A two-level Digital fixture verifies proportional response.

**C03 — Applied: delay filtering transitions.** Tone and low-cut poles follow their targets; tape/oil bandwidth and BBD clock-related controls also follow smoothed targets. This does not prove that every parameter combination is click-free. Tap positions/pans, spectral Q, resonator tuning and some geometry changes need further automation listening.

**C04 — Applied: delay-proportional reverb attenuation.** Each line has its own smoothed low-pass pole. The one-pole attenuation is matched at Nyquist relative to a 60 ms reference path, scaled by line length. This is an inexpensive approximation, not a complete multiband RT60 fitting method. Intermediate-frequency decay, interpolation loss and allpass group delay still affect measured decay.

**C05 — Applied: remove unused early-reflection work.** Reverbs without an audible early-reflection route skip that computation. Each slot still retains prepared storage so model changes allocate no memory in the callback.

**C06 — Verified, limited scope: processing allocations.** The existing instrumented stress suite tracks zero processing allocations over its fixtures at 44.1, 48, 96 and 192 kHz. This covers the candidate's new pitch buffers and coefficient updates. It is not proof about arbitrary host code.

**C07 — Open: return calibration and presets.** Existing fixed per-model trims remain. Corrected pitch cancellation, reverse windows, rotor behavior, new tank structures and Glint's early layer change levels. The included energy measurements expose these changes. Recalibration needs multiple musical recordings and matched listening; one synthetic fixture is insufficient for perceived-loudness normalization. No dynamic gain normalization was added to conceal differences.

**C08 — Open: release validation.** Mac AU/VST3 compilation, DAW automation, multiple-instance deadlines, reference-guitar listening and installers are untested. The existing wet-active ceiling remains. Checking how often ordinary presets hit it is still required.

## D01 — Digital

1. **Problem:** unconditional tanh in the feedback made a supposedly clean delay level-dependent. **Applied:** preserve linear returns in the normal range. **Verified:** tenfold input produces tenfold output in the targeted feedback fixture.
2. **Problem:** tone/low-cut coefficient jumps could add parameter artifacts. **Applied:** smoothed poles.
3. **Open:** cubic interpolation is not perfectly flat for all fractional delays. Measure frequency response at fractional and integer times, including high feedback.
4. **Open:** output headroom and mix/preset calibration need host-level listening. Emergency clipping is protection, not a normal sound-shaping target.

## D02 — Tape

1. **Problem:** age-related playback bandwidth could jump immediately. **Applied:** smooth the tape bandwidth target separately from the regular tone target.
2. **Problem:** unrelated Spectral coefficients were recomputed on each parameter block. **Applied:** changed-value/mode-specific updates.
3. **Identity retained:** continuous moving time, separate wow/flutter and irregular drift, age filtering and nonlinear repeats. It is not Digital with a darker default.
4. **Open:** this is a tape-inspired delay, not a circuit or hysteresis model. The time-control response is a moving-length approximation rather than a complete motor-speed transport model. Compare record/playhead filtering and nonlinear aliasing before adding a physical model.
5. **Open:** saturation oversampling/ADAA and long-note modulation tuning require alias spectra and guitar listening. No measured tape-equivalence claim is made.

## D03 — Bucket

1. **Problem:** clock rate/filter targets could jump while delay time slewed. **Applied:** smooth clock-rate and clock-filter targets with the moving delay controls.
2. **Identity retained:** clocked hold, nonlinear compression/expansion and clock-dependent bandwidth remain separate from Tape's transport/saturation behavior.
3. **Open:** the current compander is a static approximation. Real BBD systems have envelope dynamics and clock-dependent reconstruction behavior.
4. **Open:** residual clock/expansion aliasing has not been quantified. BLEP or more complete reconstruction is a research candidate, not implemented circuit emulation. Test both stereo timing lanes when the right time differs from the main time.

## D04 — Multitap

1. **Confirmed defect:** raw weighted feedback taps bypassed the tone and low-cut path applied to the audible return. **Applied:** independent feedback-filter states shape each successive pass.
2. **Problem:** feedback also inherited clean-mode tanh compression. **Applied:** linear normal-range feedback.
3. **Verified:** a high-frequency burst loses substantially more energy on the second repeat with a dark tone setting; the new regression checks successive-pass behavior, not merely knob response.
4. **Open:** tap time, pan, output level and feedback-weight automation can still jump. Decide the desired time-jump behavior before adding per-tap transitions.

## D05 — Reverse

1. **Confirmed structural issue:** a second sine/attack envelope gated the entire output after two phrases had already been windowed, adding periodic level holes. **Applied:** shape and normalize the two phrase windows themselves; remove the extra whole-output gate.
2. **Problem:** unconditional nonlinear feedback altered reverse repeats. **Applied:** normal-range linear feedback and smoothed output filters.
3. **Open:** phrase-boundary attacks and timing changes still need listening with isolated plucks and percussion. A reverse phrase necessarily has capture/read timing; it is not equivalent to a forward repeat at the same Time value.
4. **Open:** recovered envelope energy changes preset loudness. Existing trim must be reconsidered after audition.

## D06 — Pitch

1. **Confirmed defect:** regularly spaced four-head Hann blending produced severe note-dependent cancellation and periodic sidebands. **Applied:** one primary variable-speed trajectory, short smoothstep splices and bounded stereo waveform matching at splice points.
2. **Confirmed design concern:** globally smoothing the read window changed active head speed. **Applied:** apply a new window at a splice, retaining each active old/new trajectory during its crossfade.
3. **Confirmed design concern:** changing only an input filter cannot bandlimit samples already stored before an interval change. **Applied:** maintain full, half and quarter-band filtered histories continuously, selecting/crossfading histories as read speed increases. Coefficients do not jump with pitch automation.
4. **Problem:** pitch-routing changes moved the shifter abruptly into/out of feedback. **Applied:** morph fixed/climbing feedback contributions while advancing the shifter exactly once per sample.
5. **Verified:** 54 low-guitar/sine fixtures cover all six exposed intervals, three textures and three notes. Each must retain the requested-frequency component above 0.75 of input amplitude and RMS amplitude between 0.8 and 1.15. Linked stereo proportionality is also checked.
6. **Open:** waveform matching does not guarantee transparent polyphonic chords or pick transients. Signalsmith Stretch/Rubber Band comparisons have NOT been implemented or benchmarked. Large-shift alias rejection and imaging still depend on the finite-order filters/interpolator. Filter-bank transitions need broadband listening.
7. **Open:** pitch-window timing participates in Climbing feedback. No fixed host-latency compensation or misleading tempo correction was introduced. Search is bounded but occurs at splices; worst callback duration needs Mac testing, not just average CPU.

## D07 — Diffusion

1. **Confirmed structural issue:** density selected an integer number of allpasses, abruptly inserting or bypassing whole stages. **Applied:** continuously blend stage contributions while advancing all six stage histories.
2. **Problem:** clean diffuse repeats were unconditionally saturated. **Applied:** linear normal-range feedback with existing bounded protection.
3. **Tradeoff:** keeping inactive diffusion stages warm adds work; inspect its individual profile rather than claiming universal CPU reduction.
4. **Open:** changing texture moves allpass delay lengths and can pitch-bend or create transients. Diffusion amount, decay and coloration must be evaluated together with impulses and short guitar attacks.

## D08 — Granular Pitch

1. **Problem:** overlapping active voices could exceed the gain budget estimated from requested density. **Applied:** normalize excess actual window-weight overlap, preserving natural low-density attack rather than filling gaps with automatic gain.
2. **Identity retained:** every voice keeps its own captured-history trajectory; freeze stops the writer. It remains a granular texture, distinct from the clean Pitch path.
3. **Verified:** existing freeze fixtures confirm that new live input is ignored while captured audio remains audible; density/texture controls and reset pass.
4. **Open:** interval changes still update the capture filter while old voices/history exist. A per-voice/banked capture-bandwidth design should be measured separately. Continuous Spread is a non-diatonic companion ratio by design, not key-aware harmony. Peak normalization cannot prevent phase cancellation between grains.

## D09 — Lo-Fi

1. **Confirmed defect:** the same tone-filter state was advanced twice per sample, once before and again after sample hold. **Applied:** a separate reconstruction-filter state.
2. **Problem:** both channels received the same random noise value. **Applied:** independent bounded noise samples, while preserving existing stereo audio input.
3. **Problem:** unintended generic feedback saturation was layered over quantization. **Applied:** normal-range linear feedback so the quantizer/hold defines this family's degradation.
4. **Open:** rate-reduction aliasing and quantization are intentional here. Dither/noise shaping would be a separate sound-design choice, not automatically an improvement. Test near-silent notes and bit/rate automation.

## D10 — Oil Can

1. **Identity shortcoming:** one primary tap with two dispersive stages did not express the common two-head/residual-charge behavior of oil-can echoes. **Applied:** a second playback distance and bounded residual regeneration even with the user feedback at zero.
2. **Identity shortcoming:** age affected dispersion but did not independently darken charge retention. **Applied:** smoothed age-related bandwidth loss and irregular drift in the rotor/dispersion behavior.
3. **Retained:** dispersive allpasses, nonlinear recirculation and strong warble remain distinct from the tape transport and drum heads.
4. **Open:** 0.73 playback ratio, residual amount and bandwidth are original tuning choices, not measured hardware constants. Test timing expectations, zero-feedback residual repeats, modulation extremes and preset levels. This is an inspired algorithm, not an oil-can replica.

## D11 — Spectral

1. **Confirmed sample-rate issue:** fixed high center frequencies clamped several bands to the same upper frequency at low rates. **Applied:** distribute six centers logarithmically within the current sample rate's usable band.
2. **Confirmed waste:** every model rebuilt these coefficients. **Applied:** calculate them only for Spectral and only when relevant settings change.
3. **Verified:** re-preparing a used Spectral machine at 8 kHz matches a freshly prepared machine at that rate.
4. **Open:** six resonant bandpasses leave a characterful, incomplete spectral coverage. It is not an FFT bin-delay editor. Evaluate Q/texture changes, band balance and transients before expanding bands or adopting an allpass/FFT design. The existing large return trim needs reassessment with musical sources.

## D12 — Magnetic Drum

1. **Identity shortcoming:** fixed head ratios alone were too close to a restricted Multitap. **Applied:** independent playback-head low-pass histories, with loss increasing by head distance/age; retain fixed rotary head geometry and magnetic-style nonlinear repeats.
2. **Confirmed feedback issue:** weighted return taps bypassed the audible filtering. **Applied:** dedicated filtered feedback return, as for Multitap.
3. **Problem:** age-related bandwidth changes could jump. **Applied:** smoothed main tape/head-bandwidth path and changed-value coefficient updates.
4. **Open:** per-head coefficient changes are not yet crossfaded. Saturation and motor/head loss are approximations; no measured magnetic-medium equivalence. Head selection/level automation and correlated summed peaks need musical checks.

## R01 — Room

1. **Concern:** evenly stepped comb lengths encouraged repeated geometric relationships. **Applied:** a nonuniform four-comb geometry, retaining Room's separate short reflection field and output diffusion.
2. **Concern:** identical filter poles produced different loss per second in different-length loops. **Applied:** per-line smoothed attenuation approximation.
3. **Open:** four combs can retain audible resonances. Short-room character is not automatically a defect. Measure echo-density buildup, narrow resonances and early/late balance at small/large Size before adding more lines.

## R02 — Plate

1. **Concern:** two branch lengths were generated from an arithmetic progression. **Applied:** explicit asymmetric branch geometry and per-line damping.
2. **Applied efficiency:** skip unused early-reflection processing and unchanged coefficient rebuilds.
3. **Identity retained:** two coupled dispersive branches with distributed taps. It is an original plate-like tank, not a claim of exact Dattorro or physical plate reproduction.
4. **Open:** nominal decay gains do not fully account for the frequency-dependent group delay of the allpasses. Retune measured decay, attack and metallic character across Size/Diffusion; do not assume the Decay label is exact RT60.

## R03 — Hall

1. **Applied:** retain an eight-line Householder tank with separate early reflections/input diffusion, and use delay-proportional attenuation.
2. **Open:** injection/output signs and modal excitation remain hand-tuned. Offline optimization of those gains/matrix is a candidate supported by literature, not performed here.
3. **Open:** output width is a mid/side operation, not a spatial acoustic model. Test mono compatibility, late-tail coloration and modulation depth on sustained chords.

## R04 — Bloom

1. **Identity shortcoming:** it shared Hall's eight-line scattering and mainly changed diffusion geometry. **Applied:** adjacent-pair orthogonal rotations followed by a cyclic permutation, giving a different feedback interconnection.
2. **Envelope shortcoming:** input diffusion alone provided attack changes without a clear controlled late-field rise. **Applied:** an explicit squared attack envelope, in addition to input dispersion.
3. **Applied:** per-line damping and removal of unused early-reflection computation.
4. **Open:** retriggering after input gaps resets the shared late-field envelope and can affect existing tails. Listening must determine whether retrigger, continuous swell or a per-excitation envelope is preferable. New geometry is not yet optimized for minimal coloration.

## R05 — Chamber

1. **Confirmed ordering issue:** later combs could read a neighbor after that neighbor had already advanced for the sample. **Applied:** snapshot every coupling read before any writer advances.
2. **Applied:** nonuniform six-comb lengths and per-line attenuation, retaining short chamber reflections and three output allpasses.
3. **Open:** neighboring paths have different travel times; the coupled loop's true bandwise decay needs measurement. Uniform gain formulas alone do not certify the complete network. Verify close, tight ambience rather than converting it into another Hall.

## R06 — Cloud Ensemble

1. **Applied:** delay-proportional damping across the sixteen long lines and removal of unused early-reflection work.
2. **Identity retained:** Householder feedback plus independent deeper delay modulation and six extra output diffusers per channel. Studio Hall now uses a different matrix and lacks Cloud's output ensemble chain.
3. **Open:** deeper modulation can create pitch movement and time-varying interpolation loss. Measure stereo correlation, mono fold-down and long-note pitch texture at maximum Motion. Dense does not guarantee colorless.

## R07 — Halo Shimmer

1. **Confirmed defect:** any nonzero Shimmer activated a full regeneration-related reduction in base tank feedback. **Applied:** smooth Shimmer/Regeneration and scale the loop contribution/budget continuously by the actual Shimmer amount.
2. **Verified:** the 2–3 second late-energy ratio for Shimmer 0.001 versus 0 is approximately 0.9966 in the regression, replacing the previous roughly 18 dB discontinuity.
3. **Identity improvement:** twelve-line pair-rotation/permutation scattering and nonuniform long geometry, retaining four input dispersion stages and shifted late-field layering.
4. **Applied:** shared new pitch utility, prepared bandwidth histories, per-line tank damping, and suppression of new shifted regeneration during Hold.
5. **Open:** full Shimmer/Regeneration/Decay/Hold combinations need long stress renders and listening. Matching base-tail continuity at zero does not prove identical RT60 at every pitched setting.

## R08 — Glint

1. **Identity shortcoming:** it used substantially the same whole-tail pitch treatment as Halo. **Applied:** pitch the early-reflection field for short bright sparkle while retaining its own shorter twelve-line permuted Householder late tank.
2. **Applied:** the shared continuous regeneration fix, new pitch utility and per-line damping.
3. **Open:** early sparkle can be louder or more transient than the old whole-tail layer. Retune pitch amount/return trim with plucks and chords. Preserving deliberate brightness still requires control of large-shift aliasing.

## R09 — Studio Plate

1. **Applied:** explicit nonuniform four-branch geometry and independent attenuation targets, preserving the genuine four-way Hadamard topology and separately stored branch diffusion.
2. **Applied efficiency:** skip unused early reflections and repeated coefficient rebuilding.
3. **Open:** four branches plus allpass travel do not imply exact labeled decay. Compare buildup, stereo spread, transient peaks and late ringing with the two-branch Plate at matched perceived level. Physical-plate measurement matching is not claimed.

## R10 — Studio Hall

1. **Identity shortcoming:** it and Cloud used the same sixteen-line Householder feedback operation, differentiated largely by geometry/output processing. **Applied:** a normalized sixteen-point Hadamard transform for Studio Hall.
2. **Applied:** keep explicit early reflections/input diffusion and per-line attenuation; Cloud retains its ensemble/output-diffusion path.
3. **Open:** a Hadamard transform preserves energy but does not automatically produce a better modal distribution. Measure and audition spectral flatness, density growth and stereo behavior against the previous Householder version.

## R11 — Studio Bloom

1. **Identity improvement:** twelve-line pair rotations with a different permutation from Bloom, plus five input and three output dispersion stages.
2. **Applied:** an explicit smoothstep attack curve, distinct from Bloom's squared rise; retain separate output buildup processing.
3. **Applied:** per-line damping and removal of unused early reflections.
4. **Open:** its long diffusion/envelope chain can smear attacks and alter nominal decay. Check gap retriggers, long Attack, Size changes and the two-tail slot limit. Larger smoothing is a sound-design option, not a universal quality score.

## R12 — Nonlinear

1. **Confirmed behavior defect:** gate duration followed time since the last nonzero sample, so sustained input could hold it open indefinitely. A generic input-presence gate increment also counteracted release. **Applied:** an onset detector/refractory interval, onset-relative gate clock and a single owning gate envelope.
2. **Verified:** the finite gate closes on sustained input after its configured onset window.
3. **Applied:** retain late-field diffusion/shape control and per-line damping; remove unused early-reflection work.
4. **Open:** this simple onset detector can miss or retrigger on complicated playing. Test repeated strums, tremolo, palm mutes and slow volume swells. It is a gated/swell reverb, not a model of nonlinear acoustics.

## R13 — Resonator

1. **Confirmed tuning issue:** the shared eight-sample minimum forced high partials to identical delay lengths at low sample rates. **Applied:** a resonator-specific fractional minimum and mute partials above the usable Nyquist margin.
2. **Confirmed tuning concern:** damping adds phase delay inside the loop, lowering resonance frequency. **Applied:** compensate the one-pole phase delay at each requested partial frequency when calculating loop length.
3. **Identity retained:** a harmonically related comb bank, with original broad harmonic excitation rather than replacing it with an excessively quiet narrow modal-filter bank.
4. **Verified:** high-tuning/8 kHz fixtures remain finite and retain in-band response; standard model and control tests pass.
5. **Open:** linear interpolation and damping still affect high-frequency tuning/decay. Size continues to scale fundamental pitch. Verify target peaks and tuning automation across rates, particularly close to Nyquist and with long Decay/Hold.

## Compatibility paths inspected separately

The frozen engine contains nine delay selections (Digital, Tape, Bucket, Multitap, Reverse, Pitch, Diffusion, Granular Pitch and Lo-Fi) and eleven reverb selections (Room, Plate, Hall, Bloom and seven BoutiqueTank extensions). Their lower-order interpolation, original two-head pitch utility, fixed allpass chains and older gain/motion behavior remain intentional restoration behavior.

The legacy Bucket is primarily filtered/colored delay behavior rather than the newer clocked machine. Legacy Boutique Halo/Studio Plate use the coupled plate tank; other extensions share eight-line scattering with different input/early/buildup routes. Those identities were inspected, not relabeled as independent new algorithms. Improving the frozen code silently would break saved-sound restoration. The applied changes are restricted to the Tide 1.3 processing path; numerical compatibility comparison remains part of the regression suite.

## Validation and measurement scope

Run `python3 tests/run_processing_sweep.py` from a checkout with a C++17 compiler named `g++`. It compiles the standalone DSP checks without downloading JUCE or installing a plugin. The eight suite results and sanitizer outcome are saved in `docs/processing-sweep/validation.log`.

The checks cover legacy behavior, all 25 machines at five rates, distinct numerical responses, all 337 exposed control contracts, capture freeze, reset, four-slot routing, bounded automation, allocation instrumentation and the added specific regressions. AddressSanitizer/UndefinedBehaviorSanitizer are included. LeakSanitizer is disabled because it cannot inspect the managed process environment; a tracked-allocation test is not a leak detector.

`tests/processing_sweep_metrics.cpp` renders each effect individually at 48 kHz, 64-sample blocks, with unchanged parameter updates every 256 samples. It records peak, total/late energy, stereo correlation, total render time and worst observed block wall time. Halo/Glint use a nonzero Shimmer amount in this fixture. Input is a short two-tone burst followed by silence. The paired CSVs preserve the measurements, not a subjective ranking. Wall-time outliers include scheduling on a shared Linux environment; these are not isolated DSP instruction timings or Mac buffer guarantees.

The prepared modern Engine's heap payload increased from about 43.50 MB to about 45.09 MB at 48 kHz because the pitch shifters retain three filtered histories. At 192 kHz it is about 180.28 MB, plus the separately prepared legacy engine and host overhead. Memory is retained for transitions, and the measured stress fixtures allocate nothing during processing.

### Release gates still open

1. Matched-level guitar/keys/vocal/percussion auditions, with special attention to chords and note decay.
2. Per-model/preset return recalibration using multiple musical recordings, including the newly louder recovered pitch/reverse energy.
3. Bandwise reverb decay, echo density, modal coloration, stereo/mono and long Hold tests across a larger parameter grid.
4. Alias/imaging measurements under interval changes, nonlinear stages and intentional Lo-Fi/BBD degradation.
5. Mac AU/VST3 compile/host checks, worst-buffer/multi-instance CPU, installer validation and a saved-session migration policy for the changed modern path.

## Primary research used

These sources inform experiments and identity checks. They do not establish that Tide reproduces the authors' implementations or any named hardware.

- Driedger and Müller, *A Review of Time-Scale Modification of Music Signals*: waveform alignment, polyphony, transients and spectral phase tradeoffs. https://www.audiolabs-erlangen.de/content/resources/MIR/00_PCD_AudioLabs/2016_DriedgerMueller_TSMOverview_AppliedSciences_ePrint.pdf
- Kraft et al., *Improved PVSOLA Time Stretching and Pitch Shifting for Polyphonic Audio* (2012). https://www.dafx.de/paper-archive/details/2P-TV49ZRjU-FPFzwQXTZw
- Raffel and Smith, *Practical Modeling of Bucket-Brigade Device Circuits* (2010): filters, companding and the distinction between a model and an approximation. https://dafx10.iem.at/proceedings/papers/RaffelSmith_DAFx10_P42.pdf
- Chowdhury, *Real-time Physical Modelling for Analog Tape Machines* (2019): transport, hysteresis and playhead-loss components. https://www.dafx.de/paper-archive/2019/DAFx2019_paper_3.pdf
- Strymon, *How does an oil can delay work?*: rotating charge medium, limited bandwidth, multiple heads and residual regeneration. Functional background only. https://www.strymon.net/faq/how-does-an-oil-can-delay-work/
- Abel et al., *Spectral Delay Filters with Feedback and Time-Varying Coefficients*: alternative continuous spectral-delay architecture. https://research.spa.aalto.fi/publications/papers/dafx09-sdf/
- Eventide H90 algorithm documentation, Reverse crossfades: functional comparison only. https://cdn.eventideaudio.com/manuals/h90/1.8.5/content/algorithms/delay.html
- Dal Santo et al., *Optimizing Tiny Colorless Feedback Delay Networks* (2025): feedback/input/output tuning and perceptual validation. https://link.springer.com/article/10.1186/s13636-025-00401-w
- Välimäki, Prawda and Schlecht, *Two-Stage Attenuation Filter for Artificial Reverberation* (2024): systematic frequency-dependent decay fitting. https://research.spa.aalto.fi/publications/papers/ieee-spl-two-stage/
- Dal Santo et al., *Shimmer Reverberation with Nonlinear Feedback Delay Networks* (2026): energy/decay/alias tradeoffs in pitched feedback. https://www.dafx.de/paper-archive/2026/papers/DAFx26_paper_05.pdf
- Parker, Zavalishin and Le Bivic, *Reducing the Aliasing of Nonlinear Waveshaping Using Continuous-Time Convolution* (2016): selective antialiasing experiments. https://www.dafx.de/paper-archive/details/vem_XXF5qBbfiWOH2RVVAA
