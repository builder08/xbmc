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

#ifndef WINDOWS_SYSTEM_H_INCLUDED
#define WINDOWS_SYSTEM_H_INCLUDED
#include "system.h"
#endif

#ifndef WINDOWS_GUIWINDOWSETTINGSSCREENCALIBRATION_H_INCLUDED
#define WINDOWS_GUIWINDOWSETTINGSSCREENCALIBRATION_H_INCLUDED
#include "GUIWindowSettingsScreenCalibration.h"
#endif

#ifndef WINDOWS_GUILIB_GUIMOVERCONTROL_H_INCLUDED
#define WINDOWS_GUILIB_GUIMOVERCONTROL_H_INCLUDED
#include "guilib/GUIMoverControl.h"
#endif

#ifndef WINDOWS_GUILIB_GUIRESIZECONTROL_H_INCLUDED
#define WINDOWS_GUILIB_GUIRESIZECONTROL_H_INCLUDED
#include "guilib/GUIResizeControl.h"
#endif

#ifdef HAS_VIDEO_PLAYBACK
#include "cores/VideoRenderers/RenderManager.h"
#endif
#ifndef WINDOWS_APPLICATION_H_INCLUDED
#define WINDOWS_APPLICATION_H_INCLUDED
#include "Application.h"
#endif

#ifndef WINDOWS_SETTINGS_DISPLAYSETTINGS_H_INCLUDED
#define WINDOWS_SETTINGS_DISPLAYSETTINGS_H_INCLUDED
#include "settings/DisplaySettings.h"
#endif

#ifndef WINDOWS_SETTINGS_SETTINGS_H_INCLUDED
#define WINDOWS_SETTINGS_SETTINGS_H_INCLUDED
#include "settings/Settings.h"
#endif

#ifndef WINDOWS_GUILIB_GUIWINDOWMANAGER_H_INCLUDED
#define WINDOWS_GUILIB_GUIWINDOWMANAGER_H_INCLUDED
#include "guilib/GUIWindowManager.h"
#endif

#ifndef WINDOWS_DIALOGS_GUIDIALOGYESNO_H_INCLUDED
#define WINDOWS_DIALOGS_GUIDIALOGYESNO_H_INCLUDED
#include "dialogs/GUIDialogYesNo.h"
#endif

#ifndef WINDOWS_GUILIB_KEY_H_INCLUDED
#define WINDOWS_GUILIB_KEY_H_INCLUDED
#include "guilib/Key.h"
#endif

#ifndef WINDOWS_GUILIB_LOCALIZESTRINGS_H_INCLUDED
#define WINDOWS_GUILIB_LOCALIZESTRINGS_H_INCLUDED
#include "guilib/LocalizeStrings.h"
#endif

#ifndef WINDOWS_UTILS_LOG_H_INCLUDED
#define WINDOWS_UTILS_LOG_H_INCLUDED
#include "utils/log.h"
#endif

#ifndef WINDOWS_UTILS_STRINGUTILS_H_INCLUDED
#define WINDOWS_UTILS_STRINGUTILS_H_INCLUDED
#include "utils/StringUtils.h"
#endif

#ifndef WINDOWS_WINDOWING_WINDOWINGFACTORY_H_INCLUDED
#define WINDOWS_WINDOWING_WINDOWINGFACTORY_H_INCLUDED
#include "windowing/WindowingFactory.h"
#endif


using namespace std;

#define CONTROL_LABEL_ROW1  2
#define CONTROL_LABEL_ROW2  3
#define CONTROL_TOP_LEFT  8
#define CONTROL_BOTTOM_RIGHT 9
#define CONTROL_SUBTITLES  10
#define CONTROL_PIXEL_RATIO  11
#define CONTROL_VIDEO   20
#define CONTROL_NONE   0

CGUIWindowSettingsScreenCalibration::CGUIWindowSettingsScreenCalibration(void)
    : CGUIWindow(WINDOW_SCREEN_CALIBRATION, "SettingsScreenCalibration.xml")
{
  m_iCurRes = 0;
  m_iControl = 0;
  m_fPixelRatioBoxHeight = 0.0f;
  m_needsScaling = false;         // we handle all the scaling
}

CGUIWindowSettingsScreenCalibration::~CGUIWindowSettingsScreenCalibration(void)
{}


