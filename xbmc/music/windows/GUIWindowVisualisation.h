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

#ifndef WINDOWS_GUILIB_GUIWINDOW_H_INCLUDED
#define WINDOWS_GUILIB_GUIWINDOW_H_INCLUDED
#include "guilib/GUIWindow.h"
#endif

#ifndef WINDOWS_MUSIC_TAGS_MUSICINFOTAG_H_INCLUDED
#define WINDOWS_MUSIC_TAGS_MUSICINFOTAG_H_INCLUDED
#include "music/tags/MusicInfoTag.h"
#endif

#ifndef WINDOWS_UTILS_STOPWATCH_H_INCLUDED
#define WINDOWS_UTILS_STOPWATCH_H_INCLUDED
#include "utils/Stopwatch.h"
#endif


class CGUIWindowVisualisation :
      public CGUIWindow
{
public:
  CGUIWindowVisualisation(void);
  virtual bool OnMessage(CGUIMessage& message);
  virtual bool OnAction(const CAction &action);
  virtual void FrameMove();
protected:
  virtual EVENT_RESULT OnMouseEvent(const CPoint &point, const CMouseEvent &event);

  CStopWatch m_initTimer;
  CStopWatch m_lockedTimer;
  bool m_bShowPreset;
  MUSIC_INFO::CMusicInfoTag m_tag;    // current tag info, for finding when the info manager updates
};

