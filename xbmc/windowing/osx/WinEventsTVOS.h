/*
 *  Copyright (C) 2012-2018 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#pragma once

#include <list>
#include <queue>
#include <vector>
#include <string>

#include "threads/Event.h"
#include "threads/Thread.h"
#include "threads/CriticalSection.h"
#include "windowing/WinEvents.h"

class CWinEventsTVOS : public IWinEvents, public CThread
{
public:
    CWinEventsTVOS();
    ~CWinEventsTVOS();
    
    void MessagePush(XBMC_Event *newEvent);
    size_t GetQueueSize();
private:
  
    
    CCriticalSection             m_eventsCond;
    std::list<XBMC_Event>        m_events;
};

