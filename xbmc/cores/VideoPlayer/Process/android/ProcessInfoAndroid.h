/*
 *  Copyright (c) 2020 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#pragma once

#include "cores/IPlayer.h"
#include "cores/VideoPlayer/Process/ProcessInfo.h"

namespace VIDEOPLAYER
{
  class CProcessInfoAndroid : public CProcessInfo
  {
  public:
    CProcessInfoAndroid() = default;
    static CProcessInfo* Create();
    static void Register();
    EINTERLACEMETHOD GetFallbackDeintMethod() override;

  //protected:
  };
}
