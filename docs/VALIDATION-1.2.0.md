# Tide 1.2.0 validation — 2026-10-27

Mac universal AU/VST3/Standalone build succeeded. All four CTest suites passed (DSP, update, models, host); native editor snapshots and 52 combined / 40 side preset exports succeeded. Host checks cover original parameter ordering and selector ranges, legacy state resets, same-rate preparation tails, side scopes, locks, versioned preset validation, UI/audio identity and searchable browser inventory. Local AU/VST3/Standalone installation completed with backups; auval -v aufx Wtid Wolf succeeded. See test-logs/mac-ctest-1.2.0.log and mac-auval-1.2.0.log.

Linux standalone tests also passed. A six-scenario legacy fixture at 8 kHz matched baseline sample for sample. New model impulse responses were checked at 8/48/96 kHz. Timing regression checks a request received during an active fade. The +24 alias check is a specific two-tone energy comparison, not a complete alias spectrum.

Performance CSVs report scenario, median, p99 and maximum microseconds per 128-frame block at 48 kHz on this Mac. Scenarios 0 idle, 1 digital-only, 2 tape-only, 3 legacy Hall plus digital, 4 Cloud-only (1.2) versus Hall plus digital (1.1; not a direct algorithm comparison), 5 pitch delay plus Hall. Short synthetic runs are subject to scheduling and thermal noise; they are not a DAW CPU meter or an editor-open/closed benchmark. Do not claim a blanket CPU reduction.

Known validation limits: no independent pluginval run for this release; no manual listening on the user's instrument recordings; no new real-DAW session/save/export test; no dedicated graphics profiling or 192-kHz new-model sweep. Universal architecture, payload and signing checks are recorded by the packaging process. Bundle signatures are ad hoc; installer is not Developer ID signed or notarized.

AU and VST3 architecture check: x86_64 arm64. Installer: ~/Downloads/Wolf-Tide-1.2.0-unsigned.pkg (42 MB). SHA-256: a4f18801e38b6bfea769db42120b15eae1cd7f20ce214c58b313a52cd1dff983.
