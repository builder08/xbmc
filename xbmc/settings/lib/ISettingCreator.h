#pragma once
/*
 *      Copyright (C) 2013-present Team Kodi
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

#include <memory>
#include <string>

class CSetting;
class CSettingsManager;

/*!
 \ingroup settings
 \brief Interface for creating a new setting of a custom setting type.
 */
class ISettingCreator
{
public:
  virtual ~ISettingCreator() = default;

  /*!
   \brief Creates a new setting of the given custom setting type.

   \param settingType string representation of the setting type
   \param settingId Identifier of the setting to be created
   \param settingsManager Reference to the settings manager
   \return A new setting object of the given (custom) setting type or nullptr if the setting type is unknown
   */
  virtual std::shared_ptr<CSetting> CreateSetting(const std::string &settingType, const std::string &settingId, CSettingsManager *settingsManager = nullptr) const = 0;
};
