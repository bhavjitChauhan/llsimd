// RUN: clang -S -emit-llvm %s -o %t.ll
// RUN: opt -load-pass-plugin %root/build/libllsimd.so -passes=llsimd -S %t.ll -o %t.out.ll
// RUN: diff <(lli %t.ll) <(lli %t.out.ll)

#include <mmintrin.h>
#include <stdint.h>
#include <stdio.h>

int main() {
  __v1di m = {INT64_MIN};
  __v1di count = {1};
  __v1di result = _mm_srl_si64(m, count);

  printf("%lld\n", result[0]);
}
