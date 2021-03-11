#!/usr/bin/env bash
set -euo pipefail

for src in `ls lib/**/*.cpp include/**/*.h`; do
    echo "formatting $src"
    clang-tidy-10 $src -fix -checks="readability-braces-around-statements"  -p  Debug-build
done
