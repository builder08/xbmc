#pragma once
/*
 *      Copyright (C) 2010-2013 Team XBMC
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

#include "PlatformConfig.h"
#if defined(HAS_MMAL)

#include "DVDVideoCodec.h"
#include "MMALCodec.h"

class CMMALVideo;
class CDVDVideoCodecMMAL : public CDVDVideoCodec
{
public:
  CDVDVideoCodecMMAL();
  virtual ~CDVDVideoCodecMMAL();

  // Required overrides
  virtual bool Open(CDVDStreamInfo &hints, CDVDCodecOptions &options);
  virtual void Dispose(void);
  virtual int  Decode(uint8_t *pData, int iSize, double dts, double pts);
  virtual void Reset(void);
  virtual bool GetPicture(DVDVideoPicture *pDvdVideoPicture);
  virtual bool ClearPicture(DVDVideoPicture* pDvdVideoPicture);
  virtual unsigned GetAllowedReferences();
  virtual void SetDropState(bool bDrop);
  virtual const char* GetName(void);
  virtual bool GetCodecStats(double &pts, int &droppedPics);

protected:
  MMALVideoPtr m_decoder;
};

#endif
