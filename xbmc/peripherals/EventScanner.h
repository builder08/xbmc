/*
 *      Copyright (C) 2016-present Team Kodi
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

#include <set>

#include "EventPollHandle.h"
#include "PeripheralTypes.h"
#include "threads/CriticalSection.h"
#include "threads/Event.h"
#include "threads/Thread.h"

namespace PERIPHERALS
{
  class IEventScannerCallback
  {
  public:
    virtual ~IEventScannerCallback(void) = default;

    virtual void ProcessEvents(void) = 0;
  };

  /*!
   * \brief Class to scan for peripheral events
   *
   * By default, a rate of 60 Hz is used. A client can obtain control over when
   * input is handled by registering for a polling handle.
   */
  class CEventScanner : public IEventPollCallback,
                        protected CThread
  {
  public:
    explicit CEventScanner(IEventScannerCallback* callback);

    ~CEventScanner(void) override = default;

    void Start(void);
    void Stop(void);

    EventPollHandlePtr RegisterPollHandle();

    // implementation of IEventPollCallback
    void Activate(CEventPollHandle *handle) override;
    void Deactivate(CEventPollHandle *handle) override;
    void HandleEvents(bool bWait) override;
    void Release(CEventPollHandle *handle) override;

  protected:
    // implementation of CThread
    void Process(void) override;

  private:
    double GetScanIntervalMs(void) const;

    IEventScannerCallback* const m_callback;
    std::set<void*>              m_activeHandles;
    CEvent                       m_scanEvent;
    CEvent                       m_scanFinishedEvent;
    CCriticalSection             m_mutex;
    CCriticalSection             m_pollMutex; // Prevent two poll handles from polling at once
  };
}
