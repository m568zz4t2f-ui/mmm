# SPDX-License-Identifier: AGPL-3.0-or-later
"""Offline NumPy analysis of /tmp IRs written by ir_audit.cpp (not a plug-in dependency)."""
import numpy as np

NAMES = ['Room', 'Plate', 'Hall', 'Bloom', 'Chamber', 'Cloud', 'Halo', 'Glint',
         'StudioPlate', 'StudioHall', 'StudioBloom']
print('model,nominal_decay_s,estimated_T20x3_s,fit_R2,IR_energy_gain_dB')
for index, name in enumerate(NAMES):
    audio = np.fromfile(f'/tmp/tide-audit-ir-{index}.raw', dtype=np.float32).reshape(-1, 2).astype(np.float64)
    energy = np.sum(audio * audio, axis=1)
    decay = np.cumsum(energy[::-1])[::-1]
    db = 10 * np.log10(np.maximum(decay, 1e-40) / max(decay[0], 1e-40))
    selected = (db <= -5) & (db >= -25)
    if np.count_nonzero(selected) < 2:
        raise ValueError(f'{name}: insufficient decay interval')
    times = np.flatnonzero(selected) / 48000
    fit = np.polyfit(times, db[selected], 1)
    residual = np.sum((np.polyval(fit, times) - db[selected]) ** 2)
    variance = np.sum((db[selected] - np.mean(db[selected])) ** 2)
    r2 = 1 - residual / variance
    gain = 10 * np.log10(energy.sum() / .02)
    print(f'{name},4,{-60 / fit[0]:.4f},{r2:.5f},{gain:.3f}')
