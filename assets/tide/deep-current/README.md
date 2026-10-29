# Design reference images — not release runtime graphics

The 15 Higgsfield source generations are indexed in `docs/deep-current/ASSET_MANIFEST.csv`. A branch-specific GitHub Action retrieves and scales them into `previews/` for convenient review. They are **not yet approved, cropped or engineered as JUCE assets**.

Static design artwork must be cleaned and curated for the release. Avoid embedding original multi-megabyte art in a binary and maintain efficient caching and motion.

The high-resolution original art ZIP remains on Higgsfield; do not commit the full 43MB archive to Git history.

The original, existing Tide UI and its controls always take priority over generated illustrative mockups.
