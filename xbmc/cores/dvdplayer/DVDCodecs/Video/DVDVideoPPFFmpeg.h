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

#ifndef VIDEO_DVDVIDEOCODEC_H_INCLUDED
#define VIDEO_DVDVIDEOCODEC_H_INCLUDED
#include "DVDVideoCodec.h"
#endif

#ifndef VIDEO_DLLPOSTPROC_H_INCLUDED
#define VIDEO_DLLPOSTPROC_H_INCLUDED
#include "DllPostProc.h"
#endif


class CDVDVideoPPFFmpeg
{
public:

  CDVDVideoPPFFmpeg(const CStdString& mType);
  ~CDVDVideoPPFFmpeg();


  void SetType(const CStdString& mType, bool deinterlace);
  void SetTarget(DVDVideoPicture *pPicture){ m_pTarget = pPicture; };
  bool Process   (DVDVideoPicture *pPicture);
  bool GetPicture(DVDVideoPicture *pPicture);

protected:
  CStdString m_sType;

  void *m_pContext;
  void *m_pMode;
  bool m_deinterlace;

  DVDVideoPicture m_FrameBuffer;
  DVDVideoPicture *m_pSource;
  DVDVideoPicture *m_pTarget;

  void Dispose();

  int m_iInitWidth, m_iInitHeight;
  bool CheckInit(int iWidth, int iHeight);
  bool CheckFrameBuffer(const DVDVideoPicture* pSource);

  DllPostProc m_dll;
};


