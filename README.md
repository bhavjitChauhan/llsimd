# llsimd

Portable SIMD intrinsics through LLVM IR.

## Overview

The llsimd project enables program written in SIMD intrinsics to be run on any
CPU architecture. We do this by leveraging the target-independent nature of LLVM
IR’s first-class support of vector operations. This allows preexisting projects
to become more widely accessible and future-proof against the rise and decline
of specific architectures over time.

For more information, see [llsimd.pdf](llsimd.pdf).

## Example

```diff
-%result = call <8 x i16> @llvm.x86.sse2.pslli.w(<8 x i16> %m, i32 %count)
+%trunc = trunc i32 %count to i16
+%insert = insertelement <8 x i16> undef, i16 %trunc, i32 0
+%shuffle = shufflevector <8 x i16> %insert, <8 x i16> undef, <8 x i32> zeroinitializer
+%result = shl <8 x i16> %m, %shuffle
```

## Usage

### Requirements

- LLVM/Clang 20
- CMake 3.20+
- an x86 cross-compilation toolchain

On Debian/Ubuntu, these requirements can be fulfilled with:

```bash
wget https://apt.llvm.org/llvm.sh
chmod +x llvm.sh
sudo ./llvm.sh 20
sudo apt install cmake gcc-multilib-x86-64-linux-gnu
```

### Build

```bash
cmake -B build
cmake --build build
```

### Run

Execute the [llsimd.sh](llsimd.sh) script in the same directory as the built `libllsimd.so`:

```bash
cd build
../llsimd.sh ../in.c
```

## Development

### Get LLVM/Clang

#### Building from Source

Requirements:
- Git
- Ninja (optional)

> [!WARNING]
> Debug builds need 10-15 GB of disk space. Confirm that just enabling assertion
> checks or a `RelWithDebInfo` build doesn't cover your needs.

See the official
[Getting Started with the LLVM System](https://llvm.org/docs/GettingStarted.html)
tutorial for more information.

```bash
git clone -b llvmorg-20.1.0 --depth 1 https://github.com/llvm/llvm-project
cmake -S llvm -B build -G Ninja -DLLVM_ENABLE_PROJECTS=clang -DCMAKE_BUILD_TYPE=Release -DLLVM_ENABLE_ASSERTIONS=ON
cmake --build build
# Optionally install the build (to `/usr/local` by default)
sudo cmake --install build
```

#### Installing Pre-built Binaries

##### Debian/Ubuntu

Install the official APT packages:

```bash
wget https://apt.llvm.org/llvm.sh
chmod +x llvm.sh
sudo ./llvm.sh 20
```

##### Other

Download the binaries from LLVM's
[GitHub releases](https://github.com/llvm/llvm-project/tree/llvmorg-20.1.0).

### Test

#### Unit Tests

Requirements:

- an x86 machine
- Python 3
- [lit](https://pypi.org/project/lit/)

```bash
cmake --build build --target check
```

#### Manual

```bash
# C/C++ -> IR
clang -S -emit-llvm ../in.c
# Run pass on IR
opt -load-pass-plugin ./libllsimd.so -passes=llsimd -S in.ll -stats -o out.ll
# Interpret IR
lli in.ll
lli out.ll
```

To see pass statistics, LLVM needs to be compiled with assertion checks enabled.

The output assembly can be compared to the original:

```bash
# C/C++ -> assembly
clang -S ../in.c -o in.s
# IR -> assembly
llc out.ll
```
