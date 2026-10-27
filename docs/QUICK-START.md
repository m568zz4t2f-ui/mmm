# Wolf Tide — Quick Start

Wolf Audio · Stereo delay + reverb · 1.1.0 development build

## Install

**Mac:** Double-click the supplied Wolf Tide `.pkg` and follow Apple's installer. It installs AU and VST3 system-wide and a standalone app. Restart the DAW. Ableton Live 10.1+ can use VST3; earlier Live 10 should use AU. A source archive or Linux build cannot be installed on macOS. Unsigned installers may require approval in macOS Privacy & Security; this build is not Developer ID signed or notarized.

**Linux x86_64:** Extract the Linux bundle and run `install-linux.sh`, or copy `Wolf Tide.vst3` into `~/.vst3`. Requires a compatible Linux host. This is not a Mac installer.

## Play

1. Choose a **Combined Preset** at the bottom: guitar, vocal, piano, keys or ambient.
2. Set the overall **Mix**. Start modestly on an insert. **Send** removes dry audio on a return/aux track and survives factory preset changes.
3. Grey controls the stereo delay; blue controls the stereo reverb. Both effects support stereo in/out and mono-to-stereo. Mono output folds the completed result down.
4. The top menus load each side's factory starting points independently. The character selectors change models without replacing all settings.
5. **ON** stops accepting new audio when turned off; existing tails finish. Both off returns dry audio on an insert, with the previous tails added until they fade. In Send mode both off stays wet-only.
6. **EDIT DELAY**, **EDIT REVERB** and **ROUTING** open one lower editor. Click again to close it.

## Save and compare

- `...` beside a side's preset menu saves or loads that side's `.wolftide` files.
- `SAVE / LOAD` at the bottom saves or loads a whole combination. You can share these files with other Wolf Tide users.
- `LOCK` protects a side when loading combined presets. `LOCK MIX` protects Mix and both Amount controls.
- Factory preset browsing keeps your Send setting, output trim and fallback tempo.
- DAW project recall restores the actual saved parameter state, regardless of browse locks.
- First click on `A / B` stores a reference. Subsequent clicks swap the two states. This is a comparison feature, not an undo history.

## Timing and space

`SYNC` follows host BPM when supplied. Otherwise use the fallback BPM in Routing. Turn Sync off to type milliseconds into Time. Main delay time is limited to 8 seconds; the secondary Dual tap supports up to 16 seconds. Dual uses the B/A timing ratio; Ping-pong crosses feedback between channels. A hard-panned input is preserved in Stereo mode; stereo reverb naturally distributes energy between channels.

`Delay to reverb` sends the stereo delay independently of the audible Delay Amount. `Direct to reverb` sends the original input. Parallel disables the delay feed. Bass Weight controls input bass retention below the reverb low-cut frequency; it is not frequency-dependent decay EQ.

Tape has a moving read head, saturation and wow/flutter. Bucket has time-dependent bandwidth and modulation; it is a character approximation, not a complete circuit/compander simulation. Reverse overlaps backward read windows. Pitch delay and Shimmer have independent interval selectors. They use a linked stereo time-domain granular shifter with cubic interpolation and pitch-dependent anti-alias filtering; chords can still have audible texture. In EDIT DELAY, Fixed Harmony shifts the return while keeping the feedback loop unshifted. Climbing Repeats shifts each pass again. The wet pitch window adds latency, including inside climbing feedback; nominal delay time does not include that window. Dry audio stays immediate.

Room adds explicit early reflections to parallel combs/allpasses and caps its nominal decay at 8 seconds. Plate uses a coupled allpass tank with distributed output taps and loop-length decay compensation. Hall combines early reflections and an eight-line modulated feedback network. Bloom has a different, expanded delay geometry and longer input diffusion. These are original models, not Strymon/Eventide clones.

`HOLD` removes new reverb input and holds tank feedback. Modulation/interpolation can slowly soften held audio. `CLEAR TAILS` briefly fades the complete output and logically clears buffers with bounded work. Stop retains tails only while the DAW keeps processing audio. Disabling the audio engine cannot play a tail.

`BLOOM` increases reverb decay and delay feedback in bounded ranges. The display button cycles STATIC, ACTIVITY (default), and MOTION. Activity animates each side while sound is present; Static keeps decorative artwork still while meters work. Hidden/closed editors stop their animation timer. READY means a side is enabled and waiting for audio; TAIL means it is off but stored sound remains. The visualizations illustrate settings and activity; they are not an oscilloscope or frequency analyzer.

## Levels and quality

Amount controls return gain; Mix blends the combined wet result with dry. They interact intentionally. Wide settings should be checked in mono. Amber status means output is above 0 dBFS: lower Output in Routing. No always-on limiter is hidden in the signal path. Freeze can retain sound indefinitely, so export extra tail time manually when needed.

Factory presets are designed and numerically tested starting points. They have not been optimized against your recordings or verified in your DAW. Ducking depends on incoming level; adjust Amount and the separate sensitivity/release controls for the source. Use local processors as needed before this ambience plugin; it does not include a vocal preamp, de-esser or amplifier model.
