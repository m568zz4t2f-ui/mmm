#!/usr/bin/env python3
"""Standalone DSP checks; no JUCE, network, or plugin installation required."""
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path
import os
import subprocess

root = Path(__file__).resolve().parents[1]
out = root / 'build-processing-sweep'
logs = root / 'docs' / 'processing-sweep'
out.mkdir(exist_ok=True)
logs.mkdir(parents=True, exist_ok=True)
names = ['dsp_tests', 'models_tests', 'repair_tests', 'update_tests',
         'modern_tests', 'modern_controls', 'modern_stress', 'processing_sweep_tests']

def run(name):
    exe = out / name
    subprocess.run(['g++', '-O2', '-std=c++17', '-I', str(root / 'Source'),
                    str(root / 'tests' / (name + '.cpp')), '-o', str(exe)], check=True)
    result = subprocess.run([str(exe)], check=True, capture_output=True, text=True)
    (logs / (name + '.log')).write_text(result.stdout + result.stderr)
    return name + ': PASS'

with ThreadPoolExecutor(max_workers=4) as pool:
    results = list(pool.map(run, names))

exe = out / 'sanitized'
subprocess.run(['g++', '-O1', '-g', '-std=c++17', '-fsanitize=address,undefined',
                '-fno-omit-frame-pointer', '-I', str(root / 'Source'),
                str(root / 'tests' / 'processing_sweep_tests.cpp'), '-o', str(exe)], check=True)
env = dict(os.environ, ASAN_OPTIONS='detect_leaks=0')
result = subprocess.run([str(exe)], env=env, check=True, capture_output=True, text=True)
(logs / 'sanitizer.log').write_text(result.stdout + result.stderr)
results += ['AddressSanitizer/UndefinedBehaviorSanitizer: PASS',
            'LeakSanitizer disabled: unavailable in the managed process environment.',
            'Mac AU/VST3 host, musical audition and installer validation: NOT RUN.']
(logs / 'validation.log').write_text('\n'.join(results) + '\n')
print('\n'.join(results))
