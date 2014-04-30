

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif


#if ENABLE_OPTIMIZE

#include "macros.h"
#include "main.h"
#include "pitch.h"

#ifdef _WIN32

//  Windows
typedef char bool;
#define false 0
#define true 1

#include <intrin.h>
#define cpuid(info,x) __cpuid(info,x)
#else
#include <stdbool.h>
//  GCC Inline Assembly
void cpuid(int CPUInfo[4],int InfoType)
{
    __asm__ __volatile__ (
        "cpuid":
        "=a" (CPUInfo[0]),
        "=b" (CPUInfo[1]),
        "=c" (CPUInfo[2]),
        "=d" (CPUInfo[3]) :
        "a" (InfoType), "c" (0)
    );
}
#endif


#include "SigProc_FIX.h"
#include "celt_lpc.h"
//#include "x86/vector_ops_FIX_sse.h"

typedef struct CPU_Feature{
    //  Misc.
    bool HW_MMX;
    bool HW_x64;
    bool HW_LZCNT;
    bool HW_RDRAND;
    bool HW_BMI1;
    bool HW_BMI2;
    bool HW_ADX;

    //  SIMD: 128-bit
    bool HW_SSE;
    bool HW_SSE2;
    bool HW_SSE3;
    bool HW_SSSE3;
    bool HW_SSE41;
    bool HW_SSE42;
    bool HW_SSE4a;
    bool HW_AES;
    bool HW_SHA;

    //  SIMD: 256-bit
    bool HW_AVX;
    bool HW_XOP;
    bool HW_FMA3;
    bool HW_FMA4;
    bool HW_AVX2;

    //  SIMD: 512-bit
    bool HW_AVX512F;
    bool HW_AVX512PF;
    bool HW_AVX512ER;
    bool HW_AVX512CD;
} CPU_Feature;

static CPU_Feature cpu_feature;
static bool cpu_selected = false;

static void opus_cpu_feature_check(void)
{
    int info[4];
	int nIds;
	unsigned int nExIds;

    cpuid(info,0);
	nIds = info[0];
	cpuid(info, 0x80000000);
    nExIds = info[0];

    if (nIds >= 0x00000001){
        cpuid(info,0x00000001);
        cpu_feature.HW_MMX    = (info[3] & ((int)1 << 23)) != 0;
        cpu_feature.HW_SSE    = (info[3] & ((int)1 << 25)) != 0;
        cpu_feature.HW_SSE2   = (info[3] & ((int)1 << 26)) != 0;
        cpu_feature.HW_SSE3   = (info[2] & ((int)1 <<  0)) != 0;

        cpu_feature.HW_SSSE3  = (info[2] & ((int)1 <<  9)) != 0;
        cpu_feature.HW_SSE41  = (info[2] & ((int)1 << 19)) != 0;
        cpu_feature.HW_SSE42  = (info[2] & ((int)1 << 20)) != 0;
        cpu_feature.HW_AES    = (info[2] & ((int)1 << 25)) != 0;

        cpu_feature.HW_AVX    = (info[2] & ((int)1 << 28)) != 0;
        cpu_feature.HW_FMA3   = (info[2] & ((int)1 << 12)) != 0;

        cpu_feature.HW_RDRAND = (info[2] & ((int)1 << 30)) != 0;
    }
    if (nIds >= 0x00000007){
        cpuid(info,0x00000007);
        cpu_feature.HW_AVX2   = (info[1] & ((int)1 <<  5)) != 0;

        cpu_feature.HW_BMI1   = (info[1] & ((int)1 <<  3)) != 0;
        cpu_feature.HW_BMI2   = (info[1] & ((int)1 <<  8)) != 0;
        cpu_feature.HW_ADX    = (info[1] & ((int)1 << 19)) != 0;

        cpu_feature.HW_AVX512F  = (info[1] & ((int)1 << 16)) != 0;
        cpu_feature.HW_AVX512PF = (info[1] & ((int)1 << 26)) != 0;
        cpu_feature.HW_AVX512ER = (info[1] & ((int)1 << 27)) != 0;
        cpu_feature.HW_AVX512CD = (info[1] & ((int)1 << 28)) != 0;
        cpu_feature.HW_SHA      = (info[1] & ((int)1 << 29)) != 0;
    }
    if (nExIds >= 0x80000001){
        cpuid(info,0x80000001);
        cpu_feature.HW_x64   = (info[3] & ((int)1 << 29)) != 0;
        cpu_feature.HW_LZCNT = (info[2] & ((int)1 <<  5)) != 0;
        cpu_feature.HW_SSE4a = (info[2] & ((int)1 <<  6)) != 0;
        cpu_feature.HW_FMA4  = (info[2] & ((int)1 << 16)) != 0;
        cpu_feature.HW_XOP   = (info[2] & ((int)1 << 11)) != 0;
    }
}


void opus_cpu_select(void)
{
    if( !cpu_selected )
    {
        opus_cpu_feature_check();
#if defined(FIXED_POINT)
        if ( cpu_feature.HW_SSE42)
        {
            silk_VQ_WMat_EC = silk_VQ_WMat_EC_sse;
            silk_warped_LPC_analysis_filter_FIX = silk_warped_LPC_analysis_filter_FIX_sse;
            silk_NSQ = silk_NSQ_sse;
            silk_NSQ_del_dec = silk_NSQ_del_dec_sse;
            silk_VAD_GetSA_Q8 = silk_VAD_GetSA_Q8_sse;
            silk_burg_modified = silk_burg_modified_sse;
            celt_fir = celt_fir_sse;
            celt_inner_prod = celt_inner_prod_sse4_1;
        }
        else if (cpu_feature.HW_SSE2)
        {
            celt_inner_prod = celt_inner_prod_sse2;
        }
#endif

        cpu_selected = true;
    }
}

#endif

