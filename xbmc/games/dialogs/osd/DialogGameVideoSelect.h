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

#include "guilib/GUIDialog.h"

#include <memory>

class CFileItemList;
class CGUIViewControl;

namespace KODI
{
namespace RETRO
{
  class CGUIGameVideoHandle;
}

namespace GAME
{
  class CDialogGameVideoSelect : public CGUIDialog
  {
  public:
    ~CDialogGameVideoSelect() override;

    // implementation of CGUIControl via CGUIDialog
    bool OnMessage(CGUIMessage &message) override;

    // implementation of CGUIWindow via CGUIDialog
    void FrameMove() override;
    void OnDeinitWindow(int nextWindowID) override;

  protected:
    CDialogGameVideoSelect(int windowId);

    // implementation of CGUIWindow via CGUIDialog
    void OnWindowUnload() override;
    void OnWindowLoaded() override;
    void OnInitWindow() override;

    // Video select interface
    virtual std::string GetHeading() = 0;
    virtual void PreInit() = 0;
    virtual void GetItems(CFileItemList &items) = 0;
    virtual void OnItemFocus(unsigned int index) = 0;
    virtual unsigned int GetFocusedItem() const = 0;
    virtual void PostExit() = 0;

    void OnDescriptionChange(const std::string &description);

    std::shared_ptr<RETRO::CGUIGameVideoHandle> m_gameVideoHandle;

  private:
    void Update();
    void Clear();

    void OnRefreshList();

    void SaveSettings();

    void RegisterDialog();
    void UnregisterDialog();

    std::unique_ptr<CGUIViewControl> m_viewControl;
    std::unique_ptr<CFileItemList> m_vecItems;
  };
}
}
