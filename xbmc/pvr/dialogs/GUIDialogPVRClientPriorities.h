#pragma once
/*
 *      Copyright (C) 2017-present Team Kodi
 *      This file is part of Kodi - https://kodi.tv
 *
 *  Kodi is free software: you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  Kodi is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Kodi. If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include <map>

#include "settings/dialogs/GUIDialogSettingsManualBase.h"

#include "pvr/addons/PVRClients.h"

class CSetting;

namespace PVR
{
  class CGUIDialogPVRClientPriorities : public CGUIDialogSettingsManualBase
  {
  public:
    CGUIDialogPVRClientPriorities();

  protected:
    // implementation of ISettingCallback
    void OnSettingChanged(std::shared_ptr<const CSetting> setting) override;

    // specialization of CGUIDialogSettingsBase
    std::string GetSettingsLabel(std::shared_ptr<ISetting> pSetting) override;
    bool AllowResettingSettings() const override { return false; }
    void Save() override;
    void SetupView() override;

    // specialization of CGUIDialogSettingsManualBase
    void InitializeSettings() override;

  private:
    CPVRClientMap m_clients;
    std::map<std::string, int> m_changedValues;
  };
} // namespace PVR
