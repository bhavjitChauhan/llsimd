#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>

int main() {
  __v4hi m1 = {0, 1, 2, 3};
  __v4hi m2 = {INT8_MAX, INT8_MAX + 1, INT8_MIN, INT8_MIN - 1};
  __v8qi result = _mm_packs_pi16(m1, m2);

  for (uint8_t i = 0; i < 8; ++i)
    printf("%d\n", result[i]);
}