bool CGUIWindowSettingsScreenCalibration::OnAction(const CAction &action)
{
  switch (action.GetID())
  {
  case ACTION_CALIBRATE_SWAP_ARROWS:
    {
      NextControl();
      return true;
    }
    break;

  case ACTION_CALIBRATE_RESET:
    {
      CGUIDialogYesNo* pDialog = (CGUIDialogYesNo*)g_windowManager.GetWindow(WINDOW_DIALOG_YES_NO);
      pDialog->SetHeading(20325);
      CStdString strText = StringUtils::Format(g_localizeStrings.Get(20326).c_str(), g_graphicsContext.GetResInfo(m_Res[m_iCurRes]).strMode.c_str());
      pDialog->SetLine(0, strText);
      pDialog->SetLine(1, 20327);
      pDialog->SetChoice(0, 222);
      pDialog->SetChoice(1, 186);
      pDialog->DoModal();
      if (pDialog->IsConfirmed())
      {
        g_graphicsContext.ResetScreenParameters(m_Res[m_iCurRes]);
        ResetControls();
      }
      return true;
    }
    break;

  case ACTION_CHANGE_RESOLUTION:
    // choose the next resolution in our list
    {
      m_iCurRes = (m_iCurRes+1) % m_Res.size();
      g_graphicsContext.SetVideoResolution(m_Res[m_iCurRes]);
      ResetControls();
      return true;
    }
    break;
  }
  return CGUIWindow::OnAction(action); // base class to handle basic movement etc.
}

void CGUIWindowSettingsScreenCalibration::AllocResources(bool forceLoad)
{
  CGUIWindow::AllocResources(forceLoad);
}

void CGUIWindowSettingsScreenCalibration::FreeResources(bool forceUnload)
{
  CGUIWindow::FreeResources(forceUnload);
}


bool CGUIWindowSettingsScreenCalibration::OnMessage(CGUIMessage& message)
{
  switch ( message.GetMessage() )
  {
  case GUI_MSG_WINDOW_DEINIT:
    {
      CDisplaySettings::Get().UpdateCalibrations();
      CSettings::Get().Save();
      g_graphicsContext.SetCalibrating(false);
      g_windowManager.ShowOverlay(OVERLAY_STATE_SHOWN);
      // reset our screen resolution to what it was initially
      g_graphicsContext.SetVideoResolution(CDisplaySettings::Get().GetCurrentResolution());
      // Inform the player so we can update the resolution
#ifdef HAS_VIDEO_PLAYBACK
      g_renderManager.Update();
#endif
      g_windowManager.SendMessage(GUI_MSG_NOTIFY_ALL, 0, 0, GUI_MSG_WINDOW_RESIZE);
    }
    break;

  case GUI_MSG_WINDOW_INIT:
    {
      CGUIWindow::OnMessage(message);
      g_windowManager.ShowOverlay(OVERLAY_STATE_HIDDEN);
      g_graphicsContext.SetCalibrating(true);

      // Get the allowable resolutions that we can calibrate...
      m_Res.clear();
      if (g_application.m_pPlayer->IsPlayingVideo())
      { // don't allow resolution switching if we are playing a video

#ifdef HAS_VIDEO_PLAYBACK
        RESOLUTION res = g_renderManager.GetResolution();
        g_graphicsContext.SetVideoResolution(res);
        // Inform the renderer so we can update the resolution
        g_renderManager.Update();
#endif

        m_iCurRes = 0;
        m_Res.push_back(g_graphicsContext.GetVideoResolution());
        SET_CONTROL_VISIBLE(CONTROL_VIDEO);
      }
      else
      {
        SET_CONTROL_HIDDEN(CONTROL_VIDEO);
        m_iCurRes = (unsigned int)-1;
        g_graphicsContext.GetAllowedResolutions(m_Res);
        // find our starting resolution
        m_iCurRes = FindCurrentResolution();
      }
      if (m_iCurRes==(unsigned int)-1)
      {
        CLog::Log(LOGERROR, "CALIBRATION: Reported current resolution: %d", (int)g_graphicsContext.GetVideoResolution());
        CLog::Log(LOGERROR, "CALIBRATION: Could not determine current resolution, falling back to default");
        m_iCurRes = 0;
      }

      // Setup the first control
      m_iControl = CONTROL_TOP_LEFT;
      ResetControls();
      return true;
    }
    break;
  case GUI_MSG_CLICKED:
    {
      // clicked - change the control...
      NextControl();
    }
    break;
  case GUI_MSG_NOTIFY_ALL:
    {
      if (message.GetParam1() == GUI_MSG_WINDOW_RESIZE)
      {
        m_iCurRes = FindCurrentResolution();
      }
    }
    break;
  }
  return CGUIWindow::OnMessage(message);
}

