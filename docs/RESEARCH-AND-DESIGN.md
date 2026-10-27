# Wolf Tide — Research, scope and design decisions

Working name: Wolf Tide. The name has not undergone trademark clearance.

## Product brief

Stereo delay on the grey left; stereo reverb on the blue right. Independent presets/on-off and a lower combined browser. Core knobs stay put: time/feedback/tone/amount and decay/predelay/tone/amount. One contextual drawer exposes deeper editing. Vector echoes become ocean contours; reduced motion supported. No network audio processing or account.

## Primary references

- Strymon dTape white paper: https://www.strymon.net/strymon-dtape-technology-white-paper/ — variable tape speed vs sliding heads; mechanical pitch and magnetic coloration. Our model uses moving reads, smooth nonlinear coloration and sinusoidal modulation, not measured transport physics.
- Raffel & Smith, DAFx 2010: https://colinraffel.com/publications/dafx2010practical.pdf — BBD clocking, filters and companding. Our Bucket mode approximates coloration/bandwidth; a full circuit/compander model is not implemented.
- Strymon TimeLine MX: https://www.strymon.net/faq/what-happened-to-the-dual-pattern-trem-duck-and-swell-delay-engines/ — separate engine identity from shared dynamics/routing. Duck/Swell are broadly available; Trem specifically belongs to Filter.
- Eventide H90 routing/terminology: https://cdn.eventideaudio.com/manuals/h90/1.8.5/content/algorithms/delay.html — delay families and stereo behavior. We do not claim Eventide algorithms.
- Soundtoys EchoBoy: https://www.soundtoys.com/product/echoboy/ — focused main controls plus style/rhythm editing. Adopted stable core controls with a contextual deep editor.
- Baby Audio Transit 2: https://babyaud.io/docs/transit-2-manual — module presets and whole-chain presets, tails and smoothing. Adopted three explicit preset scopes.
- FabFilter Timeless: https://www.fabfilter.com/help/timeless/presets/sectionpresets — reusable section settings. https://www.fabfilter.com/help/timeless/using/modulationslots — contextual editing. Our first release exposes dedicated modulation, not an arbitrary modulation matrix.
- FabFilter Pro-R: https://www.fabfilter.com/help/pro-r/using/decayrateEQ — true frequency-dependent decay, distinct from post EQ. Not yet implemented in Tide; Bass Weight is input bass retention and labeled accordingly.
- JUCE state: https://docs.juce.com/master/classjuce_1_1AudioProcessorValueTreeState.html — host parameters, attachments, session recall.
- Apple distribution: https://developer.apple.com/documentation/security/notarizing-macos-software-before-distribution — Developer ID signing and notarization remain separate from compilation.

## Open-source projects evaluated

