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

#pragma once

#ifndef UTILS_UTILS_GLOBALSHANDLING_H_INCLUDED
#define UTILS_UTILS_GLOBALSHANDLING_H_INCLUDED
#include "utils/GlobalsHandling.h"
#endif

#ifndef UTILS_THREADS_CRITICALSECTION_H_INCLUDED
#define UTILS_THREADS_CRITICALSECTION_H_INCLUDED
#include "threads/CriticalSection.h"
#endif


class CryptThreadingInitializer
{
  CCriticalSection** locks;
  int numlocks;
  CCriticalSection locksLock;

public:
  CryptThreadingInitializer();
  ~CryptThreadingInitializer();

  CCriticalSection* get_lock(int index);
};

XBMC_GLOBAL_REF(CryptThreadingInitializer,g_cryptThreadingInitializer);
#define g_cryptThreadingInitializer XBMC_GLOBAL_USE(CryptThreadingInitializer)
