/*
 *      Copyright (C) 2005-2010 Team XBMC
 *      http://xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#include "AEConvert.h"
#include "AEUtil.h"
#include "AELookupU8.h"
#include "AELookupS16.h"
#include "MathUtils.h"
#include "utils/EndianSwap.h"

#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <string.h>

#ifdef __SSE__
#include <xmmintrin.h>
#endif

CAEConvert::AEConvertToFn CAEConvert::ToFloat(enum AEDataFormat dataFormat)
{
  switch(dataFormat)
  {
    case AE_FMT_U8   : return &U8_Float;
    case AE_FMT_S16LE: return &S16LE_Float;
    case AE_FMT_S16BE: return &S16BE_Float;
    case AE_FMT_S24BE: return &S24BE_Float;
    default:
      return NULL;
  }
}

CAEConvert::AEConvertFrFn CAEConvert::FrFloat(enum AEDataFormat dataFormat)
{
  switch(dataFormat)
  {
    case AE_FMT_U8   : return &Float_U8;
    case AE_FMT_S16LE: return &Float_S16LE;
    case AE_FMT_S16BE: return &Float_S16BE;
    default:
      return NULL;
  }
}

unsigned int CAEConvert::U8_Float(uint8_t *data, const unsigned int samples, float *dest)
{
  unsigned int i;
  for(i = 0; i < samples; ++i, ++data, ++dest)
    *dest = AELookupU8toFloat[*data];

  return samples;
}

unsigned int CAEConvert::S16LE_Float(uint8_t *data, const unsigned int samples, float *dest)
{
  unsigned int i;
  for(i = 0; i < samples; ++i, data += 2, ++dest)
  {
#if (BIG_ENDIAN == 0)
    *dest = AELookupS16toFloat[*(int16_t*)data + 32768];
#else
    int16_t value;
    swab(data, &value, 2);
    *dest = AELookupS16toFloat[value + 32768];
#endif
  }

  return samples;
}

unsigned int CAEConvert::S16BE_Float(uint8_t *data, const unsigned int samples, float *dest)
{
  unsigned int i;
  for(i = 0; i < samples; ++i, data += 2, ++dest)
  {
#if (BIG_ENDIAN == 0)
    int16_t value;
    swab(data, &value, 2);
    *(int32_t*)dest = AELookupS16toFloat[value + 32768];
#else
    *(int32_t*)dest = AELookupS16toFloat[*(int16_t*)data + 32768];
#endif
  }

  return samples;
}

unsigned int CAEConvert::S24BE_Float(uint8_t *data, const unsigned int samples, float *dest)
{
  unsigned int i;
  unsigned int viable = samples / 12;

  /* http://wiki.multimedia.cx/index.php?title=PCM#24-Bit_PCM */
  for(i = 0; i < viable; i += 4, data += 12, dest += 4)
  {
    dest[0] = ((data[0] << 16) | (data[1] << 8) | data[ 8]) / 167772155.0f;
    dest[1] = ((data[2] << 16) | (data[3] << 8) | data[ 9]) / 167772155.0f;
    dest[2] = ((data[4] << 16) | (data[5] << 8) | data[10]) / 167772155.0f;
    dest[3] = ((data[6] << 16) | (data[7] << 8) | data[11]) / 167772155.0f;
  }

  return viable;
}

unsigned int CAEConvert::Float_U8(float *data, const unsigned int samples, uint8_t *dest)
{
  #ifdef __SSE__
  const uint32_t even = (samples / 4) * 4;
  const __m128 mul = _mm_set_ps1(INT8_MAX+.5f);

  for(uint32_t i = 0; i < even; i += 4, data += 4, dest += 4)
  {
    __m128 val = _mm_mul_ps(_mm_load_ps(data), mul);
    __m64  con = _mm_cvtps_pi8(val);
    dest[0] = (uint8_t)(((int8_t*)&con)[0] + 127);
    dest[1] = (uint8_t)(((int8_t*)&con)[1] + 127);
    dest[2] = (uint8_t)(((int8_t*)&con)[2] + 127);
    dest[3] = (uint8_t)(((int8_t*)&con)[3] + 127);
  }

  if (samples != even)
  {
    const uint32_t odd = samples - even;
    if (odd == 1)
    {
      float val = (*data + 1.0f) * (INT8_MAX+.5f);
      val = val > UINT8_MAX+.0f ? UINT8_MAX+.0f : (val < 0.0f ? 0.0f : val);
      *dest = MathUtils::round_int(val);
    }
    else
    {
      __m128 in;
      memcpy(&in, data, sizeof(float) * odd);
      __m128 val = _mm_mul_ps(in, mul);
      __m64  con = _mm_cvtps_pi8(val);
      dest[0] = (uint8_t)(((int8_t*)&con)[0] + 127);
      dest[1] = (uint8_t)(((int8_t*)&con)[1] + 127);
      dest[2] = (uint8_t)(((int8_t*)&con)[2] + 127);
      dest[3] = (uint8_t)(((int8_t*)&con)[3] + 127);
    }
  }
  #else /* no SSE */
  for(uint32_t i = 0; i < samples; ++i, ++data, ++dest)
  {
    float val = (*data + 1.0f) * (INT8_MAX+.5f);
    val = val > UINT8_MAX+.0f ? UINT8_MAX+.0f : (val < 0.0f ? 0.0f : val);
    *dest = MathUtils::round_int(val);
  }
  #endif

  return samples;
}

