/*
 *      Copyright (C) 2002 Frodo
 *      Portions Copyright (c) by the authors of ffmpeg and xvid
 *      Copyright (C) 2002-present Team Kodi
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
// SingleLock.h: interface for the CSingleLock class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "threads/CriticalSection.h"
#include "threads/Lockables.h"

/**
 * This implements a "guard" pattern for a CCriticalSection that
 *  borrows most of it's functionality from boost's unique_lock.
 */
class CSingleLock : public XbmcThreads::UniqueLock<CCriticalSection>
{
public:
  inline explicit CSingleLock(CCriticalSection& cs) : XbmcThreads::UniqueLock<CCriticalSection>(cs) {}
  inline explicit CSingleLock(const CCriticalSection& cs) : XbmcThreads::UniqueLock<CCriticalSection> ((CCriticalSection&)cs) {}

  inline void Leave() { unlock(); }
  inline void Enter() { lock(); }
protected:
  inline CSingleLock(CCriticalSection& cs, bool dicrim) : XbmcThreads::UniqueLock<CCriticalSection>(cs,true) {}
};


/**
 * This implements a "guard" pattern for exiting all locks
 *  currently being held by the current thread and restoring
 *  those locks on destruction.
 *
 * This class can be used on a CCriticalSection that isn't owned
 *  by this thread in which case it will do nothing.
 */
class CSingleExit
{
  CCriticalSection& sec;
  unsigned int count;
public:
  inline explicit CSingleExit(CCriticalSection& cs) : sec(cs), count(cs.exit()) { }
  inline ~CSingleExit() { sec.restore(count); }
};

