#pragma once
/*
 *      Copyright (C) 2013 Team XBMC
 *      http://www.xbmc.org
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

#include <map>
#include <string>

#include "settings/ISubSettings.h"
#include "settings/VideoSettings.h"
#include "threads/CriticalSection.h"

#define VOLUME_DRC_MINIMUM 0    // 0dB
#define VOLUME_DRC_MAXIMUM 6000 // 60dB

class TiXmlNode;

typedef enum {
  WatchedModeAll        = 0,
  WatchedModeUnwatched,
  WatchedModeWatched
} WatchedMode;

class CMediaSettings : public ISubSettings
{
public:
  static CMediaSettings& Get();

  virtual bool Load(const TiXmlNode *settings);
  virtual bool Save(TiXmlNode *settings) const;

  const CVideoSettings& GetDefaultVideoSettings() const { return m_defaultVideoSettings; }
  CVideoSettings& GetDefaultVideoSettings() { return m_defaultVideoSettings; }
  const CVideoSettings& GetCurrentVideoSettings() const { return m_currentVideoSettings; }
  CVideoSettings& GetCurrentVideoSettings() { return m_currentVideoSettings; }

  /*! \brief Retreive the watched mode for the given content type
   \param content Current content type
   \return the current watch mode for this content type, WATCH_MODE_ALL if the content type is unknown.
   \sa SetWatchMode
   */
  int GetWatchedMode(const std::string &content) const;

  /*! \brief Set the watched mode for the given content type
   \param content Current content type
   \param value Watched mode to set
   \sa GetWatchMode
   */
  void SetWatchedMode(const std::string &content, WatchedMode mode);

  /*! \brief Cycle the watched mode for the given content type
   \param content Current content type
   \sa GetWatchMode, SetWatchMode
   */
  void CycleWatchedMode(const std::string &content);

protected:
  CMediaSettings();
  CMediaSettings(const CMediaSettings&);
  CMediaSettings const& operator=(CMediaSettings const&);
  virtual ~CMediaSettings();

  static std::string GetWatchedContent(const std::string &content);

private:
  CVideoSettings m_defaultVideoSettings;
  CVideoSettings m_currentVideoSettings;

  typedef std::map<std::string, WatchedMode> WatchedModes;
  WatchedModes m_watchedModes;

  CCriticalSection m_critical;
};
