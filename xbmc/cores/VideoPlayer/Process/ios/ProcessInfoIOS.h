/*
 *  Copyright (c) 2020 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#pragma once

#include "../ProcessInfo.h"

namespace VIDEOPLAYER
{

class CProcessInfoIOS : public CProcessInfo
{
public:
  static CProcessInfo* Create();
  static void Register();

  void SetSwDeinterlacingMethods() override;
};

}
