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
#include <queue>

#ifndef WAYLAND_WINDOWING_WAYLAND_EVENTLISTENER_H_INCLUDED
#define WAYLAND_WINDOWING_WAYLAND_EVENTLISTENER_H_INCLUDED
#include "windowing/wayland/EventListener.h"
#endif


class StubEventListener :
  public xbmc::IEventListener
{
public:

  StubEventListener();

  /* Returns front of event queue, otherwise throws */
  XBMC_Event FetchLastEvent();
  bool Focused();

private:

  void OnFocused();
  void OnUnfocused();
  void OnEvent(XBMC_Event &);

  bool m_focused;
  std::queue<XBMC_Event> m_events;
};
