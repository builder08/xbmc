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

#pragma once

#include <memory>
#include <vector>
#include "utils/GlobalsHandling.h"
#include "threads/CriticalSection.h"

class CryptThreadingInitializer
{
  std::vector<std::unique_ptr<CCriticalSection>> m_locks;
  CCriticalSection m_locksLock;

public:
  CryptThreadingInitializer();
  ~CryptThreadingInitializer();

  CCriticalSection* GetLock(int index);

private:
  CryptThreadingInitializer(const CryptThreadingInitializer &rhs) = delete;
  CryptThreadingInitializer& operator=(const CryptThreadingInitializer&) = delete;
};

XBMC_GLOBAL_REF(CryptThreadingInitializer,g_cryptThreadingInitializer);
#define g_cryptThreadingInitializer XBMC_GLOBAL_USE(CryptThreadingInitializer)
