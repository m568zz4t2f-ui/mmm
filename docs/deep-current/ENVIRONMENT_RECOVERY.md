# Environment and release recovery

The prior Codex environment was offline (`connectivity: offline`, command error `No usable execution environment is available`) and could not read the new ZIP. That was an infrastructure blocker, not a confirmed plugin defect. Do not repeat indefinitely in an unusable workspace; use a fresh working environment.

This repository was independently checked via the GitHub connector. The baseline branch `development/tide-ui-refresh` is at commit `464e0c3f7047c2a50198a824b1cbaf61caed4bee`. The previous 1.0 refresh's Linux VST3, eleven tests and deterministic audio equality passed; Deep Current has not been implemented or tested.

Original full-resolution 15-image ZIP: https://d2ol7oe51mr4n9.cloudfront.net/user_3JKUae4GIiCUFu3kbupkoLE3mRV/c2f843ec-eab7-4925-8832-f86915a52e11.zip

Wait for the separate design-reference import to populate `assets/tide/deep-current/previews/`. If it fails, use source URLs from `ASSET_MANIFEST.csv`; don't assume missing preview files mean Higgsfield sources are corrupt.

The existing source, installed Mac plugin version, AU validation, both architectures, DAW recall, graphics performance, installer, signing/notarization, conflict migration and rollback must be verified separately.
