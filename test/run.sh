#!/bin/sh
set -e

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
CC="${CC:-cc}"
PYTHON="${PYTHON:-python3}"
CFLAGS="-std=c99 -Wall -Wextra -Werror -I$ROOT/include"
OUT="$ROOT/build/host"

mkdir -p "$OUT"

$CC $CFLAGS "$ROOT/test/test_globe.c" "$ROOT/src/globe.c" "$ROOT/src/fxtab.c" -o "$OUT/test_globe"
"$OUT/test_globe"

"$PYTHON" "$ROOT/tools/check_zx128_layout.py" "$ROOT/test/fixtures/zx128-safe.map"

echo "ALL HOST TESTS PASSED"
