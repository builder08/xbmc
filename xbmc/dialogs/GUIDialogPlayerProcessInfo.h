/*
 *  Copyright (c) 2020 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#pragma once

#include "guilib/GUIDialog.h"

class CGUIDialogPlayerProcessInfo : public CGUIDialog
{
public:
  CGUIDialogPlayerProcessInfo(void);
  ~CGUIDialogPlayerProcessInfo(void) override;

  bool OnAction(const CAction &action) override;
};
