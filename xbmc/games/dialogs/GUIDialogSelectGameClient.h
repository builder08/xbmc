/*
 *      Copyright (C) 2016-2017 Team Kodi
 *      http://kodi.tv
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
 *  along with this Program; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */
#pragma once

#include "games/GameTypes.h"

namespace KODI
{
namespace GAME
{
  class CGUIDialogSelectGameClient
  {
  public:
    static bool ShowAndGetGameClient(const GameClientVector& candidates, const GameClientVector& installable, GameClientPtr& gameClient);

  private:
    static GameClientPtr InstallGameClient(const GameClientVector& installable);

    /*!
     * \brief Utility function to load the add-on manager for installed emulators
     */
    static void ActivateAddonMgr();

    /*!
     * \brief Utility function to load the add-on manager for all emulators
     */
    static void ActivateAddonBrowser();
  };
}
}
