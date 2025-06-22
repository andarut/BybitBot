<div align="center">

<picture>
  <source media="(prefers-color-scheme: light)" srcset="/docs/bybit_logo_light.svg">
  <img alt="liblena logo" src="/docs/bybitbot_logo_dark.svg" width="50%" height="50%">
</picture>

for simplified P2P trading on Bybit 

<h3>
</h3>

[![macos-x64](https://github.com/andarut/ByBitBot/actions/workflows/macos_x64.yml/badge.svg?branch=main&event=push)](https://github.com/andarut/liblena/actions/workflows/macos_x64.yml)
[![macos-arm](https://github.com/andarut/ByBitBot/actions/workflows/macos_arm.yml/badge.svg?branch=main&event=push)](https://github.com/andarut/liblena/actions/workflows/macos_arm.yml)
[![linux-x64](https://github.com/andarut/ByBitBot/actions/workflows/linux_x64.yml/badge.svg?branch=main&event=push)](https://github.com/andarut/liblena/actions/workflows/linux_x64.yml)

</div>

---

This Telegram bot can make your P2P trading life on Bybit easier.

Due to its extreme simplicity, it aims to be the perfect tool for P2P trader.

## Requirements

Currently library building with `conan` inside **virtualenv**, the only requirements are `Python (>= 3.6)` and C++ compiler.

**IMPORTANT! On Intel Macs you should use `clang` installed from `brew` (to enable support for AddressSanitizer)**

**IMPORTANT! Run with `source` command, so virtualenv can be kept opened.**
```
source ./scripts/prepare_<OS>_<ARCH>_<BUILD_TYPE>.sh
```

ExampleS:
```
source ./scripts/prepare_macos_x64.sh
```
```
source ./scripts/prepare_macos_arm.sh
```
```
source ./scripts/prepare_linux_x64.sh
```

This script will install dependencies and link installed `cmake` into `.venv/bin/cmake`.

## Configure
FIRST SEE [Requirements](Requirements) !

From now on you can build with **cmake configs**. Main is `conan-debug`.

```
cmake --preset 'conan-debug'
```

## Build
```
make -j$(nproc) -C build/Debug
```

On macos you can use `sysctl -n hw.ncpu` instead of `nproc`. `nproc` can be installed with `brew install coreutils`. 
```
make -j$(sysctl -n hw.ncpu) -C build/Debug
```
or
```
make -j$(sysctl -n hw.ncpu) -C build/Release
```
