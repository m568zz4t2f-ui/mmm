# Wolf Prism 0.1.0

A native macOS AU/VST3 and standalone multi-effect inspired by the public Chroma Console manual and the supplied interface photograph. This is an independent first playable implementation, not Hologram firmware, an official Hologram product, or a verified sonic clone. All twenty modes contain working DSP. The detailed comparison in docs/REFERENCE-COMPARISON.md identifies behavior that differs from the hardware.

## Install and load

Run Wolf-Prism-0.1.0-unsigned.pkg. It installs:

- /Library/Audio/Plug-Ins/Components/Wolf Prism.component
- /Library/Audio/Plug-Ins/VST3/Wolf Prism.vst3
- /Applications/Wolf Audio/Wolf Prism.app
- /Library/Application Support/Wolf Audio/Wolf Prism/Source.zip and this guide

The bundles contain Intel x86_64 and Apple Silicon arm64 slices and target macOS 11 or newer. Apple Silicon is cross-compiled, not runtime-tested. The AU uses aumf (MusicEffect) because it accepts capture MIDI. VST3 is the recommended Ableton format. In Live, enable VST3 system folders, rescan, and drag Wolf Prism from Plug-Ins onto an audio track. AU can be enabled separately. Route audio into the standalone app through its audio settings; begin with output level low enough for your interface.

The installer has no Developer ID signature and is not notarized. Bundles have local ad-hoc signatures only. macOS may require approval through its normal Privacy & Security interface. Do not disable system-wide security. Installer upgrades back up existing Wolf Prism bundles under /Library/Application Support/Wolf Audio/Wolf Prism/Backups. The first installation does not modify Wolf Tide or Wolf Cosmos.

## Play

The four columns are Character (orange), Movement (yellow), Diffusion (green), and Texture (blue). Each ON button bypasses that module. Click an effect name to select it. Top knobs are Tilt, Rate, Time, Mix. Bottom knobs are the four module Amount controls. Secondary switches those same knobs to sensitivity, two drift controls, output dB and the four module volume controls. Double-click resets a knob; Shift-drag gives precision; values can be typed.

Mix 0 returns unprocessed dry stereo at unity output; Mix 1 uses the module chain. Individual delays deliberately include direct sound within their chain output. For a true reverb send, choose Space, Amount 1 and global Mix 1. Reverse at Amount 1 and Pitch at Movement Amount 1 remove their module's direct input. The plugin reports zero added dry-path latency. Pitch uses two windowed delay heads with up to about 60 ms wet delay; reverse and other time effects have intentional additional delay. This is not zero latency throughout the effect.

Factory browser contains 16 starting points grouped by instrument/purpose. They are starting settings, not loudness-matched or listening-validated commercial presets. FX Setup exposes all 24 effect orders (C=Character, M=Movement, D=Diffusion, T=Texture), the filter's Tilt/Low-pass/High-pass styles, tempo, capture controls, and 80 user memories. Texture Filter's center is neutral in Tilt mode. Dual Bypass disables the chosen mask: Character=1, Movement=2, Diffusion=4, Texture=8; add values for multiple modules. Global Bypass always bypasses everything.

Tempo Sync quantizes timing/modulation to musical subdivisions. Host Tempo additionally follows the DAW's BPM. Tap sets manual BPM, enables tempo sync and disables Host Tempo so taps take effect immediately. Turn Host Tempo back on in FX Setup to follow the DAW. There is no MIDI clock generation, hardware DIN/USB forwarding, or physical expression calibration; host automation replaces physical expression.

## Gesture

Click GESTURE REC, move primary knobs, then click again to loop the movement. All eight primary values are sampled at 100 Hz for up to 30 seconds. Gesture Playback suspends/resumes the recorded movement; Clear Gesture deletes it. The loop speeds up/down relative to the recorded tempo. Unlike the pedal, this version records one shared eight-knob loop rather than separate independent per-knob cycles. Knob readouts show base controls, not the instantaneous Gesture modulation. Disable Gesture Playback for direct manual editing.

## Capture

FX Setup: Record replaces the current capture; Play finishes recording and starts playback. Maximum 30 seconds. Stop preserves audio. Erase deletes it. Dub toggles overdub. Undo removes all overdub layers since the original capture, preserving that original layer; it is not a multi-step undo history. Reverse and Half/Normal/Double speed change playback. Pre-FX routes capture through the effects; otherwise it records and loops the chain output. Capture Level sets the loop level. Live input continues alongside playback. There is no quantized capture in this version.

A short overlapping boundary crossfade (up to 8 ms, or one eighth of the loop) reduces boundary clicks; the overlap shortens playback period by that crossfade length. Capture duration displays recorded duration. Rate changes reinitialize running buffers; session recall resamples saved capture audio to the current rate and restores it stopped. Press Play to resume.

## Presets and saving

Save User stores all parameters and Gesture in the selected one of 80 slots; Load User recalls it. Capture audio is excluded from user slots, matching the reference's ephemeral capture policy. DAW sessions and exported .wolfprism banks include the current capture plus the full user bank. A bank includes all 80 used memories and may approach 55 MB at 192 kHz with a full 30-second capture. Restore validates version, parameter completeness, numeric ranges, loop duration, Gesture bounds and file length before committing settings.

Save, recall, and export take a consistent snapshot by briefly suspending this processor and acquiring its callback lock outside processing. These operations can interrupt audio; avoid them during critical recording. A future background snapshot design is required for uninterrupted large-loop saving. Core processing preallocates audio storage and performs no application file I/O or heap allocation. The callback FIFO is single-producer for editor commands; external MIDI capture commands execute directly at sample offsets.

MIDI CC 22 >=64 begins capture, CC 22 <64 finishes/plays, CC 23 >=64 stops, CC 24 >=64 erases. These are Wolf Prism assignments, not a full reproduction of the Hologram MIDI chart. Other knobs are available as DAW automation parameters. Map MIDI to these parameters in the host when desired. MIDI channel filtering/program change/MIDI thru are not implemented.

Ordinary host reset and same-rate prepare calls preserve tails; a changed sample rate reinitializes DSP. Global Bypass can preserve diffusion tails or smoothly return to dry without trails. Transport tails continue only while the host calls the plugin. Reported finite tail is 240 seconds; capture playback is indefinite and requires an explicit export range. Plugin host bypass and its own Bypass share a policy. There is no electrical relay true-bypass in software.

## Build, licenses, and recovery

Complete project source, tests, packaging scripts and pinned JUCE 8.0.12 archive are in Source.zip. Install CMake 3.22+ and Xcode command-line tools, extract source, then run scripts/build-mac.command. CMake's vendored source route works offline; the pinned upstream commit is 29396c22c93392d6738e021b83196283d6e4d850. Only the independent Wolf Bloom FDN core and build conventions were reused from the user's Wolf Cosmos project. No proprietary DSP code, firmware or product artwork is included.

AGPL-3.0-or-later, with corresponding source and JUCE's included license/third-party notices. Brand and effect names identify the reference and belong to their respective owners; no affiliation or endorsement. Source archive retains all dependency notices.

Uninstall the three Wolf Prism bundles at the paths above; export your bank first. Keep Application Support if you want source/backups. Roll back by replacing each bundle with its corresponding backed-up bundle. Restart/rescan the host after changing an installed bundle.

See docs/VALIDATION.md for what was actually tested and what remains untested. No claim of sonic superiority, hardware equivalence, broad DAW compatibility, zero wet latency, or Apple Silicon runtime validation is made.
