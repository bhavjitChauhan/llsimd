#include <mmintrin.h>
#include <stdint.h>
#include <stdio.h>

int main() {
  __v4hi m1 = {0, 1, 2, 3};
  __v4hi m2 = {INT8_MAX, INT8_MAX + 1, -1, UINT8_MAX + 1};
  __v8qu result = _mm_packs_pu16(m1, m2);

  for (uint8_t i = 0; i < 8; ++i)
    printf("%d\n", result[i]);
}
