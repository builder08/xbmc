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

#include <map>

#include "XBDateTime.h"
#include "guilib/GUIDialog.h"

#include "pvr/channels/PVRChannelNumber.h"

namespace PVR
{
  class CPVREpgSearchFilter;

  class CGUIDialogPVRGuideSearch : public CGUIDialog
  {
  public:
    CGUIDialogPVRGuideSearch(void);
    ~CGUIDialogPVRGuideSearch(void) override = default;
    bool OnMessage(CGUIMessage& message) override;
    void OnWindowLoaded() override;

    void SetFilterData(CPVREpgSearchFilter *searchFilter) { m_searchFilter = searchFilter; }
    bool IsConfirmed() const { return m_bConfirmed; }
    bool IsCanceled() const { return m_bCanceled; }

  protected:
    void OnInitWindow() override;

  private:
    void OnSearch();
    void UpdateChannelSpin(void);
    void UpdateGroupsSpin(void);
    void UpdateGenreSpin(void);
    void UpdateDurationSpin(void);
    CDateTime ReadDateTime(const std::string &strDate, const std::string &strTime) const;
    void Update();

    bool IsRadioSelected(int controlID);
    int  GetSpinValue(int controlID);
    std::string GetEditValue(int controlID);

    bool m_bConfirmed;
    bool m_bCanceled;
    CPVREpgSearchFilter *m_searchFilter;
    std::map<int, CPVRChannelNumber> m_channelNumbersMap;
  };
}
