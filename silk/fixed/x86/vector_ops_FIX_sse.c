#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#if ENABLE_OPTIMIZE
#include "xmmintrin.h"
#include "emmintrin.h"
#include "smmintrin.h"
#include "main.h"

#include "SigProc_FIX.h"
#include "pitch.h"

opus_int64 silk_inner_prod16_aligned_64_sse(
    const opus_int16            *inVec1,            /*    I input vector 1                                              */
    const opus_int16            *inVec2,            /*    I input vector 2                                              */
    const opus_int              len                 /*    I vector lengths                                              */
)
{
    opus_int  i, dataSize8;
    opus_int64 sum;

    __m128i xmm_tempa;
    __m128i inVec1_76543210, acc1;
    __m128i inVec2_76543210, acc2;

    sum = 0;
    dataSize8 = len & 0xFFF8;

    acc1 = _mm_setzero_si128();
    acc2 = _mm_setzero_si128();

    for( i = 0; i < dataSize8; i += 8 ) {
        inVec1_76543210 = _mm_loadu_si128((__m128i*)(&inVec1[i + 0]));
        inVec2_76543210 = _mm_loadu_si128((__m128i*)(&inVec2[i + 0]));

        // only when all 4 operands are -32768 (0x8000), this results in wrap around
        inVec1_76543210 = _mm_madd_epi16(inVec1_76543210, inVec2_76543210);

        xmm_tempa       = _mm_cvtepi32_epi64(inVec1_76543210);
        inVec1_76543210 = _mm_srli_si128(inVec1_76543210, 8);
        inVec1_76543210 = _mm_cvtepi32_epi64(inVec1_76543210);

        acc1 = _mm_add_epi64(acc1, xmm_tempa);
        acc2 = _mm_add_epi64(acc2, inVec1_76543210);
    }

    acc1 = _mm_add_epi64(acc1, acc2);

    acc2 = _mm_srli_si128(acc1, 8);
    acc1 = _mm_add_epi64(acc1, acc2);
    sum += _mm_cvtsi128_si64(acc1);

    for( ; i < len; i++ ) {
        sum = silk_SMLABB( sum, inVec1[ i ], inVec2[ i ] );
    }

    return sum;
}

#endif
