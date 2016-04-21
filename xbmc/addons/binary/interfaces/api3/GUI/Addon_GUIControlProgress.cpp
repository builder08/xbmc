/*
 *      Copyright (C) 2015-2016 Team KODI
 *      http://kodi.tv
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with KODI; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include "Addon_GUIControlProgress.h"
#include "addons/binary/interfaces/api3/AddonInterfaceBase.h"
#include "addons/binary/interfaces/api2/GUI/Addon_GUIControlProgress.h"
#include "addons/kodi-addon-dev-kit/include/kodi/api3/.internal/AddonLib_internal.hpp"

namespace V3
{
namespace KodiAPI
{

namespace GUI
{
extern "C"
{

void CAddOnControl_Progress::Init(struct CB_AddOnLib *interfaces)
{
  interfaces->GUI.Control.Progress.SetVisible    = V2::KodiAPI::GUI::CAddOnControl_Progress::SetVisible;
  interfaces->GUI.Control.Progress.SetPercentage = V2::KodiAPI::GUI::CAddOnControl_Progress::SetPercentage;
  interfaces->GUI.Control.Progress.GetPercentage = V2::KodiAPI::GUI::CAddOnControl_Progress::GetPercentage;
}

} /* extern "C" */
} /* namespace GUI */

} /* namespace KodiAPI */
} /* namespace V3 */
