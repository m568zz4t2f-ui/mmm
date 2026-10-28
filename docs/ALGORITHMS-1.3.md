# Wolf Tide 1.3 algorithms and control guide

These are original time-domain designs. Shared delay lines, filters, diffusion and pitch utilities support different processing paths. They are not measured hardware replicas or proprietary algorithms. A numerical difference is not an audition score.

## Twelve delays

| Machine | Processing and purpose | Additional controls |
| --- | --- | --- |
| Digital | Cubic stereo taps; crossfaded time jumps retain timing without a moving-head pitch sweep. Clean rhythmic repeats. | Tone/low cut, stereo or dual time, ping-pong |
| Tape | Continuously moving read head, slow wow, flutter and filtered irregular drift, feedback saturation and age-dependent treble loss. | Age, drive, motion/rate |
| Bucket | Clocked/held companded delay with clock-dependent reconstruction bandwidth and nonlinear feedback. A BBD-inspired approximation, not circuit emulation. | Age, drive, time, tone |
| Multitap | Four independently timed and panned taps, with independent output levels and normalized feedback weights. | Four time ratios, levels, pans, feedback weights |
| Reverse | Two overlapping backwards readers per channel; crossfades between old/new phrase lengths. | Reverse attack |
| Pitch | Four linked stereo Hann-window readers with normalized weights and pitch-dependent anti-alias filtering. Fixed return harmony or climbing feedback. | Interval, pitch routing, texture/window |
| Diffusion | Adjustable cascaded allpass stages inside the repeat path, creating a diffuse echo rather than a separate reverb send. | Density/stage count, diffusion, texture/geometry |
| Granular Pitch | Twelve scheduled, overlapping voices reading an owned stereo capture history after the echo. Freeze stops the capture writer. | Interval, grain texture/density, position, harmony spread, freeze |
| Lo-Fi | Independent rate hold, quantization and bounded noise, followed by tone filtering; degradation recirculates. | Bit depth, reduction, noise |
| Oil Can | Modulated dispersive allpass readers, nonlinear recirculation and age-dependent warble geometry. | Age, drive, motion/rate |
| Spectral | Six bandpass-filtered tap times vary by frequency, with texture controlling frequency-dependent timing and resonance. **A frequency-band delay, not an FFT spectral editor.** | Texture, tone, feedback |
| Magnetic Drum | Four fixed head ratios (1/3, 1/2, 2/3, 1), magnetic-style saturation and age filtering. | Head levels/pans/feedback weights, age, drive |

Time is 20–8000 ms; multitap ratios and reverse readers can span 16 seconds. Only Dual stereo uses independent right timing. Pitch and granular effects add wet window latency; the dry signal remains immediate. Grain freeze captures audio that has already entered the delay, so allow the source to arrive first. Delay freeze is offered only for Granular Pitch.

## Thirteen reverbs

| Machine | Processing and purpose | Relevant deeper controls |
| --- | --- | --- |
| Room | Six early-reflection distances and four damped parallel combs, followed by output diffusion. | Early reflections, size, diffusion |
| Plate | Two coupled dispersive branches with orthogonal scattering and distributed return taps. | Diffusion, motion, size |
| Hall | Eight-line modulated Householder feedback network and explicit early reflections. | Early reflections, diffusion, bass damping |
| Bloom | Attack-dependent input diffusion feeding an eight-line late network. Gradual buildup. | Attack, diffusion, motion |
| Chamber | Tight early geometry and six cross-coupled combs with output diffusion. | Early reflections, size, diffusion |
| Cloud Ensemble | Sixteen longer modulated lines and separate six-stage output diffusion per channel. Dense evolving ambience. | Motion, size, diffusion, bass damping |
| Halo Shimmer | Twelve-line late network with dedicated four-stage input dispersion and filtered pitch-feedback/return layering. | Shimmer, interval, regeneration, pitch texture |
| Glint | Short reflection/pitch texture, three-stage input dispersion and twelve-line late network. | Shimmer, interval, regeneration, texture |
| Studio Plate | Four dispersive branches with an orthogonal Hadamard scattering matrix and separate branch diffusion. | Motion, diffusion, size |
| Studio Hall | Sixteen-line spatial late network, expanded geometry and early-reflection return. | Early reflections, bass damping, motion |
| Studio Bloom | Five-stage attack-dependent input dispersion and separate output buildup stages around a twelve-line network. | Attack, diffusion, motion |
| Nonlinear | Finite-window gated late field, with adjustable release and rising/swell envelope shape. | Attack/gate duration, gate/swell shape |
| Resonator | Eight damped combs tuned to harmonic ratios 1, 1.5, 2, 2.5, 3, 4, 5 and 6. | Fundamental tuning, size, tone, decay |

