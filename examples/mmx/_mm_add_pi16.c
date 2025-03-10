#include <mmintrin.h>
#include <stdio.h>
#include <stdint.h>

int main() {
    __v4hi m1 = {0, 1, 2, 3};
    __v4hi m2 = {8, 9, 10, 11};
    __v4hi result = _mm_add_pi16(m1, m2);

    for (uint8_t i = 0; i < 4; ++i)
        printf("%d\n", result[i]);

    _mm_empty();
    return 0;
}
