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

OPUS_INLINE void xcorr_kernel_sse2(const opus_val16 * x, const opus_val16 * y, opus_val32 sum[4], int len)
{
    int j;
    celt_assert(len>=3);

    __m128i vecX, vecX0, vecX1, vecX2, vecX3;
    __m128i vecY0, vecY1, vecY2, vecY3;
    __m128i sum0, sum1, sum2, sum3, vecSum;

    sum0 = _mm_setzero_si128();
    sum1 = _mm_setzero_si128();
    sum2 = _mm_setzero_si128();
    sum3 = _mm_setzero_si128();

    for( j = 0; j < len-7; j += 8 )
    {
        vecX = _mm_loadu_si128((__m128i*)(&x[j + 0]));
        vecY0 = _mm_loadu_si128((__m128i*)(&y[j + 0]));
        vecY1 = _mm_loadu_si128((__m128i*)(&y[j + 1]));
        vecY2 = _mm_loadu_si128((__m128i*)(&y[j + 2]));
        vecY3 = _mm_loadu_si128((__m128i*)(&y[j + 3]));

        sum0 = _mm_add_epi32(sum0, _mm_madd_epi16(vecX, vecY0) );
        sum1 = _mm_add_epi32(sum1, _mm_madd_epi16(vecX, vecY1) );
        sum2 = _mm_add_epi32(sum2, _mm_madd_epi16(vecX, vecY2) );
        sum3 = _mm_add_epi32(sum3, _mm_madd_epi16(vecX, vecY3) );
    }

    vecX = _mm_srli_si128(sum0, 8);
    sum0 = _mm_add_epi32(sum0, vecX);
    vecX = _mm_srli_si128(sum0, 4);
    sum0 = _mm_add_epi32(sum0, vecX);

    vecY0 = _mm_srli_si128(sum1, 8);
    sum1 = _mm_add_epi32(sum1, vecY0);
    vecY0 = _mm_srli_si128(sum1, 4);
    sum1 = _mm_add_epi32(sum1, vecY0);

    vecY1 = _mm_srli_si128(sum2, 8);
    sum2 = _mm_add_epi32(sum2, vecY1);
    vecY1 = _mm_srli_si128(sum2, 4);
    sum2 = _mm_add_epi32(sum2, vecY1);

    vecY2 = _mm_srli_si128(sum3, 8);
    sum3 = _mm_add_epi32(sum3, vecY2);
    vecY2 = _mm_srli_si128(sum3, 4);
    sum3 = _mm_add_epi32(sum3, vecY2);

    vecSum = _mm_set_epi32( _mm_cvtsi128_si32(sum3),
                            _mm_cvtsi128_si32(sum2),
                            _mm_cvtsi128_si32(sum1),
                            _mm_cvtsi128_si32(sum0) );

    for( ; j < len-3; j += 4 )
    {
        vecX = _mm_cvtepi16_epi32(*(__m128i*)(&x[j + 0]));
        vecX0 = _mm_shuffle_epi32(vecX, 0x00);
        vecX1 = _mm_shuffle_epi32(vecX, 0x55);
        vecX2 = _mm_shuffle_epi32(vecX, 0xaa);
        vecX3 = _mm_shuffle_epi32(vecX, 0xff);

        vecY0 = _mm_cvtepi16_epi32(*(__m128i*)(&y[j + 0]));
        vecY1 = _mm_cvtepi16_epi32(*(__m128i*)(&y[j + 1]));
        vecY2 = _mm_cvtepi16_epi32(*(__m128i*)(&y[j + 2]));
        vecY3 = _mm_cvtepi16_epi32(*(__m128i*)(&y[j + 3]));

        sum0 = _mm_mullo_epi32(vecX0, vecY0);
        sum1 = _mm_mullo_epi32(vecX1, vecY1);
        sum2 = _mm_mullo_epi32(vecX2, vecY2);
        sum3 = _mm_mullo_epi32(vecX3, vecY3);

        sum0 = _mm_add_epi32(sum0, sum1);
        sum2 = _mm_add_epi32(sum2, sum3);
        vecSum = _mm_add_epi32(vecSum, sum0);
        vecSum = _mm_add_epi32(vecSum, sum2);
    }

    for( ; j < len; j++ ) {
        vecX = _mm_cvtepi16_epi32(*(__m128i*)(&x[j + 0]));
        vecX0 = _mm_shuffle_epi32(vecX, 0x00);

        vecY0 = _mm_cvtepi16_epi32(*(__m128i*)(&y[j + 0]));

        sum0 = _mm_mullo_epi32(vecX0, vecY0);
        vecSum = _mm_add_epi32(vecSum, sum0);
    }

    _mm_storeu_si128((__m128i*)(sum), vecSum);
}

