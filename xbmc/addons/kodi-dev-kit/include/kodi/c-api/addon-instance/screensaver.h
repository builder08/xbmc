/*
 *  Copyright (C) 2005-2020 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#ifndef C_API_ADDONINSTANCE_SCREENSAVER_H
#define C_API_ADDONINSTANCE_SCREENSAVER_H

#include "../addon_base.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

  typedef void* KODI_ADDON_SCREENSAVER_HDL;

  /*!
   * @brief Screensaver properties
   *
   * Not to be used outside this header.
   */
  typedef struct AddonProps_Screensaver
  {
    ADDON_HARDWARE_CONTEXT device;
    int x;
    int y;
    int width;
    int height;
    float pixelRatio;
    const char* name;
    const char* presets;
    const char* profile;
  } AddonProps_Screensaver;

  /*!
   * @brief Screensaver callbacks
   *
   * Not to be used outside this header.
   */
  typedef struct AddonToKodiFuncTable_Screensaver
  {
    KODI_HANDLE kodiInstance;
  } AddonToKodiFuncTable_Screensaver;

  /*!
   * @brief Screensaver function hooks
   *
   * Not to be used outside this header.
   */
  typedef struct KodiToAddonFuncTable_Screensaver
  {
    bool(__cdecl* Start)(const KODI_ADDON_SCREENSAVER_HDL hdl);
    void(__cdecl* Stop)(const KODI_ADDON_SCREENSAVER_HDL hdl);
    void(__cdecl* Render)(const KODI_ADDON_SCREENSAVER_HDL hdl);
  } KodiToAddonFuncTable_Screensaver;

  /*!
   * @brief Screensaver instance
   *
   * Not to be used outside this header.
   */
  typedef struct AddonInstance_Screensaver
  {
    struct AddonProps_Screensaver* props;
    struct AddonToKodiFuncTable_Screensaver* toKodi;
    struct KodiToAddonFuncTable_Screensaver* toAddon;
  } AddonInstance_Screensaver;

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* !C_API_ADDONINSTANCE_SCREENSAVER_H */
