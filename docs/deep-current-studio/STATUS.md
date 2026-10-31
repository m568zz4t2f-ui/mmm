# Studio Edition — implementation and material corrections in progress

Branch: `ui/deep-current-complete`. Continued from recovery `46b47a9` without reset.
Original frozen processing baseline remains `e2a6ad6`, Tide 1.3.0.

Implemented shared Higgsfield skins throughout the main editor, independent/legacy
views, browser scopes, buttons, fields, sliders, popup menus, tooltips, callout shells
and plugin-owned alerts. Functional text uses licensed embedded IBM Plex Sans;
original italic Tide identity remains unchanged. Native controls, accurate pointers,
focus and live visualizations retain their behavior. All 214 protected files and
14 editor-method invariants pass (two callout methods add only a shared visual theme).

The first native build succeeded with build ID `6b028fb82c51`. All 12 CTest suites
passed, plus two recovery guard tests. The 64-program baseline/candidate audio
fixture is byte-identical. These are intermediate results, not final artwork QA.

Actual-size review found the recovered large surfaces too bright in text regions.
Four specifically quoted Higgsfield correction requests produced three completed
images and one failed chassis attempt. The bounded chassis retry succeeded.
Prompts, quotes, inputs, statuses and result URLs are in correction-jobs.json.
No recovered original was replaced or modified; the 29 originals remain immutable.

Next: import completed correction bytes using the existing GitHub Actions path,
inspect them, record approved crops/exports, replace only the affected runtime
surfaces, then rerun final build/tests/audio/screenshots and matched normal/forced
GUI benchmarks. Mac AU/VST3/Ableton/Retina/installer checks remain pending on a Mac.
Native OS file pickers retain platform styling. This is not a finished release.
