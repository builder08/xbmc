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

#include "GUIWindowPVRTimerRules.h"

#include "FileItem.h"
#include "utils/URIUtils.h"

#include "pvr/timers/PVRTimers.h"

using namespace PVR;

CGUIWindowPVRTVTimerRules::CGUIWindowPVRTVTimerRules() :
  CGUIWindowPVRTimersBase(false, WINDOW_TV_TIMER_RULES, "MyPVRTimers.xml")
{
}

std::string CGUIWindowPVRTVTimerRules::GetDirectoryPath()
{
  const std::string basePath(CPVRTimersPath(false, true).GetPath());
  return URIUtils::PathHasParent(m_vecItems->GetPath(), basePath) ? m_vecItems->GetPath() : basePath;
}

CGUIWindowPVRRadioTimerRules::CGUIWindowPVRRadioTimerRules() :
CGUIWindowPVRTimersBase(true, WINDOW_RADIO_TIMER_RULES, "MyPVRTimers.xml")
{
}

std::string CGUIWindowPVRRadioTimerRules::GetDirectoryPath()
{
  const std::string basePath(CPVRTimersPath(true, true).GetPath());
  return URIUtils::PathHasParent(m_vecItems->GetPath(), basePath) ? m_vecItems->GetPath() : basePath;
}
