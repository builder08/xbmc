#pragma once
/*
*      Copyright (C) 2005-2014 Team XBMC
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

/**
* Playback settings
*/
enum BDPlaybackMode
{
  BD_PLAYBACK_SIMPLE_MENU = 0,
  BD_PLAYBACK_DISC_MENU,
  BD_PLAYBACK_MAIN_TITLE,
};

#include "system.h"
#ifdef HAVE_LIBBLURAY

#include "settings/lib/ISettingCallback.h"

class DllLibbluray;

class CDiscSettings : public ISettingCallback
{
public:
  /* ISettingCallback*/

  static CDiscSettings& GetInstance();
  void OnSettingChanged(std::shared_ptr<const CSetting> setting) override;
 
protected:
  CDiscSettings();
  virtual ~CDiscSettings();

  DllLibbluray*       m_dll;

};
#endif
