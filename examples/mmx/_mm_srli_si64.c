#include <mmintrin.h>
#include <stdint.h>
#include <stdio.h>

int main() {
  __v1di m = {INT64_MIN};
  __v1di result = _mm_srli_si64(m, 1);

  printf("%lld\n", result[0]);
}
