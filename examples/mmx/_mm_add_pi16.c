#include <mmintrin.h>
#include <stdint.h>
#include <stdio.h>

int main() {
  __v4hi m1 = {0, 1, INT8_MAX, INT8_MIN};
  __v4hi m2 = {4, 5, 1, -1};
  __v4hi result = _mm_add_pi16(m1, m2);

  for (uint8_t i = 0; i < 4; ++i)
    printf("%d\n", result[i]);
}
