#!/bin/bash
set -euo pipefail
cd "$(dirname "$0")/.."
cmake -S . -B build-mac -DCMAKE_BUILD_TYPE=Release '-DCMAKE_OSX_ARCHITECTURES=arm64;x86_64' -DCMAKE_OSX_DEPLOYMENT_TARGET=11.0
cmake --build build-mac -j 3
(cd build-mac && ctest --output-on-failure)
bash scripts/package-mac.sh
