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

#include "powermanagement/IPowerSyscall.h"
#include "powermanagement/PowerManager.h"
#include "threads/Event.h"
#include "threads/Thread.h"
#include <atomic>

class CWin32PowerStateWorker : public CThread
{
public:
  CWin32PowerStateWorker() : CThread("CWin32PowerStateWorker"), m_queryEvent(true), m_state(POWERSTATE_NONE) {}
  bool QueryStateChange(PowerState State);

protected:
  virtual void Process(void);
  virtual void OnStartup() { SetPriority(THREAD_PRIORITY_IDLE); };

private:
  static bool PowerManagement(PowerState State);

  std::atomic<PowerState> m_state;
  CEvent                  m_queryEvent;
};

class CWin32PowerSyscall : public CAbstractPowerSyscall
{
public:
  CWin32PowerSyscall();
  ~CWin32PowerSyscall();

  static IPowerSyscall* CreateInstance();
  static void Register();

  virtual bool Powerdown();
  virtual bool Suspend();
  virtual bool Hibernate();
  virtual bool Reboot();

  virtual bool CanPowerdown();
  virtual bool CanSuspend();
  virtual bool CanHibernate();
  virtual bool CanReboot();
  virtual int  BatteryLevel();

  virtual bool PumpPowerEvents(IPowerEventsCallback *callback);

  static void SetOnResume() { m_OnResume = true; }
  static void SetOnSuspend() { m_OnSuspend = true; }
  static bool IsSuspending() { return m_OnSuspend; }

private:
  CWin32PowerStateWorker m_worker;

  static bool m_OnResume;
  static bool m_OnSuspend;

};
