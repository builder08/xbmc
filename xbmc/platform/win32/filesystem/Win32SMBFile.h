#pragma once
/*
*      Copyright (C) 2014-present Team Kodi
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

#include "Win32File.h"

namespace XFILE
{
  class CWin32SMBFile : public CWin32File
  {
  public:
    CWin32SMBFile();
    virtual ~CWin32SMBFile();
    virtual bool Open(const CURL& url);
    virtual bool OpenForWrite(const CURL& url, bool bOverWrite = false);

    virtual bool Delete(const CURL& url);
    virtual bool Rename(const CURL& urlCurrentName, const CURL& urlNewName);
    virtual bool SetHidden(const CURL& url, bool hidden);
    virtual bool Exists(const CURL& url);
    virtual int Stat(const CURL& url, struct __stat64* statData);
  private:
    static bool ConnectAndAuthenticate(const CURL& url);
  };

}
