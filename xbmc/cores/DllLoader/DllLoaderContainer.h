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

#include "LibraryLoader.h"

class DllLoaderContainer
{
public:
  static void       Clear();
  static HMODULE    GetModuleAddress(const char* sName);
  static int        GetNrOfModules();
  static LibraryLoader* GetModule(int iPos);
  static LibraryLoader* GetModule(const char* sName);
  static LibraryLoader* GetModule(HMODULE hModule);
  static LibraryLoader* LoadModule(const char* sName, const char* sCurrentDir=NULL, bool bLoadSymbols=false);
  static void       ReleaseModule(LibraryLoader*& pDll);

  static void RegisterDll(LibraryLoader* pDll);
  static void UnRegisterDll(LibraryLoader* pDll);
  static void UnloadPythonDlls();

private:
  static LibraryLoader* FindModule(const char* sName, const char* sCurrentDir, bool bLoadSymbols);
  static LibraryLoader* LoadDll(const char* sName, bool bLoadSymbols);
  static bool       IsSystemDll(const char* sName);

  static LibraryLoader* m_dlls[64];
  static int m_iNrOfDlls;
  static bool m_bTrack;
};
