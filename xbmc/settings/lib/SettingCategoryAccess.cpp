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

#include "SettingCategoryAccess.h"
#include "SettingConditions.h"
#include "SettingsManager.h"

bool CSettingCategoryAccessCondition::Check() const
{
  if (m_value.empty())
    return true;

  if (m_settingsManager == nullptr)
    return false;

  bool found = m_settingsManager->GetConditions().Check(m_value, "true");
  if (m_negated)
    return !found;

  return found;
}

bool CSettingCategoryAccessConditionCombination::Check() const
{
  if (m_operations.empty() && m_values.empty())
    return true;

  return CSettingConditionCombination::Check();
}

CSettingCategoryAccess::CSettingCategoryAccess(CSettingsManager *settingsManager /* = nullptr */)
  : CSettingCondition(settingsManager)
{
  m_operation = CBooleanLogicOperationPtr(new CSettingCategoryAccessConditionCombination(m_settingsManager));
}
