/*
 *  Copyright (c) 2020 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#pragma once

class IRunnable
{
public:
  virtual void Run()=0;
  virtual void Cancel() {};
  virtual ~IRunnable() = default;
};
