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

#include "SettingUrlEncodedString.h"
#include "URL.h"
#include "settings/lib/SettingsManager.h"

namespace ADDON
{

CSettingUrlEncodedString::CSettingUrlEncodedString(const std::string &id, CSettingsManager *settingsManager /* = NULL */)
  : CSettingString(id, settingsManager)
{ }

CSettingUrlEncodedString::CSettingUrlEncodedString(const std::string &id, int label, const std::string &value, CSettingsManager *settingsManager /* = NULL */)
  : CSettingString(id, label, value, settingsManager)
{ }

CSettingUrlEncodedString::CSettingUrlEncodedString(const std::string &id, const CSettingUrlEncodedString &setting)
  : CSettingString(id, setting)
{ }

std::string CSettingUrlEncodedString::GetDecodedValue() const
{
  return CURL::Decode(CSettingString::GetValue());
}

bool CSettingUrlEncodedString::SetDecodedValue(const std::string &decodedValue)
{
  return CSettingString::SetValue(CURL::Encode(decodedValue));
}

} /* namespace ADDON */
