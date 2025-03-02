# psimd

Requirements:
- GCC/Clang
- LLVM 19+

```bash
cd build
cmake ..
cmake --build . --target psimd
clang -S -emit-llvm ../in.c -o in.ll
opt -load-pass-plugin ./libpsimd.so -passes=psimd -S in.ll -stats -o out.ll
```

To see pass statistics, LLVM needs to be compiled with assertion checks enabled (`-DLLVM_ENABLE_ASSERTIONS=ON`).
