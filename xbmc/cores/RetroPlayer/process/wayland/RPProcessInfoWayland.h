/*
 *  Copyright (c) 2020 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#pragma once

#include "cores/RetroPlayer/process/RPProcessInfo.h"

namespace KODI
{
namespace RETRO
{
  class CRPProcessInfoWayland : public CRPProcessInfo
  {
  public:
    CRPProcessInfoWayland();

    static CRPProcessInfo* Create();
    static void Register();
  };
}
}
