#pragma once
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

extern "C"
{

struct AddonGlobalInterface;

namespace ADDON
{

  /*!
   * @brief Global gui Add-on to Kodi callback functions
   *
   * To hold functions not related to a instance type and usable for
   * every add-on type.
   *
   * Related add-on header is "./xbmc/addons/kodi-addon-dev-kit/include/kodi/gui/dialogs/ExtendedProgress.h"
   */
  struct Interface_GUIDialogExtendedProgress
  {
    static void Init(AddonGlobalInterface* addonInterface);
    static void DeInit(AddonGlobalInterface* addonInterface);

    /*!
     * @brief callback functions from add-on to kodi
     *
     * @note To add a new function use the "_" style to directly identify an
     * add-on callback function. Everything with CamelCase is only to be used
     * in Kodi.
     *
     * The parameter `kodiBase` is used to become the pointer for a `CAddonDll`
     * class.
     */
    //@{
    static void* new_dialog(void* kodiBase, const char* title);
    static void delete_dialog(void* kodiBase, void* handle);
    static char* get_title(void* kodiBase, void* handle);
    static void set_title(void* kodiBase, void* handle, const char* title);
    static char* get_text(void* kodiBase, void* handle);
    static void set_text(void* kodiBase, void* handle, const char* text);
    static bool is_finished(void* kodiBase, void* handle);
    static void mark_finished(void* kodiBase, void* handle);
    static float get_percentage(void* kodiBase, void* handle);
    static void set_percentage(void* kodiBase, void* handle, float percentage);
    static void set_progress(void* kodiBase, void* handle, int currentItem, int itemCount);
    //@}
  };

} /* namespace ADDON */
} /* extern "C" */
