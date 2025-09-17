#include <immintrin.h>
#include <stdint.h>

bool isemptyavx2(const char *data, size_t *size) {
    const __m256i zero = _mm256_setzero_si256();

    size_t i = 0;
    bool result = true;
    for (; i + 32 <= *size && result; i += 32) {
        const __m256i cnt = _mm256_loadu_si256((const __m256i*)(data + i));
        const __m256i cmp = _mm256_cmpeq_epi8(cnt, zero);
        const int32_t mask = _mm256_movemask_epi8(cmp);
        if (mask != -1)
            result = false;
    }
    *size -= i;
    return result;
}

bool isempty(const uint8_t *data, size_t size) {
    const size_t _ss = size;
    if (isemptyavx2((const char *)data, &size)) {
        if (!size)
            return true;
        for (size_t i = _ss - size; i < size; i++) {
            if (data[i] != 0)
                return false;
        }
    } else if (size) {
        return false;
    }
    return true;
}