unsigned int CAEConvert::Float_S16LE(float *data, const unsigned int samples, uint8_t *dest)
{
  uint16_t *dst = (uint16_t*)dest;
  #ifdef __SSE__
  const uint32_t even = (samples / 4) * 4;
  const __m128 mul = _mm_set_ps1(INT16_MAX+.5f);

  for(uint32_t i = 0; i < even; i += 4, data += 4, dst += 4)
  {
    __m128 val = _mm_mul_ps(_mm_load_ps(data), mul);
    *((__m64*)dst) = _mm_cvtps_pi16(val);
    #if (BIG_ENDIAN == 1)
    dst[0] = Endian_Swap16(dst[0]);
    dst[1] = Endian_Swap16(dst[1]);
    dst[2] = Endian_Swap16(dst[2]);
    dst[3] = Endian_Swap16(dst[3]);
    #endif
  }

  if (samples != even)
  {
    const uint32_t odd = samples - even;
    if (odd == 1)
    {
      float val = *data * (INT16_MAX+.5f);
      val = val > INT16_MAX ? INT16_MAX : (val < INT16_MIN ? INT16_MIN : val);
      *dst = MathUtils::round_int(val);
    }
    else
    {
      __m128 in;
      memcpy(&in, data, sizeof(float) * odd);
      __m128 val = _mm_mul_ps(in, mul);
      __m64  con = _mm_cvtps_pi16(val);
      memcpy(dst, &con, sizeof(int16_t) * odd);
    }
  }
  #else /* no SSE */
  for(uint32_t i = 0; i < samples; ++i, ++data, ++dst)
  {
    float val = *data * (INT16_MAX+.5f);
    val = val > INT16_MAX ? INT16_MAX : (val < INT16_MIN ? INT16_MIN : val);
    *dst = MathUtils::round_int(val);
    #if (BIG_ENDIAN == 1)
    *dst = Endian_Swap16(*dst);
    #endif
  }
  #endif

  return samples * 2;
}

unsigned int CAEConvert::Float_S16BE(float *data, const unsigned int samples, uint8_t *dest)
{
  uint16_t *dst = (uint16_t*)dest;
  #ifdef __SSE__
  const uint32_t even = (samples / 4) * 4;
  const __m128 mul = _mm_set_ps1(INT16_MAX+.5f);

  for(uint32_t i = 0; i < even; i += 4, data += 4, dst += 4)
  {
    __m128 val = _mm_mul_ps(_mm_load_ps(data), mul);
    *((__m64*)dst) = _mm_cvtps_pi16(val);
    #if (BIG_ENDIAN == 0)
    dst[0] = Endian_Swap16(dst[0]);
    dst[1] = Endian_Swap16(dst[1]);
    dst[2] = Endian_Swap16(dst[2]);
    dst[3] = Endian_Swap16(dst[3]);
    #endif
  }

  if (samples != even)
  {
    const uint32_t odd = samples - even;
    if (odd == 1)
    {
      float val = *data * (INT16_MAX+.5f);
      val = val > INT16_MAX ? INT16_MAX : (val < INT16_MIN ? INT16_MIN : val);
      *dst = MathUtils::round_int(val);
    }
    else
    {
      __m128 in;
      memcpy(&in, data, sizeof(float) * odd);
      __m128 val = _mm_mul_ps(in, mul);
      __m64  con = _mm_cvtps_pi16(val);
      memcpy(dst, &con, sizeof(int16_t) * odd);
    }
  }
  #else /* no SSE */
  for(uint32_t i = 0; i < samples; ++i, ++data, ++dst)
  {
    float val = *data * (INT16_MAX+.5f);
    val = val > INT16_MAX ? INT16_MAX : (val < INT16_MIN ? INT16_MIN : val);
    *dst = MathUtils::round_int(val);
    #if (BIG_ENDIAN == 0)
    *dst = Endian_Swap16(*dst);
    #endif
  }
  #endif

  return samples * 2;
}

