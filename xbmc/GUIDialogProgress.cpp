/*
 *      Copyright (C) 2005-2007 Team XboxMediaCenter
 *      http://www.xboxmediacenter.com
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
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#include "stdafx.h"
#include "GUIDialogProgress.h"
#include "GUIProgressControl.h"
#include "Application.h"


#define CONTROL_PROGRESS_BAR 20

CGUIDialogProgress::CGUIDialogProgress(void)
    : CGUIDialogBoxBase(WINDOW_DIALOG_PROGRESS, "DialogProgress.xml")
{
  m_bCanceled = false;
  m_iCurrent=0;
  m_iMax=0;
}

CGUIDialogProgress::~CGUIDialogProgress(void)
{

}

void CGUIDialogProgress::StartModal()
{
  CSingleLock lock(g_graphicsContext);

  CLog::DebugLog("DialogProgress::StartModal called %s", m_bRunning ? "(already running)!" : "");
  m_bCanceled = false;

  // set running before it's routed, else the auto-show code
  // could show it as well if we are in a different thread from
  // the main rendering thread (this should really be handled via
  // a thread message though IMO)
  m_bRunning = true;
  m_bModal = true;
  m_dialogClosing = false;
  m_gWindowManager.RouteToWindow(this);

  // active this window...
  CGUIMessage msg(GUI_MSG_WINDOW_INIT, 0, 0);
  OnMessage(msg);
  ShowProgressBar(false);

  lock.Leave();

  while (m_bRunning && IsAnimating(ANIM_TYPE_WINDOW_OPEN))
  {
    Progress();
    // we should have rendered at least once by now - if we haven't, then
    // we must be running from fullscreen video or similar where the
    // calling thread handles rendering (ie not main app thread) but
    // is waiting on this routine before rendering begins
    if (!m_hasRendered)
      break;
  }
}

void CGUIDialogProgress::Progress()
{
  if (m_bRunning)
  {
    m_gWindowManager.Process();
  }
}

void CGUIDialogProgress::ProgressKeys()
{
  if (m_bRunning)
  {
    g_application.FrameMove();
  }
}

bool CGUIDialogProgress::OnMessage(CGUIMessage& message)
{
  switch ( message.GetMessage() )
  {

  case GUI_MSG_CLICKED:
    {
      int iAction = message.GetParam1();
      if (1 || ACTION_SELECT_ITEM == iAction)
      {
        int iControl = message.GetSenderId();
        if (iControl == 10)
        {
          string strHeading = m_strHeading;
          strHeading.append(" : ");
          strHeading.append(g_localizeStrings.Get(16024));
          CGUIDialogBoxBase::SetHeading(strHeading);
          m_bCanceled = true;
          return true;
        }
      }
    }
    break;
  }
  return CGUIDialog::OnMessage(message);
}

bool CGUIDialogProgress::OnAction(const CAction &action)
{
  if (action.wID == ACTION_CLOSE_DIALOG || action.wID == ACTION_PREVIOUS_MENU)
  {
    m_bCanceled = true;
    return true;
  }
  return CGUIDialog::OnAction(action);
}

void CGUIDialogProgress::SetPercentage(int iPercentage)
{
  if (iPercentage < 0) iPercentage = 0;
  if (iPercentage > 100) iPercentage = 100;

  g_graphicsContext.Lock();
  CGUIProgressControl* pControl = (CGUIProgressControl*)GetControl(CONTROL_PROGRESS_BAR);
  if (pControl) pControl->SetPercentage((float)iPercentage);
  g_graphicsContext.Unlock();
}

void CGUIDialogProgress::SetProgressMax(int iMax)
{
  m_iMax=iMax;
  m_iCurrent=0;
}

void CGUIDialogProgress::SetProgressAdvance(int nSteps/*=1*/)
{
  m_iCurrent+=nSteps;

  if (m_iCurrent>m_iMax)
    m_iCurrent=0;

  SetPercentage((m_iCurrent*100)/m_iMax);
}

bool CGUIDialogProgress::Abort()
{
  return m_bRunning ? m_bCanceled : false;
}

void CGUIDialogProgress::ShowProgressBar(bool bOnOff)
{
  CGUIMessage msg(bOnOff ? GUI_MSG_VISIBLE : GUI_MSG_HIDDEN, GetID(), CONTROL_PROGRESS_BAR);
  if(OwningCriticalSection(g_graphicsContext))
    OnMessage(msg);
  else
    m_gWindowManager.SendThreadMessage(msg, m_dwWindowId);
}

void CGUIDialogProgress::SetHeading(const string& strLine)
{
  m_strHeading = strLine;
  CGUIDialogBoxBase::SetHeading(m_strHeading);
}

void CGUIDialogProgress::SetHeading(int iString)
{
  m_strHeading = g_localizeStrings.Get(iString);
  CGUIDialogBoxBase::SetHeading(m_strHeading);
}

