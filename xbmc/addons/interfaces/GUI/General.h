/*
 *  Copyright (c) 2020 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#pragma once

extern "C"
{

struct AddonGlobalInterface;

namespace ADDON
{

  /*!
   * @brief Global gui Add-on to Kodi callback functions
   *
   * To hold general gui functions and initialize also all other gui related types not
   * related to a instance type and usable for every add-on type.
   *
   * Related add-on header is "./xbmc/addons/kodi-addon-dev-kit/include/kodi/gui/General.h"
   */
  struct Interface_GUIGeneral
  {
    static void Init(AddonGlobalInterface* addonInterface);
    static void DeInit(AddonGlobalInterface* addonInterface);

    /*!
     * @brief callback functions from add-on to kodi
     *
     * @note For add of new functions use the "_" style to identify direct a
     * add-on callback function. Everything with CamelCase is only for the
     * usage in Kodi only.
     *
     * The parameter `kodiBase` is used to become the pointer for a `CAddonDll`
     * class.
     */
    //@{
    static void lock();
    static void unlock();

    static int get_screen_height(void* kodiBase);
    static int get_screen_width(void* kodiBase);
    static int get_video_resolution(void* kodiBase);
    static int get_current_window_dialog_id(void* kodiBase);
    static int get_current_window_id(void* kodiBase);
    static void* get_hw_context(void* kodiBase);
    //@}

  private:
    static int m_iAddonGUILockRef;
  };

} /* namespace ADDON */
} /* extern "C" */
