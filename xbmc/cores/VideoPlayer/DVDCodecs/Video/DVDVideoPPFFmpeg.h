#pragma once

/*
 *      Copyright (C) 2005-present Team Kodi
 *      This file is part of Kodi - https://kodi.tv
 *
 *  Kodi is free software: you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  Kodi is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Kodi. If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "DVDVideoCodec.h"
#include "cores/VideoPlayer/Process/VideoBuffer.h"
#include <string>

class CProcessInfo;

class CDVDVideoPPFFmpeg
{
public:

  explicit CDVDVideoPPFFmpeg(CProcessInfo &processInfo);
  ~CDVDVideoPPFFmpeg();

  void SetType(const std::string& mType, bool deinterlace);
  void Process(VideoPicture *pPicture);

protected:
  std::string m_sType;
  CProcessInfo &m_processInfo;

  void *m_pContext;
  void *m_pMode;
  bool m_deinterlace;

  void Dispose();

  int m_iInitWidth, m_iInitHeight;
  bool CheckInit(int iWidth, int iHeight);
  bool CheckFrameBuffer(const VideoPicture* pSource);
};


