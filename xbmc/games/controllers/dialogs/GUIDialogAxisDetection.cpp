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

#include "GUIDialogAxisDetection.h"
#include "guilib/LocalizeStrings.h"
#include "input/joysticks/interfaces/IButtonMap.h"
#include "input/joysticks/DriverPrimitive.h"
#include "input/joysticks/JoystickTranslator.h"
#include "utils/StringUtils.h"

#include <algorithm>

using namespace KODI;
using namespace GAME;

std::string CGUIDialogAxisDetection::GetDialogText()
{
  // "Press all analog buttons now to detect them:[CR][CR]%s"
  std::string dialogText = g_localizeStrings.Get(35020);

  std::vector<std::string> primitives;

  for (const auto& axisEntry : m_detectedAxes)
  {
    JOYSTICK::CDriverPrimitive axis(axisEntry.second, 0, JOYSTICK::SEMIAXIS_DIRECTION::POSITIVE, 1);
    primitives.emplace_back(JOYSTICK::CJoystickTranslator::GetPrimitiveName(axis));
  }

  return StringUtils::Format(dialogText.c_str(), StringUtils::Join(primitives, " | ").c_str());
}

std::string CGUIDialogAxisDetection::GetDialogHeader()
{
  return g_localizeStrings.Get(35058); // "Controller Configuration"
}

bool CGUIDialogAxisDetection::MapPrimitiveInternal(JOYSTICK::IButtonMap* buttonMap,
                                                   IKeymap* keymap,
                                                   const JOYSTICK::CDriverPrimitive& primitive)
{
  if (primitive.Type() == JOYSTICK::PRIMITIVE_TYPE::SEMIAXIS)
    AddAxis(buttonMap->DeviceName(), primitive.Index());

  return true;
}

void CGUIDialogAxisDetection::OnLateAxis(const JOYSTICK::IButtonMap* buttonMap, unsigned int axisIndex)
{
  AddAxis(buttonMap->DeviceName(), axisIndex);
}

void CGUIDialogAxisDetection::AddAxis(const std::string& deviceName, unsigned int axisIndex)
{
  auto it = std::find_if(m_detectedAxes.begin(), m_detectedAxes.end(),
    [&deviceName, axisIndex](const AxisEntry& axis)
    {
      return axis.first == deviceName &&
             axis.second == axisIndex;
    });

  if (it == m_detectedAxes.end())
  {
    m_detectedAxes.emplace_back(std::make_pair(deviceName, axisIndex));
    m_captureEvent.Set();
  }
}
