/*
 *      Copyright (C) 2005-2013 Team XBMC
 *      http://xbmc.org
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
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#ifndef DIALOGS_GUIDIALOGMUSICOSD_H_INCLUDED
#define DIALOGS_GUIDIALOGMUSICOSD_H_INCLUDED
#include "GUIDialogMusicOSD.h"
#endif

#ifndef DIALOGS_GUILIB_GUIWINDOWMANAGER_H_INCLUDED
#define DIALOGS_GUILIB_GUIWINDOWMANAGER_H_INCLUDED
#include "guilib/GUIWindowManager.h"
#endif

#ifndef DIALOGS_GUILIB_KEY_H_INCLUDED
#define DIALOGS_GUILIB_KEY_H_INCLUDED
#include "guilib/Key.h"
#endif

#ifndef DIALOGS_INPUT_MOUSESTAT_H_INCLUDED
#define DIALOGS_INPUT_MOUSESTAT_H_INCLUDED
#include "input/MouseStat.h"
#endif

#ifndef DIALOGS_GUIUSERMESSAGES_H_INCLUDED
#define DIALOGS_GUIUSERMESSAGES_H_INCLUDED
#include "GUIUserMessages.h"
#endif

#ifndef DIALOGS_SETTINGS_SETTINGS_H_INCLUDED
#define DIALOGS_SETTINGS_SETTINGS_H_INCLUDED
#include "settings/Settings.h"
#endif

#ifndef DIALOGS_ADDONS_GUIWINDOWADDONBROWSER_H_INCLUDED
#define DIALOGS_ADDONS_GUIWINDOWADDONBROWSER_H_INCLUDED
#include "addons/GUIWindowAddonBrowser.h"
#endif


#define CONTROL_VIS_BUTTON       500
#define CONTROL_LOCK_BUTTON      501

CGUIDialogMusicOSD::CGUIDialogMusicOSD(void)
    : CGUIDialog(WINDOW_DIALOG_MUSIC_OSD, "MusicOSD.xml")
{
  m_loadType = KEEP_IN_MEMORY;
}

CGUIDialogMusicOSD::~CGUIDialogMusicOSD(void)
{
}

bool CGUIDialogMusicOSD::OnMessage(CGUIMessage &message)
{
  switch (message.GetMessage())
  {
  case GUI_MSG_CLICKED:
    {
      unsigned int iControl = message.GetSenderId();
      if (iControl == CONTROL_VIS_BUTTON)
      {
        CStdString addonID;
        if (CGUIWindowAddonBrowser::SelectAddonID(ADDON::ADDON_VIZ, addonID, true) == 1)
        {
          CSettings::Get().SetString("musicplayer.visualisation", addonID);
          CSettings::Get().Save();
          g_windowManager.SendMessage(GUI_MSG_VISUALISATION_RELOAD, 0, 0);
        }
      }
      else if (iControl == CONTROL_LOCK_BUTTON)
      {
        CGUIMessage msg(GUI_MSG_VISUALISATION_ACTION, 0, 0, ACTION_VIS_PRESET_LOCK);
        g_windowManager.SendMessage(msg);
      }
      return true;
    }
    break;
  }
  return CGUIDialog::OnMessage(message);
}

bool CGUIDialogMusicOSD::OnAction(const CAction &action)
{
  switch (action.GetID())
  {
  case ACTION_SHOW_OSD:
    Close();
    return true;

  default:
    break;
  }

  return CGUIDialog::OnAction(action);
}

void CGUIDialogMusicOSD::FrameMove()
{
  if (m_autoClosing)
  {
    // check for movement of mouse or a submenu open
    if (g_Mouse.IsActive() || g_windowManager.IsWindowActive(WINDOW_DIALOG_VIS_SETTINGS)
                           || g_windowManager.IsWindowActive(WINDOW_DIALOG_VIS_PRESET_LIST))
      // extend show time by original value
      SetAutoClose(m_showDuration);
  }
  CGUIDialog::FrameMove();
}
