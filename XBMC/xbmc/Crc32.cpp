//------------------------------------------------------------------------
// Copyright (C) Sewell Development Corporation, 1994 - 2000.
//     Web: www.sewelld.com      E-mail: support@sewelld.com
//
// LICENSE: This source code was generated by CrcGen, a product of Sewell
// Development Corporation.  Paid-up licensees of CrcGen are authorized to
// use this code on a site-wide basis without restriction as to
// the type of product it is incorporated in, except that it may not be
// resold as stand-alone CRC code, and the copyright notice and license
// agreement must not be removed from the code.
//------------------------------------------------------------------------

#include "stdafx.h"
#include "Crc32.h"

// Implementation of 32-bit CRC (cyclic redundancy check) class:
//     Polynomial:                 04C11DB7
//     Initial CRC register value: FFFFFFFF
//     Reflected input and output: No
//     Inverted final output:      No
//     CRC of CStdString "123456789":  0376E6E7

void Crc32::Compute(const void* buffer, unsigned int count)
{
  const unsigned char* ptr = (const unsigned char *) buffer;
  while (count--)
  {
    Compute(*ptr++);
  }
}

void Crc32::Compute(unsigned char value)
{
  m_crc ^= ((unsigned __int32)value << 24);
  for (int i = 0; i < 8; i++)
  {
    if (m_crc & 0x80000000)
    {
      m_crc = (m_crc << 1) ^ 0x04C11DB7;
    }
    else
    {
      m_crc <<= 1;
    }
  }
}

void Crc32::Compute(const CStdString& strValue)
{
  Compute(strValue.c_str(), strValue.size());
}

void Crc32::ComputeFromLowerCase(const CStdString& strValue)
{
  CStdString strLower = strValue;
  strLower.MakeLower();
  Compute(strLower.c_str(), strLower.size());
}

