#pragma once

/*
 *      Copyright (C) 2005-present Team Kodi
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
#include "utils/Stopwatch.h"
#include "view/GUIViewControl.h"

class CFileItemList;

class CGUIWindowLoginScreen : public CGUIWindow
{
public:
  CGUIWindowLoginScreen(void);
  ~CGUIWindowLoginScreen(void) override;
  bool OnMessage(CGUIMessage& message) override;
  bool OnAction(const CAction &action) override;
  bool OnBack(int actionID) override;
  void FrameMove() override;
  bool HasListItems() const override { return true; };
  CFileItemPtr GetCurrentListItem(int offset = 0) override;
  int GetViewContainerID() const override { return m_viewControl.GetCurrentControl(); };
  static void LoadProfile(unsigned int profile);

protected:
  void OnInitWindow() override;
  void OnWindowLoaded() override;
  void OnWindowUnload() override;
  void Update();
  void SetLabel(int iControl, const std::string& strLabel);

  bool OnPopupMenu(int iItem);
  CGUIViewControl m_viewControl;
  CFileItemList* m_vecItems;

  int m_iSelectedItem;
  CStopWatch watch;
};
