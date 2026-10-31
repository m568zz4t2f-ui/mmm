"""Exact 1.3.1 processing authorization; retain the historical 1.3.0 freeze."""

# User-authorized processing sweep merged with the completed Studio interface.
# This is a fixed replacement, never a wildcard exemption for DSP edits.
MODERN_BASE = '5102de63f25384e240ca463534aec63732c3d9df391164d3245a94ea35522e77'
MODERN_131 = 'e5106dceac1faec3b667840aeb3a5ce5dab762de52484cc8ed0d20c4bfade024'


def effective_hashes(frozen):
    expected = dict(frozen)
    if 'Source/ModernDSP.h' in expected:
        if expected['Source/ModernDSP.h'] != MODERN_BASE:
            raise ValueError('Unexpected historical ModernDSP baseline')
        expected['Source/ModernDSP.h'] = MODERN_131
    return expected
