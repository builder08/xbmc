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

#include "utils/Geometry.h"
#include <vector>

class CDirtyRegion : public CRect
{
public:
  explicit CDirtyRegion(const CRect &rect) : CRect(rect) { m_age = 0; }
  CDirtyRegion(float left, float top, float right, float bottom) : CRect(left, top, right, bottom) { m_age = 0; }
  CDirtyRegion() : CRect() { m_age = 0; }

  int UpdateAge() { return ++m_age; }
private:
  int m_age;
};

typedef std::vector<CDirtyRegion> CDirtyRegionList;
