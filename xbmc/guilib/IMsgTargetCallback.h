/*
 *  Copyright (C) 2005-2018 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#pragma once

#include "GUIMessage.h"

class IMsgTargetCallback
{
public:
  virtual bool OnMessage(CGUIMessage& message) = 0;
  virtual ~IMsgTargetCallback() = default;
};
