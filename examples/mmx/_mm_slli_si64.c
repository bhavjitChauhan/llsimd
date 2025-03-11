#include <mmintrin.h>
#include <stdint.h>
#include <stdio.h>

int main() {
  __v1di m = {INT32_MAX};
  __v1di result = _mm_slli_si64(m, 1);

  printf("%lld\n", result[0]);
}
