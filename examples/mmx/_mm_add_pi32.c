#include <mmintrin.h>
#include <stdio.h>
#include <stdint.h>

int main() {
    __m64 a = _mm_set_pi32(20, 10);
    __m64 b = _mm_set_pi32(40, 30);

    __m64 result = _mm_add_pi32(a, b);

    int32_t *res = (int32_t *)&result;
    printf("Result: %d, %d\n", res[0], res[1]);

    _mm_empty();

    return 0;
}
