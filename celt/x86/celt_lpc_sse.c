#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#if defined(HAVE_SSE4_1) && defined(OPUS_HAVE_RTCD) && defined(FIXED_POINT)

#pragma GCC target ("sse4.1")

#include <xmmintrin.h>
#include <emmintrin.h>
#include <smmintrin.h>
#include "celt_lpc.h"
#include "stack_alloc.h"
#include "mathops.h"
#include "pitch.h"

void celt_fir_sse4_1(const opus_val16 *_x,
         const opus_val16 *num,
         opus_val16 *_y,
         int N,
         int ord,
         opus_val16 *mem,
         const int arch)
{
    int i,j;
    VARDECL(opus_val16, rnum);
    VARDECL(opus_val16, x);

    __m128i vecNoA;
    opus_int32 noA ;
    opus_val32 sums[4]={0,0,0,0};
    __m128i vecSum, vecX;
    opus_val32 sum ;
    SAVE_STACK;

   ALLOC(rnum, ord, opus_val16);
   ALLOC(x, N+ord, opus_val16);
   for(i=0;i<ord;i++)
      rnum[i] = num[ord-i-1];
   for(i=0;i<ord;i++)
      x[i] = mem[ord-i-1];
	  
   for (i=0;i<N-7;i+=8)
   {
	   x[i+ord  ]=_x[i  ];
	   x[i+ord+1]=_x[i+1];
	   x[i+ord+2]=_x[i+2];
	   x[i+ord+3]=_x[i+3];
	   x[i+ord+4]=_x[i+4];
	   x[i+ord+5]=_x[i+5];
	   x[i+ord+6]=_x[i+6];
	   x[i+ord+7]=_x[i+7];
   }

   for (;i<N-3;i+=4)
   {
	   x[i+ord  ]=_x[i  ];
	   x[i+ord+1]=_x[i+1];
	   x[i+ord+2]=_x[i+2];
	   x[i+ord+3]=_x[i+3];
   }

   for (;i<N;i++)
         x[i+ord]=_x[i];

   for(i=0;i<ord;i++)
      mem[i] = _x[N-i-1];
#ifdef SMALL_FOOTPRINT
   for (i=0;i<N;i++)
   {
      //opus_val32 
      sum = SHL32(EXTEND32(_x[i]), SIG_SHIFT);
      for (j=0;j<ord;j++)
      {
         sum = MAC16_16(sum,rnum[j],x[i+j]);
      }
      _y[i] = SATURATE16(PSHR32(sum, SIG_SHIFT));
   }
#else
   
   //opus_int32 
   noA = (EXTEND32(1)<<(SIG_SHIFT)>>1);
  // __m128i 
    vecNoA = _mm_set_epi32(noA, noA, noA, noA);

   for (i=0;i<N-3;i+=4)
   {
     // opus_val32 sums[4]={0,0,0,0};
      xcorr_kernel(rnum, x+i, sums, ord, arch);
	  
      //__m128i vecSum, vecX;
      vecSum = _mm_loadu_si128( (__m128i*)sums );
      vecSum = _mm_add_epi32(vecSum, vecNoA);
      vecSum = _mm_srai_epi32(vecSum, SIG_SHIFT);
      vecX = _mm_cvtepi16_epi32( *(__m128i*)(_x+i) );
      vecSum = _mm_add_epi32(vecSum, vecX);
      vecSum = _mm_packs_epi32(vecSum, vecSum);
      _mm_storeu_si128( (__m128i*)(_y+i), vecSum );
	  
   }
   for (;i<N;i++)
   {
      //opus_val32 
      sum = 0;
      for (j=0;j<ord;j++)
         sum = MAC16_16(sum,rnum[j],x[i+j]);
      _y[i] = SATURATE16(ADD32(EXTEND32(_x[i]), PSHR32(sum, SIG_SHIFT)));
   }

   
#endif
   RESTORE_STACK;
}

#endif