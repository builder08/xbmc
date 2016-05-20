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

#include "Addon_GUIControlRendering.h"
#include "Addon_GUIGeneral.h"

#include "addons/Addon.h"
#include "addons/binary/ExceptionHandling.h"
#include "addons/binary/interfaces/api2/AddonInterfaceBase.h"
#include "addons/kodi-addon-dev-kit/include/kodi/api2/.internal/AddonLib_internal.hpp"
#include "guilib/GUIRenderingControl.h"

using namespace ADDON;

namespace V2
{
namespace KodiAPI
{

namespace GUI
{
extern "C"
{

void CAddOnControl_Rendering::Init(struct CB_AddOnLib *interfaces)
{
  interfaces->GUI.Control.Rendering.SetCallbacks = CAddOnControl_Rendering::SetCallbacks;
  interfaces->GUI.Control.Rendering.Delete = CAddOnControl_Rendering::Delete;
}

void CAddOnControl_Rendering::SetCallbacks(
                            void *addonData, void* handle, void* clienthandle,
                            bool (*createCB)(void*,int,int,int,int,void*),
                            void (*renderCB)(void*), void (*stopCB)(void*), bool (*dirtyCB)(void*))
{
  try
  {
    if (!addonData || !handle)
      throw ADDON::WrongValueException("CAddOnControl_Rendering - %s - invalid handler data", __FUNCTION__);

    CGUIAddonRenderingControl* pAddonControl = static_cast<CGUIAddonRenderingControl *>(handle);

    CAddOnGUIGeneral::Lock();
    pAddonControl->m_clientHandle = clienthandle;
    pAddonControl->CBCreate = createCB;
    pAddonControl->CBRender = renderCB;
    pAddonControl->CBStop = stopCB;
    pAddonControl->CBDirty = dirtyCB;
    CAddOnGUIGeneral::Unlock();

    pAddonControl->m_pControl->InitCallback(pAddonControl);
  }
  HANDLE_ADDON_EXCEPTION
}

void CAddOnControl_Rendering::Delete(void *addonData, void* handle)
{
  try
  {
    if (!addonData || !handle)
      throw ADDON::WrongValueException("CAddOnControl_Rendering - %s - invalid handler data", __FUNCTION__);

    CAddOnGUIGeneral::Lock();
    static_cast<CGUIAddonRenderingControl*>(handle)->Delete();
    CAddOnGUIGeneral::Unlock();
  }
  HANDLE_ADDON_EXCEPTION
}


CGUIAddonRenderingControl::CGUIAddonRenderingControl(CGUIRenderingControl *pControl)
  : CBCreate{nullptr},
    CBRender{nullptr},
    CBStop{nullptr},
    CBDirty{nullptr},
    m_clientHandle{nullptr},
    m_pControl{pControl},
    m_refCount{1}
{
}

bool CGUIAddonRenderingControl::Create(int x, int y, int w, int h, void *device)
{
  try
  {
    if (CBCreate)
    {
      if (CBCreate(m_clientHandle, x, y, w, h, device))
      {
        ++m_refCount;
        return true;
      }
    }
  }
  HANDLE_ADDON_EXCEPTION

  return false;
}

void CGUIAddonRenderingControl::Render()
{
  try
  {
    if (CBRender)
    {
      CBRender(m_clientHandle);
    }
  }
  HANDLE_ADDON_EXCEPTION
}

void CGUIAddonRenderingControl::Stop()
{
  try
  {
    if (CBStop)
    {
      CBStop(m_clientHandle);
    }
  }
  HANDLE_ADDON_EXCEPTION

  --m_refCount;
  if (m_refCount <= 0)
    delete this;
}

void CGUIAddonRenderingControl::Delete()
{
  --m_refCount;
  if (m_refCount <= 0)
    delete this;
}

bool CGUIAddonRenderingControl::IsDirty()
{
  bool ret = true;
  try
  {
    if (CBDirty)
    {
      ret = CBDirty(m_clientHandle);
    }
  }
  HANDLE_ADDON_EXCEPTION

  return ret;
}

} /* extern "C" */
} /* namespace GUI */

} /* namespace KodiAPI */
} /* namespace V2 */
