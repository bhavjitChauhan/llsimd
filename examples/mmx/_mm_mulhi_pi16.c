#include <mmintrin.h>
#include <stdint.h>
#include <stdio.h>

int main() {
  __v4hi m1 = {INT16_MAX, INT16_MAX, INT16_MAX, INT16_MAX};
  __v4hi m2 = {2, 3, INT16_MAX / 2, INT16_MAX};
  __v4hi result = _mm_mulhi_pi16(m1, m2);

  for (uint8_t i = 0; i < 4; ++i)
    printf("%d\n", result[i]);
}
