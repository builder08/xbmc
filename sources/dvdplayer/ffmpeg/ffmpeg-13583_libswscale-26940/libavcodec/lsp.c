/*
 * LSP routines for ACELP-based codecs
 *
 * Copyright (c) 2008 Vladimir Voroshilov
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

#include <inttypes.h>

#include "avcodec.h"
#define FRAC_BITS 14
#include "mathops.h"
#include "lsp.h"
#include "acelp_math.h"

void ff_acelp_reorder_lsf(int16_t* lsfq, int lsfq_min_distance, int lsfq_min, int lsfq_max, int lp_order)
{
    int i, j;

    /* sort lsfq in ascending order. float bubble agorithm,
       O(n) if data already sorted, O(n^2) - otherwise */
    for(i=0; i<lp_order-1; i++)
        for(j=i; j>=0 && lsfq[j] > lsfq[j+1]; j--)
            FFSWAP(int16_t, lsfq[j], lsfq[j+1]);

    for(i=0; i<lp_order; i++)
    {
        lsfq[i] = FFMAX(lsfq[i], lsfq_min);
        lsfq_min = lsfq[i] + lsfq_min_distance;
    }
    lsfq[lp_order-1] = FFMIN(lsfq[lp_order-1], lsfq_max);//Is warning required ?
}

void ff_acelp_lsf2lsp(int16_t *lsp, const int16_t *lsf, int lp_order)
{
    int i;

    /* Convert LSF to LSP, lsp=cos(lsf) */
    for(i=0; i<lp_order; i++)
        // 20861 = 2.0 / PI in (0.15)
        lsp[i] = ff_cos(lsf[i] * 20861 >> 15); // divide by PI and (0,13) -> (0,14)
}

/**
 * \brief decodes polynomial coefficients from LSP
 * \param f [out] decoded polynomial coefficients (-0x20000000 <= (3.22) <= 0x1fffffff)
 * \param lsp LSP coefficients (-0x8000 <= (0.15) <= 0x7fff)
 */
static void lsp2poly(int* f, const int16_t* lsp, int lp_half_order)
{
    int i, j;

    f[0] = 0x400000;          // 1.0 in (3.22)
    f[1] = -lsp[0] << 8;      // *2 and (0.15) -> (3.22)

    for(i=2; i<=lp_half_order; i++)
    {
        f[i] = f[i-2];
        for(j=i; j>1; j--)
            f[j] -= MULL(f[j-1], lsp[2*i-2]) - f[j-2]; // (3.22) * (0.15) * 2 -> (3.22)

        f[1] -= lsp[2*i-2] << 8;
    }
}

void ff_acelp_lsp2lpc(int16_t* lp, const int16_t* lsp, int lp_half_order)
{
    int i;
    int f1[lp_half_order+1]; // (3.22)
    int f2[lp_half_order+1]; // (3.22)

    lsp2poly(f1, lsp  , lp_half_order);
    lsp2poly(f2, lsp+1, lp_half_order);

    /* 3.2.6 of G.729, Equations 25 and  26*/
    lp[0] = 4096;
    for(i=1; i<lp_half_order+1; i++)
    {
        int ff1 = f1[i] + f1[i-1]; // (3.22)
        int ff2 = f2[i] - f2[i-1]; // (3.22)

        ff1 += 1 << 10; // for rounding
        lp[i]    = (ff1 + ff2) >> 11; // divide by 2 and (3.22) -> (3.12)
        lp[(lp_half_order << 1) + 1 - i] = (ff1 - ff2) >> 11; // divide by 2 and (3.22) -> (3.12)
    }
}

void ff_acelp_lp_decode(int16_t* lp_1st, int16_t* lp_2nd, const int16_t* lsp_2nd, const int16_t* lsp_prev, int lp_order)
{
    int16_t lsp_1st[lp_order]; // (0.15)
    int i;

    /* LSP values for first subframe (3.2.5 of G.729, Equation 24)*/
    for(i=0; i<lp_order; i++)
#ifdef G729_BITEXACT
        lsp_1st[i] = (lsp_2nd[i] >> 1) + (lsp_prev[i] >> 1);
#else
        lsp_1st[i] = (lsp_2nd[i] + lsp_prev[i]) >> 1;
#endif

    ff_acelp_lsp2lpc(lp_1st, lsp_1st, lp_order >> 1);

    /* LSP values for second subframe (3.2.5 of G.729)*/
    ff_acelp_lsp2lpc(lp_2nd, lsp_2nd, lp_order >> 1);
}
