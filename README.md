# psimd (Working Name)

## Usage

Requirements:
- LLVM/Clang 20
- CMake 3.20+

Recommended:
- Linux

### Build

```bash
cd build
cmake ..
cmake --build . --target psimd
```

### Run

```bash
clang -fpass-plugin=./libpsimd.so ../in.c
```

To see pass statistics, LLVM needs to be compiled with assertion checks enabled.

## Development

### Get LLVM/Clang

#### Building from Source

Requirements:
- Git

Recommended:
- Ninja

> [!WARNING]
> Debug builds need 10-15 GB of disk space. Confirm that just enabling assertion
> checks or a `RelWithDebInfo` build don't cover your needs.

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

Alternatively, install the latest pre-built binaries:

```bash
bash -c "$(wget -O - https://apt.llvm.org/llvm.sh)"
```

##### Other

https://github.com/llvm/llvm-project/releases/latest

### Test

```bash
clang -S -emit-llvm ../in.c -o in.ll
opt -load-pass-plugin ./libpsimd.so -passes=psimd -S in.ll -stats -o out.ll
llc out.ll -o out.s
clang out.s -o out
```

The output assembly can be compared to the original:

```bash
clang -S ../in.c -o in.s
```
