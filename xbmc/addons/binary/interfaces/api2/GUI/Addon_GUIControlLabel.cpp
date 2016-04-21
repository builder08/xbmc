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

#include "Addon_GUIControlLabel.h"

#include "addons/Addon.h"
#include "addons/binary/ExceptionHandling.h"
#include "addons/binary/interfaces/api2/AddonInterfaceBase.h"
#include "addons/kodi-addon-dev-kit/include/kodi/api2/.internal/AddonLib_internal.hpp"
#include "guilib/GUILabelControl.h"
#include "guilib/GUIWindowManager.h"

using namespace ADDON;

namespace V2
{
namespace KodiAPI
{

namespace GUI
{
extern "C"
{

void CAddOnControl_Label::Init(struct CB_AddOnLib *interfaces)
{
  interfaces->GUI.Control.Label.SetVisible = CAddOnControl_Label::SetVisible;
  interfaces->GUI.Control.Label.SetLabel   = CAddOnControl_Label::SetLabel;
  interfaces->GUI.Control.Label.GetLabel   = CAddOnControl_Label::GetLabel;
}

void CAddOnControl_Label::SetVisible(void *addonData, void* handle, bool visible)
{
  try
  {
    if (!handle)
      throw ADDON::WrongValueException("CAddOnControl_Label - %s - invalid handler data", __FUNCTION__);

    static_cast<CGUILabelControl*>(handle)->SetVisible(visible);
  }
  HANDLE_ADDON_EXCEPTION
}

void CAddOnControl_Label::SetLabel(void *addonData, void* handle, const char *label)
{
  try
  {
    if (!handle)
      throw ADDON::WrongValueException("CAddOnControl_Label - %s - invalid handler data", __FUNCTION__);

    CGUILabelControl* pControl = static_cast<CGUILabelControl*>(handle);

    // create message
    CGUIMessage msg(GUI_MSG_LABEL_SET, pControl->GetParentID(), pControl->GetID());
    msg.SetLabel(label);

    // send message
    g_windowManager.SendThreadMessage(msg, pControl->GetParentID());
  }
  HANDLE_ADDON_EXCEPTION
}

void CAddOnControl_Label::GetLabel(void *addonData, void* handle, char &label, unsigned int &iMaxStringSize)
{
  try
  {
    if (!handle)
      throw ADDON::WrongValueException("CAddOnControl_Label - %s - invalid handler data", __FUNCTION__);

    std::string text = static_cast<CGUILabelControl*>(handle)->GetDescription(); // GetDescription = label
    strncpy(&label, text.c_str(), iMaxStringSize);
    iMaxStringSize = text.length();
  }
  HANDLE_ADDON_EXCEPTION
}

} /* extern "C" */
} /* namespace GUI */

} /* namespace KodiAPI */
} /* namespace V2 */
