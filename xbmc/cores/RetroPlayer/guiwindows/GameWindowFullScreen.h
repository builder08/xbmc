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
#pragma once

#include "guilib/GUIWindow.h"

class CGUIDialog;

namespace KODI
{
namespace RETRO
{
  class CGameWindowFullScreenText;
  class CGUIRenderHandle;

  class CGameWindowFullScreen : public CGUIWindow
  {
  public:
    CGameWindowFullScreen();
    ~CGameWindowFullScreen() override;

    // implementation of CGUIControl via CGUIWindow
    void Process(unsigned int currentTime, CDirtyRegionList &dirtyregion) override;
    void Render() override;
    void RenderEx() override;
    bool OnAction(const CAction &action) override;
    bool OnMessage(CGUIMessage& message) override;

    // implementation of CGUIWindow
    void FrameMove() override;
    void ClearBackground() override;
    bool HasVisibleControls() override;
    void OnWindowLoaded() override;
    void OnDeinitWindow(int nextWindowID) override;

  protected:
    // implementation of CGUIWindow
    void OnInitWindow() override;

  private:
    void ToggleOSD();
    void TriggerOSD();
    CGUIDialog *GetOSD();

    void RegisterWindow();
    void UnregisterWindow();

    // GUI parameters
    std::unique_ptr<CGameWindowFullScreenText> m_fullscreenText;

    // Rendering parameters
    std::shared_ptr<CGUIRenderHandle> m_renderHandle;
  };
}
}
