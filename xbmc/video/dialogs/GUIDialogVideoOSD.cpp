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

#include "GUIDialogVideoOSD.h"
#include "Application.h"
#include "GUIUserMessages.h"
#include "guilib/GUIComponent.h"
#include "guilib/GUIWindowManager.h"
#include "guilib/WindowIDs.h"
#include "input/ActionIDs.h"
#include "input/InputManager.h"
#include "ServiceBroker.h"

using namespace PVR;

CGUIDialogVideoOSD::CGUIDialogVideoOSD(void)
    : CGUIDialog(WINDOW_DIALOG_VIDEO_OSD, "VideoOSD.xml")
{
  m_loadType = KEEP_IN_MEMORY;
}

CGUIDialogVideoOSD::~CGUIDialogVideoOSD(void) = default;

void CGUIDialogVideoOSD::FrameMove()
{
  if (m_autoClosing)
  {
    // check for movement of mouse or a submenu open
    if (CServiceBroker::GetInputManager().IsMouseActive()
                           || CServiceBroker::GetGUI()->GetWindowManager().IsWindowActive(WINDOW_DIALOG_AUDIO_OSD_SETTINGS)
                           || CServiceBroker::GetGUI()->GetWindowManager().IsWindowActive(WINDOW_DIALOG_SUBTITLE_OSD_SETTINGS)
                           || CServiceBroker::GetGUI()->GetWindowManager().IsWindowActive(WINDOW_DIALOG_VIDEO_OSD_SETTINGS)
                           || CServiceBroker::GetGUI()->GetWindowManager().IsWindowActive(WINDOW_DIALOG_CMS_OSD_SETTINGS)
                           || CServiceBroker::GetGUI()->GetWindowManager().IsWindowActive(WINDOW_DIALOG_VIDEO_BOOKMARKS)
                           || CServiceBroker::GetGUI()->GetWindowManager().IsWindowActive(WINDOW_DIALOG_PVR_OSD_CHANNELS)
                           || CServiceBroker::GetGUI()->GetWindowManager().IsWindowActive(WINDOW_DIALOG_PVR_CHANNEL_GUIDE)
                           || CServiceBroker::GetGUI()->GetWindowManager().IsWindowActive(WINDOW_DIALOG_OSD_TELETEXT))
      // extend show time by original value
      SetAutoClose(m_showDuration);
  }
  CGUIDialog::FrameMove();
}

bool CGUIDialogVideoOSD::OnAction(const CAction &action)
{
  if (action.GetID() == ACTION_SHOW_OSD)
  {
    Close();
    return true;
  }

  return CGUIDialog::OnAction(action);
}

EVENT_RESULT CGUIDialogVideoOSD::OnMouseEvent(const CPoint &point, const CMouseEvent &event)
{
  if (event.m_id == ACTION_MOUSE_WHEEL_UP)
  {
    return g_application.OnAction(CAction(ACTION_ANALOG_SEEK_FORWARD, 0.5f)) ? EVENT_RESULT_HANDLED : EVENT_RESULT_UNHANDLED;
  }
  if (event.m_id == ACTION_MOUSE_WHEEL_DOWN)
  {
    return g_application.OnAction(CAction(ACTION_ANALOG_SEEK_BACK, 0.5f)) ? EVENT_RESULT_HANDLED : EVENT_RESULT_UNHANDLED;
  }

  return CGUIDialog::OnMouseEvent(point, event);
}

bool CGUIDialogVideoOSD::OnMessage(CGUIMessage& message)
{
  switch ( message.GetMessage() )
  {
  case GUI_MSG_VIDEO_MENU_STARTED:
    {
      // We have gone to the DVD menu, so close the OSD.
      Close();
    }
    break;
  case GUI_MSG_WINDOW_DEINIT:  // fired when OSD is hidden
    {
      // Remove our subdialogs if visible
      CGUIDialog *pDialog = CServiceBroker::GetGUI()->GetWindowManager().GetDialog(WINDOW_DIALOG_AUDIO_OSD_SETTINGS);
      if (pDialog && pDialog->IsDialogRunning())
        pDialog->Close(true);
      pDialog = CServiceBroker::GetGUI()->GetWindowManager().GetDialog(WINDOW_DIALOG_SUBTITLE_OSD_SETTINGS);
      if (pDialog && pDialog->IsDialogRunning())
        pDialog->Close(true);
    }
    break;
  }
  return CGUIDialog::OnMessage(message);
}

