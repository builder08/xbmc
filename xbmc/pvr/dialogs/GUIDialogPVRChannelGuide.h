#pragma once
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

#include "guilib/GUIDialog.h"
#include "view/GUIViewControl.h"

#include "pvr/PVRTypes.h"

class CFileItemList;

namespace PVR
{
  class CGUIDialogPVRChannelGuide : public CGUIDialog
  {
  public:
    CGUIDialogPVRChannelGuide(void);
    ~CGUIDialogPVRChannelGuide(void) override;
    bool OnMessage(CGUIMessage& message) override;
    void OnWindowLoaded() override;
    void OnWindowUnload() override;

    void Open(const CPVRChannelPtr &channel);

  protected:
    void OnInitWindow() override;
    void OnDeinitWindow(int nextWindowID) override;

    CGUIControl *GetFirstFocusableControl(int id) override;

    std::unique_ptr<CFileItemList> m_vecItems;
    CGUIViewControl m_viewControl;

  private:
    void ShowInfo(int iItem);
    void Clear();

    CPVRChannelPtr m_channel;
  };
}
