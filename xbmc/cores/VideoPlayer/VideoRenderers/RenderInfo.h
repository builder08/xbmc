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

#include <cstddef>
#include <vector>
#include "cores/IPlayer.h"

extern "C" {
#include "libavutil/pixfmt.h"
}

struct CRenderInfo
{
  CRenderInfo()
  {
    Reset();
  }
  void Reset()
  {
    optimal_buffer_size = 0;
    max_buffer_size = 0;
    opaque_pointer = nullptr;
    m_deintMethods.clear();
    formats.clear();
  }
  unsigned int optimal_buffer_size;
  unsigned int max_buffer_size;
  // Supported pixel formats, can be called before configure
  std::vector<AVPixelFormat> formats;
  std::vector<EINTERLACEMETHOD> m_deintMethods;
  // Can be used for initialising video codec with information from renderer (e.g. a shared image pool)
  void *opaque_pointer;
};
