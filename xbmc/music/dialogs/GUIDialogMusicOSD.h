/*
 *  Copyright (c) 2020 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#pragma once

#include "guilib/GUIDialog.h"

class CGUIDialogMusicOSD :
      public CGUIDialog
{
public:
  CGUIDialogMusicOSD(void);
  ~CGUIDialogMusicOSD(void) override;
  bool OnMessage(CGUIMessage &message) override;
  bool OnAction(const CAction &action) override;
  void FrameMove() override;
};
