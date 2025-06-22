# BybitBot

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
