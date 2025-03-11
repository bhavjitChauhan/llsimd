#include <mmintrin.h>
#include <stdint.h>
#include <stdio.h>

int main() {
  __v2si m1 = {0, 1};
  __v2si m2 = {INT16_MAX, INT16_MAX + 1};
  __v4hi result = _mm_packs_pi32(m1, m2);

  for (uint8_t i = 0; i < 4; ++i)
    printf("%d\n", result[i]);
}
