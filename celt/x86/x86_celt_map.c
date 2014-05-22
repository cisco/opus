/* Copyright (c) 2010 Xiph.Org Foundation
 * Copyright (c) 2013 Parrot */
/*
   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

   - Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.

   - Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
   OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#if (defined(HAVE_SSE4_1) || defined(HAVE_SSE2))&& defined(OPUS_HAVE_RTCD) && defined(FIXED_POINT)

#if defined(HAVE_SSE4_1)
#pragma GCC target ("sse4.1")
#else
#pragma GCC target ("sse2")
#endif

#include "x86/x86cpu.h"
#include "structs.h"
#include "celt_lpc.h"
#include "pitch.h"


# if defined(HAVE_SSE4_1)
void (*const CELT_FIR_IMPL[OPUS_ARCHMASK + 1])(
         const opus_val16 *,
         const opus_val16 *,
         opus_val16 *,
         int ,
         int ,
         opus_val16 *,
         int
) = {
  celt_fir_c,                /* non-sse */
  celt_fir_c, 
  MAY_HAVE_SSE4_1(celt_fir), /* sse4.1  */
  celt_fir_c
};

void (*const XCORR_KERNEL_IMPL[OPUS_ARCHMASK + 1])(
         const opus_val16 *,
         const opus_val16 *,
         opus_val32       *,
         int
) = {
  xcorr_kernel_c,                /* non-sse */
  xcorr_kernel_c,
  MAY_HAVE_SSE4_1(xcorr_kernel), /* sse4.1  */
  xcorr_kernel_c
};
#endif

opus_val32 (*const CELT_INNER_PROD_IMPL[OPUS_ARCHMASK + 1])(
         const opus_val16 *,
         const opus_val16 *,
         int
) = {
  celt_inner_prod_c,                /* non-sse */
  MAY_HAVE_SSE2(celt_inner_prod), 
  MAY_HAVE_SSE4_1(celt_inner_prod), /* sse4.1  */
  celt_inner_prod_c
};

#endif

