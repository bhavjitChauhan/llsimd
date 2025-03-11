#include <mmintrin.h>
#include <stdint.h>
#include <stdio.h>

int main() {
  __v2si m1 = {INT16_MAX, INT16_MIN};
  __v2si m2 = {1, -1};
  __v2si result = _mm_add_pi32(m1, m2);

  for (uint8_t i = 0; i < 2; ++i)
    printf("%d\n", result[i]);
}
