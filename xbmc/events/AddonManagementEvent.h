/*
 *  Copyright (c) 2020 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#pragma once

#include "events/AddonEvent.h"

class CAddonManagementEvent : public CAddonEvent
{
public:
  CAddonManagementEvent(ADDON::AddonPtr addon, const CVariant& description);
  CAddonManagementEvent(ADDON::AddonPtr addon, const CVariant& description, const CVariant& details);
  CAddonManagementEvent(ADDON::AddonPtr addon, const CVariant& description, const CVariant& details, const CVariant& executionLabel);
  CAddonManagementEvent(ADDON::AddonPtr addon, EventLevel level, const CVariant& description);
  CAddonManagementEvent(ADDON::AddonPtr addon, EventLevel level, const CVariant& description, const CVariant& details);
  CAddonManagementEvent(ADDON::AddonPtr addon, EventLevel level, const CVariant& description, const CVariant& details, const CVariant& executionLabel);
  ~CAddonManagementEvent() override = default;

  const char* GetType() const override { return "AddonManagementEvent"; }
  std::string GetExecutionLabel() const override;

  bool CanExecute() const override { return m_addon != NULL; }
  bool Execute() const override;
};
