/*
 *      Copyright (C) 2005-2013 Team XBMC
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

#include "stdio_utf8.h"
#include "utils/CharsetConverter.h"

int remove_utf8(const char* __filename)
{
  std::wstring filenameW;
  if (!CCharsetConverter::Utf8ToWSystemSafe(__filename, filenameW))
    return -1;
  return ::DeleteFileW(filenameW.c_str()) ? 0 : -1;
}

int rename_utf8(const char* __old, const char* __new)
{
  std::wstring oldW, newW;
  if (CCharsetConverter::Utf8ToWSystemSafe(__old, oldW) &&
      CCharsetConverter::Utf8ToWSystemSafe(__new, newW))
  {
    return ::MoveFileW(oldW.c_str(), newW.c_str()) ? 0 : -1;
  }
  return -1;
}

FILE* fopen64_utf8(const char* __filename, const char* __modes)
{
  std::wstring filenameW, modesW;
  if (CCharsetConverter::Utf8ToWSystemSafe(__filename, filenameW) &&
      CCharsetConverter::Utf8ToWSystemSafe(__modes, modesW))
  {
    return _wfopen(filenameW.c_str(), modesW.c_str());
  }
  errno = EINVAL;
  return NULL;
}

