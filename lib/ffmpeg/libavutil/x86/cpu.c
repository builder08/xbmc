/*
 * CPU detection code, extracted from mmx.h
 * (c)1997-99 by H. Dietz and R. Fisher
 * Converted to C and improved by Fabrice Bellard.
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <stdlib.h>
#include <string.h>
#include "libavutil/x86_cpu.h"
#include "libavutil/cpu.h"

/* ebx saving is necessary for PIC. gcc seems unable to see it alone */
#define cpuid(index,eax,ebx,ecx,edx)\
    __asm__ volatile\
        ("mov %%"REG_b", %%"REG_S"\n\t"\
         "cpuid\n\t"\
         "xchg %%"REG_b", %%"REG_S\
         : "=a" (eax), "=S" (ebx),\
           "=c" (ecx), "=d" (edx)\
         : "0" (index));

/* Function to test if multimedia instructions are supported...  */
int ff_get_cpu_flags_x86(void)
{
    int rval = 0;
    int eax, ebx, ecx, edx;
    int max_std_level, max_ext_level, std_caps=0, ext_caps=0;
    int family=0, model=0;
    union { int i[3]; char c[12]; } vendor;

#if ARCH_X86_32
    x86_reg a, c;
    __asm__ volatile (
        /* See if CPUID instruction is supported ... */
        /* ... Get copies of EFLAGS into eax and ecx */
        "pushfl\n\t"
        "pop %0\n\t"
        "mov %0, %1\n\t"

        /* ... Toggle the ID bit in one copy and store */
        /*     to the EFLAGS reg */
        "xor $0x200000, %0\n\t"
        "push %0\n\t"
        "popfl\n\t"

        /* ... Get the (hopefully modified) EFLAGS */
        "pushfl\n\t"
        "pop %0\n\t"
        : "=a" (a), "=c" (c)
        :
        : "cc"
        );

    if (a == c)
        return 0; /* CPUID not supported */
#endif

    cpuid(0, max_std_level, vendor.i[0], vendor.i[2], vendor.i[1]);

    if(max_std_level >= 1){
        cpuid(1, eax, ebx, ecx, std_caps);
        family = ((eax>>8)&0xf) + ((eax>>20)&0xff);
        model  = ((eax>>4)&0xf) + ((eax>>12)&0xf0);
        if (std_caps & (1<<23))
            rval |= AV_CPU_FLAG_MMX;
        if (std_caps & (1<<25))
            rval |= AV_CPU_FLAG_MMX2
#if HAVE_SSE
                  | AV_CPU_FLAG_SSE;
        if (std_caps & (1<<26))
            rval |= AV_CPU_FLAG_SSE2;
        if (ecx & 1)
            rval |= AV_CPU_FLAG_SSE3;
        if (ecx & 0x00000200 )
            rval |= AV_CPU_FLAG_SSSE3;
        if (ecx & 0x00080000 )
            rval |= AV_CPU_FLAG_SSE4;
        if (ecx & 0x00100000 )
            rval |= AV_CPU_FLAG_SSE42;
#endif
                  ;
    }

    cpuid(0x80000000, max_ext_level, ebx, ecx, edx);

    if(max_ext_level >= 0x80000001){
        cpuid(0x80000001, eax, ebx, ecx, ext_caps);
        if (ext_caps & (1<<31))
            rval |= AV_CPU_FLAG_3DNOW;
        if (ext_caps & (1<<30))
            rval |= AV_CPU_FLAG_3DNOWEXT;
        if (ext_caps & (1<<23))
            rval |= AV_CPU_FLAG_MMX;
        if (ext_caps & (1<<22))
            rval |= AV_CPU_FLAG_MMX2;
    }

    if (!strncmp(vendor.c, "GenuineIntel", 12) &&
        family == 6 && (model == 9 || model == 13 || model == 14)) {
        /* 6/9 (pentium-m "banias"), 6/13 (pentium-m "dothan"), and 6/14 (core1 "yonah")
         * theoretically support sse2, but it's usually slower than mmx,
         * so let's just pretend they don't. */
        if (rval & AV_CPU_FLAG_SSE2) rval ^= AV_CPU_FLAG_SSE2SLOW|AV_CPU_FLAG_SSE2;
        if (rval & AV_CPU_FLAG_SSE3) rval ^= AV_CPU_FLAG_SSE3SLOW|AV_CPU_FLAG_SSE3;
    }

    return rval;
}
