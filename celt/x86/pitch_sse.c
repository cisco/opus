#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#if ENABLE_OPTIMIZE && defined(FIXED_POINT)

#include "xmmintrin.h"
#include "emmintrin.h"
#include "smmintrin.h"
#include "macros.h"
#include "celt_lpc.h"
#include "stack_alloc.h"
#include "mathops.h"
#include "pitch.h"


OPUS_INLINE opus_val32 celt_inner_prod_sse4_1(const opus_val16 *x, const opus_val16 *y,
      int N)
{
    opus_int  i, dataSize16;
    opus_int32 sum;

    __m128i inVec1_76543210, inVec1_FEDCBA98, acc1;
    __m128i inVec2_76543210, inVec2_FEDCBA98, acc2;
    __m128i inVec1_3210, inVec2_3210;

    sum = 0;
    dataSize16 = N & 0xFFF0;

    acc1 = _mm_setzero_si128();
    acc2 = _mm_setzero_si128();

    for( i = 0; i < dataSize16; i += 16 ) {
        inVec1_76543210 = _mm_loadu_si128((__m128i*)(&x[i + 0]));
        inVec2_76543210 = _mm_loadu_si128((__m128i*)(&y[i + 0]));

        inVec1_FEDCBA98 = _mm_loadu_si128((__m128i*)(&x[i + 8]));
        inVec2_FEDCBA98 = _mm_loadu_si128((__m128i*)(&y[i + 8]));

        inVec1_76543210 = _mm_madd_epi16(inVec1_76543210, inVec2_76543210);
        inVec1_FEDCBA98 = _mm_madd_epi16(inVec1_FEDCBA98, inVec2_FEDCBA98);

        acc1 = _mm_add_epi32(acc1, inVec1_76543210);
        acc2 = _mm_add_epi32(acc2, inVec1_FEDCBA98);
    }

    acc1 = _mm_add_epi32(acc1, acc2);

    if (N - i >= 8)
    {
        inVec1_76543210 = _mm_loadu_si128((__m128i*)(&x[i + 0]));
        inVec2_76543210 = _mm_loadu_si128((__m128i*)(&y[i + 0]));

        inVec1_76543210 = _mm_madd_epi16(inVec1_76543210, inVec2_76543210);

        acc1 = _mm_add_epi32(acc1, inVec1_76543210);
        i += 8;
    }

    if (N - i >= 4)
    {
        inVec1_3210 = _mm_cvtepi16_epi32(*(__m128i*)(&x[i + 0]));
        inVec2_3210 = _mm_cvtepi16_epi32(*(__m128i*)(&y[i + 0]));

        inVec1_3210 = _mm_mullo_epi32(inVec1_3210, inVec2_3210);

        acc1 = _mm_add_epi32(acc1, inVec1_3210);
        i += 4;
    }

    acc1 = _mm_hadd_epi32(acc1, acc1);
    acc1 = _mm_hadd_epi32(acc1, acc1);
    sum += _mm_cvtsi128_si32(acc1);

    for( ; i < N; i++ ) {
        sum = silk_SMLABB( sum, x[ i ], y[ i ] );
    }

    return sum;
}

OPUS_INLINE opus_val32 celt_inner_prod_sse2(const opus_val16 *x, const opus_val16 *y,
      int N)
{
    opus_int  i, dataSize16;
    opus_int32 sum;

    __m128i inVec1_76543210, inVec1_FEDCBA98, acc1;
    __m128i inVec2_76543210, inVec2_FEDCBA98, acc2;

    sum = 0;
    dataSize16 = N & 0xFFF0;

    acc1 = _mm_setzero_si128();
    acc2 = _mm_setzero_si128();

    for( i = 0; i < dataSize16; i += 16 ) {
        inVec1_76543210 = _mm_loadu_si128((__m128i*)(&x[i + 0]));
        inVec2_76543210 = _mm_loadu_si128((__m128i*)(&y[i + 0]));

        inVec1_FEDCBA98 = _mm_loadu_si128((__m128i*)(&x[i + 8]));
        inVec2_FEDCBA98 = _mm_loadu_si128((__m128i*)(&y[i + 8]));

        inVec1_76543210 = _mm_madd_epi16(inVec1_76543210, inVec2_76543210);
        inVec1_FEDCBA98 = _mm_madd_epi16(inVec1_FEDCBA98, inVec2_FEDCBA98);

        acc1 = _mm_add_epi32(acc1, inVec1_76543210);
        acc2 = _mm_add_epi32(acc2, inVec1_FEDCBA98);
    }

    acc1 = _mm_add_epi32(acc1, acc2);

    if (N - i >= 8)
    {
        inVec1_76543210 = _mm_loadu_si128((__m128i*)(&x[i + 0]));
        inVec2_76543210 = _mm_loadu_si128((__m128i*)(&y[i + 0]));

        inVec1_76543210 = _mm_madd_epi16(inVec1_76543210, inVec2_76543210);

        acc1 = _mm_add_epi32(acc1, inVec1_76543210);
        i += 8;
    }

    acc2 = _mm_srli_si128(acc1, 8);
    acc1 = _mm_add_epi32(acc1, acc2);
    acc2 = _mm_srli_si128(acc1, 4);
    acc1 = _mm_add_epi32(acc1, acc2);
    sum += _mm_cvtsi128_si32(acc1);

    for( ; i < N; i++ ) {
        sum = silk_SMLABB( sum, x[ i ], y[ i ] );
    }

    return sum;
}
#endif