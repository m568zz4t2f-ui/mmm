# Tide backup index

October 27, 2026. This backup consolidates the existing 1.2 source/audit history and newly recovered Mac project material on the default branch. It does not change DSP, install a new build or fix the audit findings.

## Contents

| Material | Location |
| --- | --- |
| Current 1.2 source and preset definitions | [Source](../Source) |
| 52 combined and 40 side preset files | [Factory-Presets](../Factory-Presets) |
| Four native editor screenshots | [preview](../preview) |
| Full 20-effect evaluation and reference projects | [effect audit](TIDE-1.2.0-EFFECT-AUDIT.md) |
| Audit programs and analysis scripts | [tests](../tests) |
| Measured render matrices, stability/memory/IR results | [test logs](test-logs) |
| Recovered Mac build, packaging, install and AU-validation evidence | [test logs](test-logs) |
| Release state and validation limits | [release](RELEASE-1.2.0.md), [validation](VALIDATION-1.2.0.md) |
| Prior design, implementation plan and primary references | [research](RESEARCH-AND-DESIGN.md), [specification](TIDE-1.2-UPDATE-SPEC.md), [handoff](TIDE-1.2-GO-HANDOFF.md) |
| Current durable project handoff | [project memory](PROJECT-MEMORY.md) |
| File identities and local binary artifacts | [artifact manifest](ARTIFACT-MANIFEST-2026-10-27.json) |

Recovered logs replace user-home/project absolute paths with $HOME/$PROJECT. Historical failure logs are retained as evidence, not counted as new failures or passing validations. Source identity was checked against the Mac's Source/tests/scripts files; there were no mismatches across 21 checked files.

## Preserved history

- Last 1.1/planning default-branch commit: d38085d9136d98b1995963342c547cf9973ad838.
- Installed 1.2 code baseline: bfe1552ac3ca6f549860aa56e141cd25779bd5ad.
- Release/validation branch: [tide-1.2.0](https://github.com/m568zz4t2f-ui/mmm/tree/tide-1.2.0), final release-baseline commit e72b71042d57e22a334ef17379ec1b9030ab1896.
- Audit branch: [tide-1.2-audit](https://github.com/m568zz4t2f-ui/mmm/tree/tide-1.2-audit), commit 72d3241648fe230a14082f825bc0fc1231584d9e.
- This backup is a descendant of that audit. Previous releases/audit branches are retained.

## Large artifacts remaining on the build Mac

The connected GitHub tools do not expose release-asset upload. This backup uploads source/text and the four small preview binaries, and records the larger artifacts for separate release distribution:

- Installer: ~/Downloads/Wolf-Tide-1.2.0-unsigned.pkg and ~/Downloads/WolfTide-1.2.0/dist/Wolf-Tide-1.2.0-unsigned.pkg, 43,887,533 bytes. SHA-256 a4f18801e38b6bfea769db42120b15eae1cd7f20ce214c58b313a52cd1dff983. Ad hoc bundles; installer is not Developer ID signed/notarized.
- Vendored JUCE archive: ~/Downloads/WolfTide-1.2.0/vendor/JUCE-8.0.12.tar.gz, 22,808,845 bytes. SHA-256 91505bee78c343c37d757f03d87c50e494aff85d41811f25ab389698781704e6. CMake can instead fetch the exact pinned upstream commit.
- Build products, intermediate objects and installation rollback copies remain on the Mac. They are not confused with committed source or a downloadable GitHub installer.

## Graphify

Repository: m568zz4t2f-ui/mmm; ID 3f47f366-4027-44d4-8dd4-05d8e143a84c. Relevant release facts, measured failures, compatibility constraints, research candidates and backup references are saved through durable intake. Intake is not publication: extracted memories need acceptance in Memory > Needs review.

Graphify initially indexed d38085d, before the 1.2 release. Promoting the consolidated history to main makes the updated files available to the integration; indexing is asynchronous and its commit must still be verified. GitHub remains authoritative when the graph is stale.
