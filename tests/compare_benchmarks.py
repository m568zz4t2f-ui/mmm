#!/usr/bin/env python3
"""Compare identically compiled benchmark.cpp against two source revisions."""
import json
from pathlib import Path
import statistics
import subprocess
import sys

before, after, output = sys.argv[1:]
records = {name: [[] for _ in range(8)] for name in ('before', 'after')}
for run in range(3):
    for name in (('before', 'after') if run % 2 == 0 else ('after', 'before')):
        result = subprocess.run([before if name == 'before' else after], check=True, capture_output=True, text=True)
        for line in result.stdout.splitlines():
            i, seconds = line.split(',')
            records[name][int(i)].append(float(seconds))
labels = ['Tape + Room', 'Tape + Plate', 'Tape + Hall', 'Tape + Bloom', 'Tape + Hall + Shimmer', 'Reverse + Hall', 'Pitch + Hall + Shimmer', 'Both effects off, silence']
rows = []
for i, label in enumerate(labels):
    a, b = (statistics.median(records[name][i]) for name in ('before', 'after'))
    rows.append(dict(scenario=label, before_seconds=a, after_seconds=b, reduction_percent=round(100*(1-b/a), 1)))
data = dict(method='48 kHz stereo, 3 seconds audio; median of three trials per invocation; three alternating before/after invocations; C++17 O3. DSP only, excluding GUI, allocation/preparation and DAW overhead.', results=rows)
path = Path(output)
path.parent.mkdir(parents=True, exist_ok=True)
path.write_text(json.dumps(data, indent=2)+'\n')
print(json.dumps(data, indent=2))
