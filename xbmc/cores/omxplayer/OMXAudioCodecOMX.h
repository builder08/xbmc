#pragma once

/*
 *      Copyright (C) 2005-2013 Team XBMC
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

#ifndef OMXPLAYER_CORES_AUDIOENGINE_UTILS_AEAUDIOFORMAT_H_INCLUDED
#define OMXPLAYER_CORES_AUDIOENGINE_UTILS_AEAUDIOFORMAT_H_INCLUDED
#include "cores/AudioEngine/Utils/AEAudioFormat.h"
#endif

#ifndef OMXPLAYER_DLLAVCODEC_H_INCLUDED
#define OMXPLAYER_DLLAVCODEC_H_INCLUDED
#include "DllAvCodec.h"
#endif

#ifndef OMXPLAYER_DLLAVFORMAT_H_INCLUDED
#define OMXPLAYER_DLLAVFORMAT_H_INCLUDED
#include "DllAvFormat.h"
#endif

#ifndef OMXPLAYER_DLLAVUTIL_H_INCLUDED
#define OMXPLAYER_DLLAVUTIL_H_INCLUDED
#include "DllAvUtil.h"
#endif

#ifndef OMXPLAYER_DLLSWRESAMPLE_H_INCLUDED
#define OMXPLAYER_DLLSWRESAMPLE_H_INCLUDED
#include "DllSwResample.h"
#endif


#ifndef OMXPLAYER_DVDSTREAMINFO_H_INCLUDED
#define OMXPLAYER_DVDSTREAMINFO_H_INCLUDED
#include "DVDStreamInfo.h"
#endif

#ifndef OMXPLAYER_LINUX_PLATFORMDEFS_H_INCLUDED
#define OMXPLAYER_LINUX_PLATFORMDEFS_H_INCLUDED
#include "linux/PlatformDefs.h"
#endif


class COMXAudioCodecOMX
{
public:
  COMXAudioCodecOMX();
  virtual ~COMXAudioCodecOMX();
  bool Open(CDVDStreamInfo &hints);
  void Dispose();
  int Decode(BYTE* pData, int iSize);
  int GetData(BYTE** dst);
  void Reset();
  int GetChannels();
  uint64_t GetChannelMap();
  int GetSampleRate();
  int GetBitsPerSample();
  static const char* GetName() { return "FFmpeg"; }
  int GetBitRate();

protected:
  AVCodecContext* m_pCodecContext;
  SwrContext*     m_pConvert;
  enum AVSampleFormat m_iSampleFormat;
  enum AVSampleFormat m_desiredSampleFormat;

  AVFrame* m_pFrame1;

  BYTE *m_pBufferOutput;
  int   m_iBufferOutputAlloced;

  bool m_bOpenedCodec;

  int     m_channels;

  bool m_bFirstFrame;
  bool m_bGotFrame;
  DllAvCodec m_dllAvCodec;
  DllAvUtil m_dllAvUtil;
  DllSwResample m_dllSwResample;
};
