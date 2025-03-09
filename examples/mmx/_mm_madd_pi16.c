#include <mmintrin.h>
#include <stdint.h>
#include <stdio.h>

int main() {
  __v4hi m1 = {0, 1, 2, 3};
  __v4hi m2 = {4, 5, 6, 7};
  __v2si result = _mm_madd_pi16(m1, m2);

  for (uint8_t i = 0; i < 2; ++i)
    printf("%d\n", result[i]);
}
