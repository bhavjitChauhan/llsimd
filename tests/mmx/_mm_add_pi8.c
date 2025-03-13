// RUN: clang -S -emit-llvm %s -o %t.ll
// RUN: opt -load-pass-plugin %root/build/libpsimd.so -passes=psimd -S %t.ll -o %t.out.ll
// RUN: diff <(lli %t.ll) <(lli %t.out.ll)

#include <mmintrin.h>
#include <stdint.h>
#include <stdio.h>

int main() {
  __v8qi m1 = {0, 1, 2, 3, 4, 5, 6, 7};
  __v8qi m2 = {8, 9, 10, 11, 12, 13, 14, 15};
  __v8qi result = _mm_add_pi8(m1, m2);

  for (uint8_t i = 0; i < 8; ++i)
    printf("%d\n", result[i]);
}
