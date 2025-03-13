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
opt -load-pass-plugin ./libpsimd.so -passes=psimd -S in.ll -stats -o out.ll
# Interpret IR
lli in.ll
lli out.ll
```

The output assembly can be compared to the original:

```bash
# C/C++ -> assembly
clang -S ../in.c -o in.s
# IR -> assembly
llc out.ll
```
