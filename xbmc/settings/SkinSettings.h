#pragma once
/*
 *      Copyright (C) 2013 Team XBMC
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

#include <map>
#include <string>

#ifndef SETTINGS_SETTINGS_LIB_ISUBSETTINGS_H_INCLUDED
#define SETTINGS_SETTINGS_LIB_ISUBSETTINGS_H_INCLUDED
#include "settings/lib/ISubSettings.h"
#endif

#ifndef SETTINGS_THREADS_CRITICALSECTION_H_INCLUDED
#define SETTINGS_THREADS_CRITICALSECTION_H_INCLUDED
#include "threads/CriticalSection.h"
#endif


class TiXmlNode;

class CSkinString
{
public:
  std::string name;
  std::string value;
};

class CSkinBool
{
public:
  CSkinBool()
    : value(false)
  { }

  std::string name;
  bool value;
};

class CSkinSettings : public ISubSettings
{
public:
  static CSkinSettings& Get();

  virtual bool Load(const TiXmlNode *settings);
  virtual bool Save(TiXmlNode *settings) const;
  virtual void Clear();

  int TranslateString(const std::string &setting);
  const std::string& GetString(int setting) const;
  void SetString(int setting, const std::string &label);

  int TranslateBool(const std::string &setting);
  bool GetBool(int setting) const;
  void SetBool(int setting, bool set);

  void Reset(const std::string &setting);
  void Reset();

protected:
  CSkinSettings();
  CSkinSettings(const CSkinSettings&);
  CSkinSettings& operator=(CSkinSettings const&);
  virtual ~CSkinSettings();

  std::string GetCurrentSkin() const;

private:
  std::map<int, CSkinString> m_strings;
  std::map<int, CSkinBool> m_bools;
  CCriticalSection m_critical;
};
