/*
 *  Copyright (c) 2020 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#pragma once

namespace PERIPHERALS
{
  class IEventScannerCallback
  {
  public:
    virtual ~IEventScannerCallback(void) = default;

    virtual void ProcessEvents(void) = 0;
  };
}
