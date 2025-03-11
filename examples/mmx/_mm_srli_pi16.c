#include <mmintrin.h>
#include <stdint.h>
#include <stdio.h>

int main() {
  __v4hi m = {0, -1, INT16_MAX, INT16_MIN};
  __v4hi result = _mm_srli_pi16(m, 2);

  for (uint8_t i = 0; i < 4; ++i)
    printf("%d\n", result[i]);
}
