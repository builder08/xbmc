#pragma once

/*
 *      Copyright (C) 2005-2012 Team XBMC
 *      http://www.xbmc.org
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
 *  along with XBMC; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#include "StdString.h"
#include "threads/CriticalSection.h"
#include "interfaces/AnnouncementManager.h"

class Observable;
class ObservableMessageJob;

class Observer
{
  friend class Observable;

public:
  Observer(void) {};
  virtual ~Observer(void);

  /*!
   * @brief Remove this observer from all observables.
   */
  virtual void StopObserving(void);

  /*!
   * @brief Check whether this observer is observing an observable.
   * @param obs The observable to check.
   * @return True if this observer is observing the given observable, false otherwise.
   */
  virtual bool IsObserving(const Observable &obs) const;

  /*!
   * @brief Process a message from an observable.
   * @param obs The observable that sends the message.
   * @param msg The message.
   */
  virtual void Notify(const Observable &obs, const CStdString& msg) = 0;

protected:
  /*!
   * @brief Callback to register an observable.
   * @param obs The observable to register.
   */
  virtual void RegisterObservable(Observable *obs);

  /*!
   * @brief Callback to unregister an observable.
   * @param obs The observable to unregister.
   */
  virtual void UnregisterObservable(Observable *obs);

  std::vector<Observable *> m_observables;     /*!< all observables that are watched */
  CCriticalSection          m_obsCritSection;  /*!< mutex */
};

class Observable : public ANNOUNCEMENT::IAnnouncer
{
  friend class ObservableMessageJob;

public:
  Observable();
  virtual ~Observable();
  virtual Observable &operator=(const Observable &observable);

  /*!
   * @brief Remove this observable from all observers.
   */
  virtual void StopObserver(void);

  /*!
   * @brief Register an observer.
   * @param obs The observer to register.
   */
  virtual void RegisterObserver(Observer *obs);

  /*!
   * @brief Unregister an observer.
   * @param obs The observer to unregister.
   */
  virtual void UnregisterObserver(Observer *obs);

  /*!
   * @brief Send a message to all observers when m_bObservableChanged is true.
   * @param strMessage The message to send.
   * @param bAsync True to send the message async, using the jobmanager.
   */
  virtual void NotifyObservers(const CStdString& strMessage = "", bool bAsync = false);

  /*!
   * @brief Mark an observable changed.
   * @param bSetTo True to mark the observable changed, false to mark it as unchanged.
   */
  virtual void SetChanged(bool bSetTo = true);

  /*!
   * @brief Check whether this observable is being observed by an observer.
   * @param obs The observer to check.
   * @return True if this observable is being observed by the given observer, false otherwise.
   */
  virtual bool IsObserving(const Observer &obs) const;

  virtual void Announce(ANNOUNCEMENT::EAnnouncementFlag flag, const char *sender, const char *message, const CVariant &data);

protected:
  /*!
   * @brief Send a message to all observer when m_bObservableChanged is true.
   * @param obs The observer that sends the message.
   * @param observers The observers to send the message to.
   * @param strMessage The message to send.
   */
  static void SendMessage(Observable *obs, const std::vector<Observer *> *observers, const CStdString &strMessage);

  bool                    m_bObservableChanged; /*!< true when the observable is marked as changed, false otherwise */
  std::vector<Observer *> m_observers;          /*!< all observers */
  CCriticalSection        m_obsCritSection;     /*!< mutex */
  bool                    m_bAsyncAllowed;      /*!< true when async messages are allowed, false otherwise */
};
