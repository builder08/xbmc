#pragma once
/*
 *      Copyright (C) 2005-2011 Team XBMC
 *      http://www.xbmc.org
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
 *  along with XBMC; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#include "IDirtyRegionSolver.h"
#include "DirtyRegionSolvers.h"

#if defined(TARGET_DARWIN_IOS)
#define DEFAULT_BUFFERING 4
#else
#define DEFAULT_BUFFERING 3
#endif

class CDirtyRegionTracker
{
public:
  CDirtyRegionTracker();
  ~CDirtyRegionTracker();
  void SelectAlgorithm();
  void MarkDirtyRegion(const CDirtyRegion &region);

  const CDirtyRegionList &GetMarkedRegions() const;
  CDirtyRegionList GetDirtyRegions();
  void CleanMarkedRegions(int buffering = DEFAULT_BUFFERING);

private:
  CDirtyRegionList m_markedRegions;
  IDirtyRegionSolver *m_solver;
};
