/* Copyright (c) 2009-2010 Xiph.Org Foundation
   Written by Jean-Marc Valin */
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

#ifndef PLC_H
#define PLC_H

#include "arch.h"
#include "cpu_support.h"

#define LPC_ORDER 24

void _celt_lpc(opus_val16 *_lpc, const opus_val32 *ac, int p);

void celt_fir_c(
         const opus_val16 *x,
         const opus_val16 *num,
         opus_val16 *y,
         int N,
         int ord,
         opus_val16 *mem,
         const int arch);

/*Is run-time CPU detection enabled on this platform?*/
# if defined(HAVE_SSE4_1) && defined(OPUS_HAVE_RTCD) && defined(FIXED_POINT)
void celt_fir_sse4_1(
         const opus_val16 *x,
         const opus_val16 *num,
         opus_val16 *y,
         int N,
         int ord,
         opus_val16 *mem,
         const int arch);

extern void (*const CELT_FIR_IMPL[OPUS_ARCHMASK + 1])(
         const opus_val16 *x,
         const opus_val16 *num,
         opus_val16 *y,
         int N,
         int ord,
         opus_val16 *mem,
         const int arch);

#  define celt_fir(_a, _b, _c, _d, _e, _f, arch) \
    ((*CELT_FIR_IMPL[(arch) & OPUS_ARCHMASK])(_a, _b, _c, _d, _e, _f, arch))
# else
#  define celt_fir(_a, _b, _c, _d, _e, _f, arch) \
    (celt_fir_c(_a, _b, _c, _d, _e, _f, arch))

# endif

void celt_iir(const opus_val32 *x,
         const opus_val16 *den,
         opus_val32 *y,
         int N,
         int ord,
         opus_val16 *mem,
         const int arch);

int _celt_autocorr(const opus_val16 *x, opus_val32 *ac,
         const opus_val16 *window, int overlap, int lag, int n, int arch);

#endif /* PLC_H */
