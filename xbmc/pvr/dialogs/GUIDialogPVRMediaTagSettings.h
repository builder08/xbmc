/*
 *  Copyright (C) 2017-2018 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#pragma once

#include "settings/dialogs/GUIDialogSettingsManualBase.h"

#include <memory>
#include <string>
#include <vector>

class CFileItem;
class CSetting;

struct IntegerSettingOption;

namespace PVR
{
class CPVRMediaTag;

class CGUIDialogPVRMediaTagSettings : public CGUIDialogSettingsManualBase
{
public:
  CGUIDialogPVRMediaTagSettings();

  void SetMediaTag(const std::shared_ptr<CPVRMediaTag>& mediaTag);
  static bool CanEditMediaTag(const CFileItem& item);

protected:
  // implementation of ISettingCallback
  bool OnSettingChanging(const std::shared_ptr<const CSetting>& setting) override;
  void OnSettingChanged(const std::shared_ptr<const CSetting>& setting) override;

  // specialization of CGUIDialogSettingsBase
  bool AllowResettingSettings() const override { return false; }
  bool Save() override;
  void SetupView() override;

  // specialization of CGUIDialogSettingsManualBase
  void InitializeSettings() override;

private:
  std::shared_ptr<CPVRMediaTag> m_mediaTag;
  std::string m_strTitle;
  int m_iPlayCount = 0;
};
} // namespace PVR
