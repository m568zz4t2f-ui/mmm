# Curated Deep Current materials

Rebuild with `python scripts/prepare-deep-current.py` (Pillow, preparation only).
The inputs are the supplied repository's 1200-pixel previews. They are not the
2048/2688-pixel originals described by the historical design manifest.

Only these five 256×256 textures enter BinaryData. Material patches exclude baked
controls and lettering. The knob's painted needle is repaired with adjacent
brushed material; its speckled outer alpha is replaced with a clean ellipse.
JUCE draws the live pointer and value arc. Lighting stays fixed as values change.
See manifest.json for input hashes, crop rectangles, actual sizes and byte costs.

The same knob face serves every rotary size. Existing bounded physical-size
caching remains in use. Static chassis, panel and glass composition happens only
when the existing backdrop cache is rebuilt. No resources enter the audio thread.
The system sans family requires no redistributed font. Existing Wolf/Tide brand
text remains code-drawn and is not extracted from the generated mockup.
