#include <mmintrin.h>
#include <stdio.h>

int main() {
  unsigned char a[8] = {10, 20, 30, 40, 50, 60, 70, 80};
  unsigned char b[8] = {1, 2, 3, 4, 5, 6, 7, 8};
  unsigned char result[8];

  __m64 mmx_a = _mm_set_pi8(a[7], a[6], a[5], a[4], a[3], a[2], a[1], a[0]);
  __m64 mmx_b = _mm_set_pi8(b[7], b[6], b[5], b[4], b[3], b[2], b[1], b[0]);
  __m64 mmx_result = _mm_add_pi8(mmx_a, mmx_b);

  *(__m64 *)result = mmx_result;

  printf("Result: ");
  for (int i = 0; i < 8; i++) {
    printf("%d ", result[i]);
  }
  printf("\n");

  _mm_empty();

  return 0;
}
