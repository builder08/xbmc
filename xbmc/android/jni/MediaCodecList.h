#pragma once
/*
 *      Copyright (C) 2013 Team XBMC
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
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#ifndef JNI_JNIBASE_H_INCLUDED
#define JNI_JNIBASE_H_INCLUDED
#include "JNIBase.h"
#endif

#ifndef JNI_MEDIACODECINFO_H_INCLUDED
#define JNI_MEDIACODECINFO_H_INCLUDED
#include "MediaCodecInfo.h"
#endif


class CJNIMediaCodecList : public CJNIBase
{
public:
  CJNIMediaCodecList(const jni::jhobject &object) : CJNIBase(object) {};
  //~CJNIMediaCodecList() {};

  static int   getCodecCount();
  static const CJNIMediaCodecInfo getCodecInfoAt(int index);

private:
  CJNIMediaCodecList();

  static const char *m_classname;
};
