/*
 *  Copyright (c) 2020 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#pragma once

#include "utils/Temperature.h"

#include "platform/posix/CPUInfoPosix.h"

class CCPUInfoFreebsd : public CCPUInfoPosix
{
public:
  CCPUInfoFreebsd();
  ~CCPUInfoFreebsd() = default;

  int GetUsedPercentage() override;
  float GetCPUFrequency() override;
  bool GetTemperature(CTemperature& temperature) override;
};
