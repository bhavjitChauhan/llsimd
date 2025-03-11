#include <mmintrin.h>
#include <stdint.h>
#include <stdio.h>

int main() {
  __v1di m = {INT64_MIN};
  __v1di count = {1};
  __v1di result = _mm_srl_si64(m, count);

  printf("%lld\n", result[0]);
}
