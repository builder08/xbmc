/*
 *      Copyright (C) 2017-present Team Kodi
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
#pragma once

#include "DialogGameVideoSelect.h"
#include "cores/IPlayer.h"
#include "FileItem.h"

namespace KODI
{
namespace GAME
{
  class CDialogGameVideoFilter : public CDialogGameVideoSelect
  {
  public:
    CDialogGameVideoFilter();
    ~CDialogGameVideoFilter() override = default;

  protected:
    // implementation of CDialogGameVideoSelect
    std::string GetHeading() override;
    void PreInit() override;
    void GetItems(CFileItemList &items) override;
    void OnItemFocus(unsigned int index) override;
    unsigned int GetFocusedItem() const override;
    void PostExit() override;

  private:
    void InitScalingMethods();

    static void GetProperties(const CFileItem &item, ESCALINGMETHOD &scalingMethod, std::string &description);

    CFileItemList m_items;

    //! \brief Set to true when a description has first been set
    bool m_bHasDescription = false;
  };
}
}
