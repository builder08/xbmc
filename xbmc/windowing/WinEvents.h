/*
 *  Copyright (c) 2020 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#pragma once

#include "XBMC_events.h"

class IWinEvents
{
public:
  virtual ~IWinEvents() = default;
  virtual bool  MessagePump() = 0;
};

