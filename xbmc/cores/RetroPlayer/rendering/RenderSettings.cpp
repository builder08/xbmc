/*
 *      Copyright (C) 2017-present Team Kodi
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

#include "RenderSettings.h"

using namespace KODI;
using namespace RETRO;

void CRenderSettings::Reset()
{
  m_geometry.Reset();
  m_videoSettings.Reset();
}

bool CRenderSettings::operator==(const CRenderSettings &rhs) const
{
  return m_geometry == rhs.m_geometry &&
         m_videoSettings == rhs.m_videoSettings;
}

bool CRenderSettings::operator<(const CRenderSettings &rhs) const
{
  if (m_geometry < rhs.m_geometry) return true;
  if (m_geometry > rhs.m_geometry) return false;

  if (m_videoSettings < rhs.m_videoSettings) return true;
  if (m_videoSettings > rhs.m_videoSettings) return false;

  return false;
}
