/*
 *      Copyright (C) 2012-present Team Kodi
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

#include "GameSettings.h"
#include "settings/lib/Setting.h"
#include "settings/Settings.h"

using namespace KODI;
using namespace GAME;

CGameSettings::CGameSettings(CSettings &settings) :
  m_settings(settings)
{
  m_settings.RegisterCallback(this, {
    CSettings::SETTING_GAMES_ENABLEREWIND,
    CSettings::SETTING_GAMES_REWINDTIME,
  });
}

CGameSettings::~CGameSettings()
{
  m_settings.UnregisterCallback(this);
}

void CGameSettings::OnSettingChanged(std::shared_ptr<const CSetting> setting)
{
  if (setting == nullptr)
    return;

  const std::string& settingId = setting->GetId();

  if (settingId == CSettings::SETTING_GAMES_ENABLEREWIND ||
      settingId == CSettings::SETTING_GAMES_REWINDTIME)
  {
    SetChanged();
    NotifyObservers(ObservableMessageSettingsChanged);
  }
}
