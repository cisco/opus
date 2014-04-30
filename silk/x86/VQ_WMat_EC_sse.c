#ifdef HAVE_CONFIG_H
#include "config.h"
#endif


#if ENABLE_OPTIMIZE
#include "xmmintrin.h"
#include "emmintrin.h"
#include "smmintrin.h"
#include "main.h"

/* Entropy constrained matrix-weighted VQ, hard-coded to 5-element vectors, for a single input data vector */
void silk_VQ_WMat_EC_sse(
    opus_int8                   *ind,                           /* O    index of best codebook vector               */
    opus_int32                  *rate_dist_Q14,                 /* O    best weighted quant error + mu * rate       */
    opus_int                    *gain_Q7,                       /* O    sum of absolute LTP coefficients            */
    const opus_int16            *in_Q14,                        /* I    input vector to be quantized                */
    const opus_int32            *W_Q18,                         /* I    weighting matrix                            */
    const opus_int8             *cb_Q7,                         /* I    codebook                                    */
    const opus_uint8            *cb_gain_Q7,                    /* I    codebook effective gain                     */
    const opus_uint8            *cl_Q5,                         /* I    code length for each codebook vector        */
    const opus_int              mu_Q9,                          /* I    tradeoff betw. weighted error and rate      */
    const opus_int32            max_gain_Q7,                    /* I    maximum sum of absolute LTP coefficients    */
    opus_int                    L                               /* I    number of vectors in codebook               */
)
{
    opus_int   k, gain_tmp_Q7;
    const opus_int8 *cb_row_Q7;
    opus_int16 diff_Q14[ 5 ];
    opus_int32 sum1_Q14, sum2_Q16;
    opus_int32 sum2_Q16_tmp1, sum2_Q16_tmp2, sum2_Q16_tmp3;

    /* Loop over codebook */
    *rate_dist_Q14 = silk_int32_MAX;
    cb_row_Q7 = cb_Q7;
    for( k = 0; k < L; k++ ) {
	    gain_tmp_Q7 = cb_gain_Q7[k];

		diff_Q14[ 0 ] = in_Q14[ 0 ] - silk_LSHIFT( cb_row_Q7[ 0 ], 7 );

        __m128i C_tmp1, C_tmp2, C_tmp3, C_tmp4, C_tmp5;
        C_tmp1 = _mm_loadu_si128((__m128i*)(&in_Q14[1]));
        C_tmp1 = _mm_cvtepi16_epi32(C_tmp1);
        C_tmp2 = _mm_loadu_si128((__m128i*)(&cb_row_Q7[1]));
        C_tmp2 = _mm_cvtepi8_epi32(C_tmp2);
        C_tmp2 = _mm_slli_epi32(C_tmp2, 7);
        C_tmp1 = _mm_sub_epi32(C_tmp1, C_tmp2);

        diff_Q14[ 1 ] = _mm_extract_epi16(C_tmp1, 0);
        diff_Q14[ 2 ] = _mm_extract_epi16(C_tmp1, 2);
        diff_Q14[ 3 ] = _mm_extract_epi16(C_tmp1, 4);
        diff_Q14[ 4 ] = _mm_extract_epi16(C_tmp1, 6);

		/* Weighted rate */
        sum1_Q14 = silk_SMULBB( mu_Q9, cl_Q5[ k ] );

		/* Penalty for too large gain */
		sum1_Q14 = silk_ADD_LSHIFT32( sum1_Q14, silk_max( silk_SUB32( gain_tmp_Q7, max_gain_Q7 ), 0 ), 10 );

		silk_assert( sum1_Q14 >= 0 );

		/* first row of W_Q18 */
        C_tmp3 = _mm_loadu_si128((__m128i*)(&W_Q18[1]));
        C_tmp4 = _mm_mul_epi32(C_tmp3, C_tmp1);
        C_tmp4 = _mm_srli_si128(C_tmp4, 2);

        C_tmp1 = _mm_srli_si128(C_tmp1, 4);
        C_tmp3 = _mm_srli_si128(C_tmp3, 4);

        C_tmp5 = _mm_mul_epi32(C_tmp3, C_tmp1);
        C_tmp5 = _mm_srli_si128(C_tmp5, 2);

        C_tmp5 = _mm_add_epi32(C_tmp4, C_tmp5);
        C_tmp5 = _mm_slli_epi32(C_tmp5, 1);
        sum2_Q16 = _mm_extract_epi32(C_tmp5, 0) + _mm_extract_epi32(C_tmp5, 2);

        sum2_Q16 = silk_SMLAWB( sum2_Q16, W_Q18[  0 ], diff_Q14[ 0 ] );
        sum1_Q14 = silk_SMLAWB( sum1_Q14, sum2_Q16,    diff_Q14[ 0 ] );

        /* second row of W_Q18 */
        sum2_Q16_tmp1 = silk_SMULWB(           W_Q18[  7 ], diff_Q14[ 2 ] );
        sum2_Q16_tmp2 = silk_SMULWB(           W_Q18[  8 ], diff_Q14[ 3 ] );
        sum2_Q16_tmp3 = silk_SMULWB(           W_Q18[  9 ], diff_Q14[ 4 ] );
        sum2_Q16 = sum2_Q16_tmp1 + sum2_Q16_tmp2 + sum2_Q16_tmp3;

        sum2_Q16 = silk_LSHIFT( sum2_Q16, 1 );
        sum2_Q16 = silk_SMLAWB( sum2_Q16, W_Q18[  6 ], diff_Q14[ 1 ] );
        sum1_Q14 = silk_SMLAWB( sum1_Q14, sum2_Q16,    diff_Q14[ 1 ] );

        /* third row of W_Q18 */
        sum2_Q16_tmp1 = silk_SMULWB(           W_Q18[ 13 ], diff_Q14[ 3 ] );
        sum2_Q16_tmp2 = silk_SMULWB(           W_Q18[ 14 ], diff_Q14[ 4 ] );
        sum2_Q16 = sum2_Q16_tmp1 + sum2_Q16_tmp2;

        sum2_Q16 = silk_LSHIFT( sum2_Q16, 1 );
        sum2_Q16 = silk_SMLAWB( sum2_Q16, W_Q18[ 12 ], diff_Q14[ 2 ] );
        sum1_Q14 = silk_SMLAWB( sum1_Q14, sum2_Q16,    diff_Q14[ 2 ] );

        /* fourth row of W_Q18 */
        sum2_Q16 = silk_SMULWB(           W_Q18[ 19 ], diff_Q14[ 4 ] );
        sum2_Q16 = silk_LSHIFT( sum2_Q16, 1 );
        sum2_Q16 = silk_SMLAWB( sum2_Q16, W_Q18[ 18 ], diff_Q14[ 3 ] );
        sum1_Q14 = silk_SMLAWB( sum1_Q14, sum2_Q16,    diff_Q14[ 3 ] );

        /* last row of W_Q18 */
        sum2_Q16 = silk_SMULWB(           W_Q18[ 24 ], diff_Q14[ 4 ] );
        sum1_Q14 = silk_SMLAWB( sum1_Q14, sum2_Q16,    diff_Q14[ 4 ] );

        silk_assert( sum1_Q14 >= 0 );

        /* find best */
        if( sum1_Q14 < *rate_dist_Q14 ) {
            *rate_dist_Q14 = sum1_Q14;
            *ind = (opus_int8)k;
			*gain_Q7 = gain_tmp_Q7;
        }

        /* Go to next cbk vector */
        cb_row_Q7 += LTP_ORDER;
    }
}
#endif

