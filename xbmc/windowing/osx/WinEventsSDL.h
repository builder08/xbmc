/*
 *  Copyright (c) 2020 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#pragma once

#include "windowing/WinEvents.h"

#include <SDL/SDL_events.h>

class CWinEventsSDL : public IWinEvents
{
public:
  bool MessagePump() override;

private:
  static bool ProcessOSXShortcuts(SDL_Event& event);
};
