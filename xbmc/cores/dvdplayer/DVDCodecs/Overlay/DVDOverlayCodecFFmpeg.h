#pragma once

/*
 *      Copyright (C) 2015 Team Kodi
 *      http://kodi.tv
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
 *  along with Kodi; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include "DVDOverlayCodec.h"

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavutil/avutil.h"
}

class CDVDOverlaySpu;
class CDVDOverlayText;

class CDVDOverlayCodecFFmpeg : public CDVDOverlayCodec
{
public:
  CDVDOverlayCodecFFmpeg();
  virtual ~CDVDOverlayCodecFFmpeg();
  virtual bool Open(CDVDStreamInfo &hints, CDVDCodecOptions &options);
  virtual void Dispose();
  virtual int Decode(DemuxPacket *pPacket);
  virtual void Reset();
  virtual void Flush();
  virtual CDVDOverlay* GetOverlay();

private:
  void FreeSubtitle(AVSubtitle &sub);

  AVCodecContext* m_pCodecContext;
  AVSubtitle      m_Subtitle;
  int             m_SubtitleIndex;
  double          m_StartTime;
  double          m_StopTime;

  int             m_width;
  int             m_height;
};
