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

#include <string>
#include <vector>
#include "threads/CriticalSection.h"
#include "utils/GlobalsHandling.h"

//  forward
class LibraryLoader;

class CSectionLoader
{
public:
  class CDll
  {
  public:
    std::string m_strDllName;
    long m_lReferenceCount;
    LibraryLoader *m_pDll;
    unsigned int m_unloadDelayStartTick;
    bool m_bDelayUnload;
  };
  CSectionLoader(void);
  virtual ~CSectionLoader(void);

  static LibraryLoader* LoadDLL(const std::string& strSection, bool bDelayUnload=true, bool bLoadSymbols=false);
  static void UnloadDLL(const std::string& strSection);
  static void UnloadDelayed();
  void UnloadAll();

protected:
  std::vector<CDll> m_vecLoadedDLLs;
  CCriticalSection m_critSection;

};

extern  CSectionLoader g_sectionLoader;