OPUS_INLINE void xcorr_kernel_sse4_1(const opus_val16 * x, const opus_val16 * y, opus_val32 sum[4], int len)
{
    int j;
    celt_assert(len>=3);

    __m128i vecX, vecX0, vecX1, vecX2, vecX3;
    __m128i vecY0, vecY1, vecY2, vecY3;
    __m128i sum0, sum1, sum2, sum3, vecSum;

    sum0 = _mm_setzero_si128();
    sum1 = _mm_setzero_si128();
    sum2 = _mm_setzero_si128();
    sum3 = _mm_setzero_si128();

    for( j = 0; j < len-7; j += 8 )
    {
        vecX = _mm_loadu_si128((__m128i*)(&x[j + 0]));
        vecY0 = _mm_loadu_si128((__m128i*)(&y[j + 0]));
        vecY1 = _mm_loadu_si128((__m128i*)(&y[j + 1]));
        vecY2 = _mm_loadu_si128((__m128i*)(&y[j + 2]));
        vecY3 = _mm_loadu_si128((__m128i*)(&y[j + 3]));

        sum0 = _mm_add_epi32(sum0, _mm_madd_epi16(vecX, vecY0) );
        sum1 = _mm_add_epi32(sum1, _mm_madd_epi16(vecX, vecY1) );
        sum2 = _mm_add_epi32(sum2, _mm_madd_epi16(vecX, vecY2) );
        sum3 = _mm_add_epi32(sum3, _mm_madd_epi16(vecX, vecY3) );
    }

    sum0 = _mm_hadd_epi32(sum0, sum0);
    sum0 = _mm_hadd_epi32(sum0, sum0);

    sum1 = _mm_hadd_epi32(sum1, sum1);
    sum1 = _mm_hadd_epi32(sum1, sum1);

    sum2 = _mm_hadd_epi32(sum2, sum2);
    sum2 = _mm_hadd_epi32(sum2, sum2);

    sum3 = _mm_hadd_epi32(sum3, sum3);
    sum3 = _mm_hadd_epi32(sum3, sum3);

    vecSum = _mm_set_epi32( _mm_cvtsi128_si32(sum3),
                            _mm_cvtsi128_si32(sum2),
                            _mm_cvtsi128_si32(sum1),
                            _mm_cvtsi128_si32(sum0) );

    for( ; j < len-3; j += 4 )
    {
        vecX = _mm_cvtepi16_epi32(*(__m128i*)(&x[j + 0]));
        vecX0 = _mm_shuffle_epi32(vecX, 0x00);
        vecX1 = _mm_shuffle_epi32(vecX, 0x55);
        vecX2 = _mm_shuffle_epi32(vecX, 0xaa);
        vecX3 = _mm_shuffle_epi32(vecX, 0xff);

        vecY0 = _mm_cvtepi16_epi32(*(__m128i*)(&y[j + 0]));
        vecY1 = _mm_cvtepi16_epi32(*(__m128i*)(&y[j + 1]));
        vecY2 = _mm_cvtepi16_epi32(*(__m128i*)(&y[j + 2]));
        vecY3 = _mm_cvtepi16_epi32(*(__m128i*)(&y[j + 3]));

        sum0 = _mm_mullo_epi32(vecX0, vecY0);
        sum1 = _mm_mullo_epi32(vecX1, vecY1);
        sum2 = _mm_mullo_epi32(vecX2, vecY2);
        sum3 = _mm_mullo_epi32(vecX3, vecY3);

        sum0 = _mm_add_epi32(sum0, sum1);
        sum2 = _mm_add_epi32(sum2, sum3);
        vecSum = _mm_add_epi32(vecSum, sum0);
        vecSum = _mm_add_epi32(vecSum, sum2);
    }

    for( ; j < len; j++ ) {
        vecX = _mm_cvtepi16_epi32(*(__m128i*)(&x[j + 0]));
        vecX0 = _mm_shuffle_epi32(vecX, 0x00);

        vecY0 = _mm_cvtepi16_epi32(*(__m128i*)(&y[j + 0]));

        sum0 = _mm_mullo_epi32(vecX0, vecY0);
        vecSum = _mm_add_epi32(vecSum, sum0);
    }

    _mm_storeu_si128((__m128i*)(sum), vecSum);
}
#endif