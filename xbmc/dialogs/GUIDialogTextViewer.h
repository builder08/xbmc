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

class CGUIDialogTextViewer :
      public CGUIDialog
{
public:
  CGUIDialogTextViewer(void);
  ~CGUIDialogTextViewer(void) override;
  bool OnMessage(CGUIMessage& message) override;
  void SetText(const std::string& strText) { m_strText = strText; }
  void SetHeading(const std::string& strHeading) { m_strHeading = strHeading; }
  void UseMonoFont(bool use);

  //! \brief Load a file into memory and show in dialog.
  //! \param path Path to file
  //! \param useMonoFont True to use monospace font
  static void ShowForFile(const std::string& path, bool useMonoFont);
protected:
  void OnDeinitWindow(int nextWindowID) override;
  bool OnAction(const CAction &action) override;

  std::string m_strText;
  std::string m_strHeading;
  bool m_mono = false;

  void SetText();
  void SetHeading();
};