ChowMatrix (https://github.com/Chowdhury-DSP/ChowMatrix): read DelayProc.cpp, VariableDelay.cpp, Reverser.cpp and TempoSyncUtils.h during earlier research. Useful examples of feedback coloration, tempo divisions and reverse windows. BBDDelay (https://github.com/jatinchowdhury18/BBDDelay): experimental, not a production-ready dependency. AnalogTapeModel (https://github.com/jatinchowdhury18/AnalogTapeModel): magnetic processing, not a complete echo. Signalsmith Stretch (https://github.com/Signalsmith-Audio/signalsmith-stretch): polyphonic alternative with latency/CPU implications.

None of these DSP implementations were copied into Tide. JUCE 8.0.12 is the only bundled external framework. BloomDSP is reused and extended from the user's Wolf Bloom source. All other DSP is original. No commercial binary was disassembled; no hardware measurement or sonic-match claim is made.

## Implementation and tradeoffs

- Six delay modes share a stereo ring buffer and feedback architecture, with distinct read/processing behavior. Clean timing changes crossfade; Tape time changes bend pitch.
- Room and Plate use different feedback topologies. Hall/Bloom share an eight-line FDN foundation with different diffusion/voicing; these two are not wholly unrelated architectures.
- Reverb models remain allocated and running so changes let old tank energy decay. Fixed CPU use trades against bursty allocation or an unlimited stack of old engines. This is not complete preset spillover: changing parameters also changes old tank processing, and the single delay engine transitions its existing buffers.
- One dry path. Audible delay and reverb levels are separate from the stereo delay-to-reverb feed. Pitch delay and shimmer have independent interval settings.
- 43 automatable parameters. Side locks and mix lock apply to browsing; project recall restores actual state. Presets validate known finite parameters before applying.
- No real-time allocations except none in processing; explicit Clear zeroes already allocated buffers after a short fade. Prepare allocates outside processing. Host reset intentionally preserves tails; prepare clears them.
- No look-ahead/global latency; pitch windows create delayed wet content. No claimed zero-latency pitch shifting. No hidden automatic gain matching or output limiter.

## Deferred features

No full BBD circuit/compander simulation; no arbitrary modulation matrix; no external sidechain bus; no independently pitched shimmer voices; no reverse-grain editor; no frequency-dependent decay EQ; no preset favorites/search or undo history; no AAX/CLAP/Windows build in this delivery. Factory sounds require real source and DAW listening before calling them production-optimized.

## Replication research next

Capture wet-only reference impulses, bursts, sweeps, chords and several input levels. Compare first and successive repeats, modulation distributions, time-change behavior and stereo routing. Fit one behavior at a time and blind-test loudness-matched real performances. A static impulse response is insufficient for nonlinear/time-varying effects. AI can inspect code and fit candidate models, but reference recordings and listening provide evidence.

## 1.1.0 implementation update

The update keeps six delay families and four reverb families. It improves those implementations; it does not claim a new library of multiple physical/spectral algorithms per family.

- Routing-aware activity states suspend unused reverb tanks, muted unconnected returns and silent pitch paths. Outgoing reverbs retain tails. Echo inactivity uses a conservative 16.25-second silent horizon so Dual/Reverse contents are not mistaken for silence. Selected reverbs retain a 0.75-second quiet horizon, longer than their internal propagation gaps. Sleep detection uses a conservative 1e-9 sample threshold, not the UI meter. Retained allocations do not imply processing.
- Delays clear logically using 64-sample block generation stamps. Clear itself changes two scalar values; the first subsequent write to a block clears at most 64 floats. Reads validate stamps, including both interpolation neighbors. No buffer allocations occur in processing.
- Coefficients update on parameter/tempo changes; continuous smoothing remains. Reverse owns its writes only while selected; its transitions deliberately fade/restart that buffer. Delay-type changes retain the existing short wet dip rather than promising seamless parallel delay-engine spillover.
- Pitch has linked stereo phases, cubic interpolation, fourth-order pitch-dependent input filtering and longer windows at +19/+24. This reduces aliasing but does not remove granular smearing or produce formant-preserved spectral pitch. Upward anti-alias filtering necessarily removes input content which would exceed output Nyquist. Fixed Harmony keeps feedback unshifted; Climbing Repeats shifts the feedback. The pitch window remains wet latency, and climbing feedback still changes effective repeat timing. No zero-latency pitch claim is made.
- Room has explicit stereo early reflections. Plate uses distributed output taps and accounts for allpass lengths in its nominal loop-decay gain calculation; frequency-dependent group delay means this is still an approximation, not a measured physical plate.
- Hall has a stereo early-reflection bank. Hall/Bloom's embedded wet core omits duplicated outer predelay, bass filters, mix and width processing. Bloom has a distinct expanded delay geometry and longer diffusion, while retaining the shared efficient Hadamard FDN primitive.
- Tape separates a slow wow oscillator and normalized flutter oscillator. Bucket adds a second bandwidth-loss pole; neither is a newly validated physical tape/BBD circuit emulation. Multitap adds a fourth, stereo-dependent tap. Digital avoids redundant crossfade reads once timing settles; zero modulation skips delay trigonometry.
- Cached high-resolution printed artwork gives the interface its texture and contour lines. Dirty regions cover changing visuals/meters. Static, Activity and Motion modes affect UI only. Hidden/closed editor timers stop, and display-only meter accumulation is collected only with a visible editor. Host window occlusion support varies; full occlusion is not inferred from isShowing().

Primary research and possible future comparison implementations: Signalsmith Stretch (MIT), Rubber Band LiveShifter (GPL/commercial), DaisySP pitchshifter (MIT-style), and Dattorro/FDN/physical-plate literature listed above. No third-party DSP source was incorporated in 1.1.0. Spectral/formant pitch, physical BBD/tape, dispersive/modal physical plates and a broader per-family algorithm library remain future work requiring separate prototypes, benchmarks and listening tests.