Decay is nominal: damping, size, allpass geometry, modulation, pitch feedback and gate shape affect the measured result. Size also scales Resonator tuning. Motion is disabled for non-modulated Room, Chamber and Resonator. Early reflections, pitch and envelope controls appear only where implemented. HOLD stops excitation and retains feedback; interpolation, filtering and modulation can dissipate held energy.

## Four slots and routing

Delay A and Reverb A use the familiar main controls. EDIT DELAY / EDIT REVERB opens a scrollable native panel. Its page menu selects Delay A, Delay B, Reverb A, Reverb B or Routing/global. B slots have independent model, timing/decay, filtering, modulation, width, return and bypass. Delay B has its own tempo sync/rhythm and pitch interval.

- **Series:** Delay A feeds Delay B; their returns feed Reverb A, then Reverb B. A disabled upstream slot passes fresh source to the next enabled slot; its outgoing tail still contributes to the return.
- **Parallel:** All enabled slots receive the source independently. Delay-to-reverb feed is disabled.
- **Frequency split:** Complementary low/high source bands feed A/B. Each delay feeds its corresponding reverb. Split frequency controls the crossover.

Cross feedback exchanges the previous sample's **uncalibrated** return between the two delays and between the two reverbs, up to 30%. It reduces internal regeneration and bounds the exchanged signal with a nonlinearity. Return calibration is outside these loops. Width above 100% normalizes the widened result to avoid increasing the operator's peak gain. There is one master dry path.

Configuration changes ramp routing weights and return levels. Each slot retains its current machine and one outgoing tail. A third rapid selection retires the oldest voice over 40 ms; unlimited tails are intentionally not retained. The first transient of a previously silent slot is preserved. Bypass fades fresh excitation over 10 ms and leaves stored tails running.

## Compatibility and presets

The first 61 parameter IDs, normalized ranges and ordering are preserved. New controls are appended. Schema-1/2/3 sessions and presets without the new controls default each section to Legacy, with B off. For sessions where both sections are Legacy, the original complete 1.2.2 processing path runs. Its default arithmetic is unchanged; a small optional stem interface allows one legacy section to coexist with the new router.

Each side has a visible Legacy / Tide 1.3 processing choice. Upgrading a delay preset changes its delay revision and controls, leaving the reverb revision intact. A section lock protects both A/B slots and its revision. Routing lock covers the new routing, split and cross-feedback controls; mix lock also protects B returns. New routing uses only the A slot for a legacy section; B becomes available when that section is upgraded. The frequency split applies to the new machines; a legacy reverb retains its original full-band tank path.

Factory programs 0–15 are explicit compatibility sounds. Existing program names/indices 16–51 remain stable and now contain revised 1.3 recipes. Programs 52–63 add the new families and four-slot combinations. Old projects restore their saved parameters rather than rebuilding from the current factory recipe. There are 24 delay and 26 reverb starters. Factory/User/Modified identity, independent names, favorites, search, undo-load and A/B comparison remain supported.

## Levels and resource budget

Fixed per-model return trims are derived from the two documented recording fixtures at the stated settings, with separate nominal targets for delay and reverb. They do not estimate perceived loudness from your performance. They preserve dynamics and stay outside internal/cross-feedback loops. Transients, very different sources and long/frozen tails still require output monitoring. Wet-active output has a soft emergency ceiling; transparent mix-zero and inactive dry paths retain the output trim and may exceed 0 dBFS when boosted.

All four slots and their transition voices are prepared before processing, along with the compatibility engine. Disabled slots sleep; memory remains allocated so automation cannot allocate on the audio callback. The measured modern-engine heap payload is approximately 41.5 MiB at 48 kHz and 165.9 MiB at 192 kHz, **plus** the legacy engine and host/JUCE overhead. Plan high-rate multitrack sessions accordingly. CPU measurements in the release evidence are Linux measurements, not Mac deadline guarantees.
