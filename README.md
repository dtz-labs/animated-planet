# Animated Planet

[![CI](https://github.com/dtz-labs/animated-planet/actions/workflows/ci.yml/badge.svg)](https://github.com/dtz-labs/animated-planet/actions/workflows/ci.yml)

A rotating wireframe planet for Timex TC2048/TC2068/TS2068 and ZX Spectrum 128K,
built with z88dk.

The demo draws a 50 Hz double-buffered globe using fixed-point tables only. It
targets two screen-flip implementations:

The Timex build uses the SCLD display files at `0x4000` and `0x6000`, flipping
with `OUT (0xFF),0/1`. The ZX128 build uses the shadow screen in RAM page 7,
kept mapped at `0xC000`, and flips with bit 3 of port `0x7FFD`.

## Installation

Install a C compiler for the host tests. To build TAP images, install z88dk and
either keep it next to this checkout as `../z88dk` or set `Z88DK_HOME`:

```sh
export Z88DK_HOME=/path/to/z88dk
```

The Makefile automatically adds `$Z88DK_HOME/bin` to `PATH` and configures
`ZCCCFG` when it finds the local z88dk checkout.

## Usage

```sh
./build.sh
make timex   # build/planet-timex.tap
make zx128   # build/planet-zx128.tap
make test    # host-side fixed-point/projection tests
```

Run helpers are included for ZEsarUX:

```sh
make run-tc2048
make run-tc2068
make run-zx128
```

The renderer draws the front-facing meridian and parallel points into the hidden
page, erases only bytes touched the last time that page was hidden, and flips at
50 Hz. The host tests compile the pure globe projection code with the system C
compiler, so CI can validate the fixed-point geometry without a Spectrum
toolchain.

## License

MIT. See [LICENSE](LICENSE).