unsigned int CGUIWindowSettingsScreenCalibration::FindCurrentResolution()
{
  RESOLUTION curRes = g_graphicsContext.GetVideoResolution();
  for (unsigned int i = 0; i < m_Res.size(); i++)
  {
    // If it's a CUSTOM (monitor) resolution, then g_graphicsContext.GetAllowedResolutions()
    // returns just one entry with CUSTOM in it. Update that entry to point to the current
    // CUSTOM resolution.
    if (curRes>=RES_CUSTOM)
    {
      if (m_Res[i]==RES_CUSTOM)
      {
        m_Res[i] = curRes;
        return i;
      }
    }
    else if (m_Res[i] == g_graphicsContext.GetVideoResolution())
      return i;
  }
  return 0;
}

void CGUIWindowSettingsScreenCalibration::NextControl()
{ // set the old control invisible and not focused, and choose the next control
  CGUIControl *pControl = (CGUIControl *)GetControl(m_iControl);
  if (pControl)
  {
    pControl->SetVisible(false);
    pControl->SetFocus(false);
  }
  // switch to the next control
  m_iControl++;
  if (m_iControl > CONTROL_PIXEL_RATIO)
    m_iControl = CONTROL_TOP_LEFT;
  // enable the new control
  EnableControl(m_iControl);
}

void CGUIWindowSettingsScreenCalibration::EnableControl(int iControl)
{
  SET_CONTROL_VISIBLE(CONTROL_TOP_LEFT);
  SET_CONTROL_VISIBLE(CONTROL_BOTTOM_RIGHT);
  SET_CONTROL_VISIBLE(CONTROL_SUBTITLES);
  SET_CONTROL_VISIBLE(CONTROL_PIXEL_RATIO);
  SET_CONTROL_FOCUS(iControl, 0);
}

void CGUIWindowSettingsScreenCalibration::ResetControls()
{
  // disable the video control, so that our other controls take mouse clicks etc.
  CONTROL_DISABLE(CONTROL_VIDEO);
  // disable the UI calibration for our controls
  // and set their limits
  // also, set them to invisible if they don't have focus
  CGUIMoverControl *pControl = (CGUIMoverControl*)GetControl(CONTROL_TOP_LEFT);
  RESOLUTION_INFO info = g_graphicsContext.GetResInfo(m_Res[m_iCurRes]);
  if (pControl)
  {
    pControl->SetLimits( -info.iWidth / 4,
                         -info.iHeight / 4,
                         info.iWidth / 4,
                         info.iHeight / 4);
    pControl->SetPosition((float)info.Overscan.left,
                          (float)info.Overscan.top);
    pControl->SetLocation(info.Overscan.left,
                          info.Overscan.top, false);
  }
  pControl = (CGUIMoverControl*)GetControl(CONTROL_BOTTOM_RIGHT);
  if (pControl)
  {
    pControl->SetLimits(info.iWidth*3 / 4,
                        info.iHeight*3 / 4,
                        info.iWidth*5 / 4,
                        info.iHeight*5 / 4);
    pControl->SetPosition((float)info.Overscan.right - (int)pControl->GetWidth(),
                          (float)info.Overscan.bottom - (int)pControl->GetHeight());
    pControl->SetLocation(info.Overscan.right,
                          info.Overscan.bottom, false);
  }
  // Subtitles and OSD controls can only move up and down
  pControl = (CGUIMoverControl*)GetControl(CONTROL_SUBTITLES);
  if (pControl)
  {
    pControl->SetLimits(0, info.iHeight*3 / 4,
                        0, info.iHeight*5 / 4);
    pControl->SetPosition((info.iWidth - pControl->GetWidth()) * 0.5f,
                          info.iSubtitles - pControl->GetHeight());
    pControl->SetLocation(0, info.iSubtitles, false);
  }
  // lastly the pixel ratio control...
  CGUIResizeControl *pResize = (CGUIResizeControl*)GetControl(CONTROL_PIXEL_RATIO);
  if (pResize)
  {
    pResize->SetLimits(info.iWidth*0.25f, info.iHeight*0.5f,
                       info.iWidth*0.75f, info.iHeight*0.5f);
    pResize->SetHeight(info.iHeight * 0.5f);
    pResize->SetWidth(pResize->GetHeight() / info.fPixelRatio);
    pResize->SetPosition((info.iWidth - pResize->GetWidth()) / 2,
                         (info.iHeight - pResize->GetHeight()) / 2);
  }
  // Enable the default control
  EnableControl(m_iControl);
}

