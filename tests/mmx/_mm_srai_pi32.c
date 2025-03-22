// RUN: clang -S -emit-llvm %s -o %t.ll
// RUN: opt -load-pass-plugin %root/build/libllsimd.so -passes=llsimd -S %t.ll -o %t.out.ll
// RUN: diff <(lli %t.ll) <(lli %t.out.ll)

#include <mmintrin.h>
#include <stdint.h>
#include <stdio.h>

int main() {
  __v2si m = {INT32_MAX, INT32_MIN};
  __v2si result = _mm_srai_pi32(m, 1);

  for (uint8_t i = 0; i < 2; ++i)
    printf("%d\n", result[i]);
}
