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

#include <set>
#include <string>
#include <vector>

#include "storage/IStorageProvider.h"

class CAndroidStorageProvider : public IStorageProvider
{
public:
  CAndroidStorageProvider();
  virtual ~CAndroidStorageProvider() { }

  virtual void Initialize() { }
  virtual void Stop() { }
  virtual bool Eject(const std::string& mountpath) { return false; }

  virtual void GetLocalDrives(VECSOURCES &localDrives);
  virtual void GetRemovableDrives(VECSOURCES &removableDrives);
  virtual std::vector<std::string> GetDiskUsage();

  virtual bool PumpDriveChangeEvents(IStorageEventsCallback *callback);

private:
  std::string unescape(const std::string& str);
  VECSOURCES m_removableDrives;
  unsigned int m_removableLength;

  static std::set<std::string> GetRemovableDrives();
  static std::set<std::string> GetRemovableDrivesLinux();
};