void CGUIWindowSettingsScreenCalibration::UpdateFromControl(int iControl)
{
  CStdString strStatus;
  RESOLUTION_INFO info = g_graphicsContext.GetResInfo(m_Res[m_iCurRes]);

  if (iControl == CONTROL_PIXEL_RATIO)
  {
    CGUIResizeControl *pControl = (CGUIResizeControl*)GetControl(CONTROL_PIXEL_RATIO);
    if (pControl)
    {
      float fWidth = (float)pControl->GetWidth();
      float fHeight = (float)pControl->GetHeight();
      info.fPixelRatio = fHeight / fWidth;
      // recenter our control...
      pControl->SetPosition((info.iWidth - pControl->GetWidth()) / 2,
                            (info.iHeight - pControl->GetHeight()) / 2);
      strStatus = StringUtils::Format("%s (%5.3f)", g_localizeStrings.Get(275).c_str(), info.fPixelRatio);
      SET_CONTROL_LABEL(CONTROL_LABEL_ROW2, 278);
    }
  }
  else
  {
    CGUIMoverControl *pControl = (CGUIMoverControl*)GetControl(iControl);
    if (pControl)
    {
      switch (iControl)
      {
      case CONTROL_TOP_LEFT:
        {
          info.Overscan.left = pControl->GetXLocation();
          info.Overscan.top = pControl->GetYLocation();
          strStatus = StringUtils::Format("%s (%i,%i)", g_localizeStrings.Get(272).c_str(), pControl->GetXLocation(), pControl->GetYLocation());
          SET_CONTROL_LABEL(CONTROL_LABEL_ROW2, 276);
        }
        break;

      case CONTROL_BOTTOM_RIGHT:
        {
          info.Overscan.right = pControl->GetXLocation();
          info.Overscan.bottom = pControl->GetYLocation();
          int iXOff1 = info.iWidth - pControl->GetXLocation();
          int iYOff1 = info.iHeight - pControl->GetYLocation();
          strStatus = StringUtils::Format("%s (%i,%i)", g_localizeStrings.Get(273).c_str(), iXOff1, iYOff1);
          SET_CONTROL_LABEL(CONTROL_LABEL_ROW2, 276);
        }
        break;

      case CONTROL_SUBTITLES:
        {
          info.iSubtitles = pControl->GetYLocation();
          strStatus = StringUtils::Format("%s (%i)", g_localizeStrings.Get(274).c_str(), pControl->GetYLocation());
          SET_CONTROL_LABEL(CONTROL_LABEL_ROW2, 277);
        }
        break;
      }
    }
  }

  g_graphicsContext.SetResInfo(m_Res[m_iCurRes], info);

  // set the label control correctly
  CStdString strText;
  if (g_Windowing.IsFullScreen())
    strText = StringUtils::Format("%ix%i@%.2f - %s | %s",
                                  info.iScreenWidth,
                                  info.iScreenHeight,
                                  info.fRefreshRate,
                                  g_localizeStrings.Get(244).c_str(),
                                  strStatus.c_str());
  else
    strText = StringUtils::Format("%ix%i - %s | %s",
                                  info.iScreenWidth,
                                  info.iScreenHeight,
                                  g_localizeStrings.Get(242).c_str(),
                                  strStatus.c_str());

  SET_CONTROL_LABEL(CONTROL_LABEL_ROW1, strText);
}

void CGUIWindowSettingsScreenCalibration::FrameMove()
{
  //  g_Windowing.Get3DDevice()->Clear(0, NULL, D3DCLEAR_TARGET, 0, 0, 0);
  m_iControl = GetFocusedControlID();
  if (m_iControl >= 0)
  {
    UpdateFromControl(m_iControl);
  }
  else
  {
    SET_CONTROL_LABEL(CONTROL_LABEL_ROW1, "");
    SET_CONTROL_LABEL(CONTROL_LABEL_ROW2, "");
  }
  CGUIWindow::FrameMove();
}

void CGUIWindowSettingsScreenCalibration::DoProcess(unsigned int currentTime, CDirtyRegionList &dirtyregions)
{
  MarkDirtyRegion();

  for (int i = CONTROL_TOP_LEFT; i <= CONTROL_PIXEL_RATIO; i++)
    SET_CONTROL_HIDDEN(i);

  m_needsScaling = true;
  CGUIWindow::DoProcess(currentTime, dirtyregions);
  m_needsScaling = false;

  g_graphicsContext.SetRenderingResolution(m_Res[m_iCurRes], false);
  g_graphicsContext.AddGUITransform();

  // process the movers etc.
  for (int i = CONTROL_TOP_LEFT; i <= CONTROL_PIXEL_RATIO; i++)
  {
    SET_CONTROL_VISIBLE(i);
    CGUIControl *control = (CGUIControl *)GetControl(i);
    if (control)
      control->DoProcess(currentTime, dirtyregions);
  }
  g_graphicsContext.RemoveTransform();
}

void CGUIWindowSettingsScreenCalibration::DoRender()
{
  // we set that we need scaling here to render so that anything else on screen scales correctly
  m_needsScaling = true;
  CGUIWindow::DoRender();
  m_needsScaling = false;
}
