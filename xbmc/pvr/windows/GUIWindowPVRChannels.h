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

#include "GUIWindowPVRBase.h"

namespace PVR
{
  class CGUIWindowPVRChannels : public CGUIWindowPVRBase
  {
  public:
    CGUIWindowPVRChannels(bool bRadio);
    virtual ~CGUIWindowPVRChannels(void) {};

    virtual bool OnMessage(CGUIMessage& message) override;
    virtual void GetContextButtons(int itemNumber, CContextButtons &buttons) override;
    virtual bool OnContextButton(int itemNumber, CONTEXT_BUTTON button) override;
    virtual bool Update(const std::string &strDirectory, bool updateFilterPath = true) override;
    virtual void UpdateButtons(void) override;
    virtual void ResetObservers(void) override;
    void UnregisterObservers(void);
    virtual bool OnAction(const CAction &action) override;

  protected:
    virtual std::string GetDirectoryPath(void) override;

  private:
    bool OnContextButtonAdd(CFileItem *item, CONTEXT_BUTTON button);
    bool OnContextButtonGroupManager(CFileItem *item, CONTEXT_BUTTON button);
    bool OnContextButtonInfo(CFileItem *item, CONTEXT_BUTTON button);
    bool OnContextButtonManage(CFileItem *item, CONTEXT_BUTTON button);
    bool OnContextButtonUpdateEpg(CFileItem *item, CONTEXT_BUTTON button);
    bool OnContextButtonStartRecord(CFileItem *item, CONTEXT_BUTTON button);
    bool OnContextButtonStopRecord(CFileItem *item, CONTEXT_BUTTON button);

    void ShowChannelManager();
    void ShowGroupManager(void);

    bool m_bShowHiddenChannels;
  };
}
