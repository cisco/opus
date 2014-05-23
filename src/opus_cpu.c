

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif


#if defined(FIXED_POINT) && defined(OPUS_HAVE_RTCD)

#include "cpu_support.h"
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
#include <cpuid.h>

static void cpuid(unsigned int CPUInfo[4], unsigned int InfoType)
{
    __get_cpuid(InfoType, &(CPUInfo[0]), &(CPUInfo[1]), &(CPUInfo[2]), &(CPUInfo[3]));
}

#endif

#include "SigProc_FIX.h"
#include "celt_lpc.h"

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

#define OPUS_DEFAULT 0
#define OPUS_SSE2    1
#define OPUS_SSE4_1  2

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


int opus_select_arch(void)
{
    static int arch = OPUS_DEFAULT;
    if( !cpu_selected )
    {
        opus_cpu_feature_check();
        
        if ( cpu_feature.HW_SSE41)
        {
            arch = OPUS_SSE4_1;
        }
        else if (cpu_feature.HW_SSE2)
        {
            arch = OPUS_SSE2;
        }

        cpu_selected = true;
    }

    return arch;
}

#endif

