/*
 *      Copyright (C) 2014-present Team Kodi
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
#pragma once

#include "addons/RepositoryUpdater.h"
#include "guilib/GUIDialog.h"

namespace KODI
{
namespace GAME
{
  class IControllerList;
  class IFeatureList;

  class CGUIControllerWindow : public CGUIDialog
  {
  public:
    CGUIControllerWindow(void);
    virtual ~CGUIControllerWindow(void);

    // implementation of CGUIControl via CGUIDialog
    virtual void DoProcess(unsigned int currentTime, CDirtyRegionList &dirtyregions) override;
    virtual bool OnMessage(CGUIMessage& message) override;

  protected:
    // implementation of CGUIWindow via CGUIDialog
    virtual void OnInitWindow(void) override;
    virtual void OnDeinitWindow(int nextWindowID) override;

  private:
    void OnControllerFocused(unsigned int controllerIndex);
    void OnControllerSelected(unsigned int controllerIndex);
    void OnFeatureFocused(unsigned int featureIndex);
    void OnFeatureSelected(unsigned int featureIndex);
    void OnEvent(const ADDON::CRepositoryUpdater::RepositoryUpdated& event);
    void UpdateButtons(void);

    // Action for the available button
    void GetMoreControllers(void);
    void ResetController(void);
    void ShowHelp(void);
    void ShowButtonCaptureDialog(void);

    IControllerList* m_controllerList;
    IFeatureList*    m_featureList;
  };
}
}
