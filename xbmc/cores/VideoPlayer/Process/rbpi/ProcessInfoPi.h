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
#pragma once

#include "cores/IPlayer.h"
#include "cores/VideoPlayer/Process/ProcessInfo.h"

namespace MMAL {
  class CMMALYUVBuffer;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

class CVideoBufferManagerPi : public CVideoBufferManager
{
public:
  CVideoBufferManagerPi();
  void RegisterPool(std::shared_ptr<IVideoBufferPool> pool);
  void ReleasePools();
  CVideoBuffer* Get(AVPixelFormat format, int width, int height);
  CVideoBuffer* Get(AVPixelFormat format, int size);
  void SetDimensions(int width, int height, const int (&strides)[YuvImage::MAX_PLANES]);

protected:
};

class CProcessInfoPi : public CProcessInfo
{
public:
  CProcessInfoPi();
  static CProcessInfo* Create();
  static void Register();
  EINTERLACEMETHOD GetFallbackDeintMethod() override;
  bool AllowDTSHDDecode() override;

//protected:
};
