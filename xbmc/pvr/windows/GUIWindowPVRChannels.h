#pragma once

/*
 *      Copyright (C) 2012-2013 Team XBMC
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

#include "GUIWindowPVRCommon.h"

namespace PVR
{
  class CGUIWindowPVR;

  class CGUIWindowPVRChannels : public CGUIWindowPVRCommon
  {
    friend class CGUIWindowPVR;

  public:
    CGUIWindowPVRChannels(CGUIWindowPVR *parent);
    virtual ~CGUIWindowPVRChannels(void);

    void GetContextButtons(int itemNumber, CContextButtons &buttons) const;
    bool OnContextButton(int itemNumber, CONTEXT_BUTTON button);
    void UpdateData(bool bUpdateSelectedFile = true);
    void UpdateButtons(void);
    void Notify(const Observable &obs, const ObservableMessage msg);
    void ResetObservers(void);
    void UnregisterObservers(void);

  private:
    bool OnClickButton(CGUIMessage &message);
    bool OnClickList(CGUIMessage &message);

    bool OnContextButtonAdd(CFileItem *item, CONTEXT_BUTTON button);
    bool OnContextButtonGroupManager(CFileItem *item, CONTEXT_BUTTON button);
    bool OnContextButtonHide(CFileItem *item, CONTEXT_BUTTON button);
    bool OnContextButtonInfo(CFileItem *item, CONTEXT_BUTTON button);
    bool OnContextButtonMove(CFileItem *item, CONTEXT_BUTTON button);
    bool OnContextButtonPlay(CFileItem *item, CONTEXT_BUTTON button);
    bool OnContextButtonSetThumb(CFileItem *item, CONTEXT_BUTTON button);
    bool OnContextButtonShowHidden(CFileItem *item, CONTEXT_BUTTON button);
    bool OnContextButtonFilter(CFileItem *item, CONTEXT_BUTTON button);
    bool OnContextButtonUpdateEpg(CFileItem *item, CONTEXT_BUTTON button);
    bool OnContextButtonRecord(CFileItem *item, CONTEXT_BUTTON button);
    bool OnContextButtonLock(CFileItem *item, CONTEXT_BUTTON button);

    void ShowGroupManager(void);

    bool              m_bShowHiddenChannels;
  };
}
