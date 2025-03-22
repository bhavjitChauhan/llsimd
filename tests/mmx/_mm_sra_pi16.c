// RUN: clang -S -emit-llvm %s -o %t.ll
// RUN: opt -load-pass-plugin %root/build/libllsimd.so -passes=llsimd -S %t.ll -o %t.out.ll
// RUN: diff <(lli %t.ll) <(lli %t.out.ll)

#include <mmintrin.h>
#include <stdint.h>
#include <stdio.h>

int main() {
  __v4hi m = {0, -1, INT16_MAX, INT16_MIN};
  __v1di count = {2};
  __v4hi result = _mm_sra_pi16(m, count);

  for (uint8_t i = 0; i < 4; ++i)
    printf("%d\n", result[i]);
}
