/*
 *  Copyright (C) 2012-2018 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#include "PVRTimers.h"

#include <utility>

#include "FileItem.h"
#include "ServiceBroker.h"
#include "addons/PVRClient.h"
#include "guilib/LocalizeStrings.h"
#include "settings/Settings.h"
#include "threads/SingleLock.h"
#include "utils/log.h"

#include "pvr/PVRDatabase.h"
#include "pvr/PVRJobs.h"
#include "pvr/PVRManager.h"
#include "pvr/addons/PVRClients.h"
#include "pvr/channels/PVRChannel.h"
#include "pvr/epg/EpgContainer.h"
#include "pvr/timers/PVRTimerRuleMatcher.h"
#include "pvr/timers/PVRTimersPath.h"

using namespace PVR;

bool CPVRTimersContainer::UpdateFromClient(const CPVRTimerInfoTagPtr &timer)
{
  CSingleLock lock(m_critSection);
  CPVRTimerInfoTagPtr tag = GetByClient(timer->m_iClientId, timer->m_iClientIndex);
  if (!tag)
  {
    tag.reset(new CPVRTimerInfoTag());
    tag->m_iTimerId = ++m_iLastId;
    InsertEntry(tag);
  }

  return tag->UpdateEntry(timer);
}

std::shared_ptr<CPVRTimerInfoTag> CPVRTimersContainer::GetByClient(int iClientId, int iClientIndex) const
{
  CSingleLock lock(m_critSection);
  for (const auto startDates : m_tags)
  {
    for (const auto timer : startDates.second)
    {
      if (timer->m_iClientId == iClientId && timer->m_iClientIndex == iClientIndex)
      {
        return timer;
      }
    }
  }

  return CPVRTimerInfoTagPtr();
}

void CPVRTimersContainer::InsertEntry(const std::shared_ptr<CPVRTimerInfoTag>& newTimer)
{
  auto it = m_tags.find(newTimer->m_bStartAnyTime ? CDateTime() : newTimer->StartAsUTC());
  if (it == m_tags.end())
  {
    VecTimerInfoTag addEntry({newTimer});
    m_tags.insert(std::make_pair(newTimer->m_bStartAnyTime ? CDateTime() : newTimer->StartAsUTC(), addEntry));
  }
  else
  {
    it->second.emplace_back(newTimer);
  }
}

CPVRTimers::CPVRTimers(void)
: CThread("PVRTimers"),
  m_settings({
    CSettings::SETTING_PVRPOWERMANAGEMENT_DAILYWAKEUP,
    CSettings::SETTING_PVRPOWERMANAGEMENT_PREWAKEUP,
    CSettings::SETTING_PVRPOWERMANAGEMENT_BACKENDIDLETIME,
    CSettings::SETTING_PVRPOWERMANAGEMENT_DAILYWAKEUPTIME,
    CSettings::SETTING_PVRRECORD_TIMERNOTIFICATIONS,
    CSettings::SETTING_PVRTIMERS_HIDEDISABLEDTIMERS
  })
{
}

CPVRTimers::~CPVRTimers(void)
{
}

bool CPVRTimers::Load(void)
{
  // unload previous timers
  Unload();

  // load local timers from database
  bool bReturn = LoadFromDatabase();

  Update(); // update from clients

  CServiceBroker::GetPVRManager().EpgContainer().RegisterObserver(this);
  Create();

  return bReturn;
}

void CPVRTimers::Unload()
{
  StopThread();
  CServiceBroker::GetPVRManager().EpgContainer().UnregisterObserver(this);

  // remove all tags
  CSingleLock lock(m_critSection);
  m_tags.clear();
}

bool CPVRTimers::Update(void)
{
  {
    CSingleLock lock(m_critSection);
    if (m_bIsUpdating)
      return false;
    m_bIsUpdating = true;
  }

  CLog::LogFC(LOGDEBUG, LOGPVR, "Updating timers");
  CPVRTimersContainer newTimerList;
  std::vector<int> failedClients;
  CServiceBroker::GetPVRManager().Clients()->GetTimers(&newTimerList, failedClients);
  return UpdateEntries(newTimerList, failedClients);
}

bool CPVRTimers::LoadFromDatabase()
{
  const std::shared_ptr<CPVRDatabase> database = CServiceBroker::GetPVRManager().GetTVDatabase();
  if (database)
  {
    bool bChanged = false;

    const std::vector<std::shared_ptr<CPVRTimerInfoTag>> timers = database->GetTimers(*this);
    for (const auto& timer : timers)
    {
      bChanged |= !!UpdateEntry(timer);
    }

    if (bChanged)
      NotifyTimersEvent();
  }
  return true;
}

void CPVRTimers::Process()
{
  static const unsigned int MAX_NOTIFICATION_DELAY = 10; // secs

  while (!m_bStop)
  {
    // update all timers not owned by a client (e.g. reminders)
    UpdateEntries(MAX_NOTIFICATION_DELAY);

    Sleep(MAX_NOTIFICATION_DELAY * 1000);
  }
}

bool CPVRTimers::IsRecording(void) const
{
  CSingleLock lock(m_critSection);

  for (MapTags::const_iterator it = m_tags.begin(); it != m_tags.end(); ++it)
    for (VecTimerInfoTag::const_iterator timerIt = it->second.begin(); timerIt != it->second.end(); ++timerIt)
      if ((*timerIt)->IsRecording())
        return true;

  return false;
}

void CPVRTimers::RemoveEntry(const std::shared_ptr<CPVRTimerInfoTag>& tag)
{
  CSingleLock lock(m_critSection);

  auto it = m_tags.find(tag->m_bStartAnyTime ? CDateTime() : tag->StartAsUTC());
  if (it != m_tags.end())
  {
    it->second.erase(std::remove_if(it->second.begin(),
                                    it->second.end(),
                                    [&tag](const std::shared_ptr<CPVRTimerInfoTag>& timer)
                                    {
                                      return tag->m_iClientId == timer->m_iClientId &&
                                      tag->m_iClientIndex == timer->m_iClientIndex;
                                    }),
                     it->second.end());

    if (it->second.empty())
      m_tags.erase(it);
  }
}

bool CPVRTimers::UpdateEntries(const CPVRTimersContainer &timers, const std::vector<int> &failedClients)
{
  bool bChanged(false);
  bool bAddedOrDeleted(false);
  std::vector< std::pair< int, std::string> > timerNotifications;

  CSingleLock lock(m_critSection);

  /* go through the timer list and check for updated or new timers */
  for (MapTags::const_iterator it = timers.GetTags().begin(); it != timers.GetTags().end(); ++it)
  {
    for (VecTimerInfoTag::const_iterator timerIt = it->second.begin(); timerIt != it->second.end(); ++timerIt)
    {
      /* check if this timer is present in this container */
      CPVRTimerInfoTagPtr existingTimer = GetByClient((*timerIt)->m_iClientId, (*timerIt)->m_iClientIndex);
      if (existingTimer)
      {
        /* if it's present, update the current tag */
        bool bStateChanged(existingTimer->m_state != (*timerIt)->m_state);
        if (existingTimer->UpdateEntry(*timerIt))
        {
          bChanged = true;
          existingTimer->ResetChildState();

          if (bStateChanged)
          {
            std::string strMessage;
            existingTimer->GetNotificationText(strMessage);
            timerNotifications.emplace_back(std::make_pair((*timerIt)->m_iClientId, strMessage));
          }

          CLog::LogFC(LOGDEBUG, LOGPVR, "Updated timer %d on client %d",
                      (*timerIt)->m_iClientIndex, (*timerIt)->m_iClientId);
        }
      }
      else
      {
        /* new timer */
        CPVRTimerInfoTagPtr newTimer = CPVRTimerInfoTagPtr(new CPVRTimerInfoTag);
        newTimer->UpdateEntry(*timerIt);
        newTimer->m_iTimerId = ++m_iLastId;
        InsertEntry(newTimer);

        bChanged = true;
        bAddedOrDeleted = true;

        std::string strMessage;
        newTimer->GetNotificationText(strMessage);
        timerNotifications.push_back(std::make_pair(newTimer->m_iClientId, strMessage));

        CLog::LogFC(LOGDEBUG, LOGPVR, "Added timer %d on client %d",
                    (*timerIt)->m_iClientIndex, (*timerIt)->m_iClientId);
      }
    }
  }

  /* to collect timer with changed starting time */
  VecTimerInfoTag timersToMove;

  /* check for deleted timers */
  for (MapTags::iterator it = m_tags.begin(); it != m_tags.end();)
  {
    for (std::vector<CPVRTimerInfoTagPtr>::iterator it2 = it->second.begin(); it2 != it->second.end();)
    {
      const std::shared_ptr<CPVRTimerInfoTag> timer = *it2;
      if (!timers.GetByClient(timer->m_iClientId, timer->m_iClientIndex))
      {
        /* timer was not found */
        bool bIgnoreTimer = !timer->IsOwnedByClient();
        if (!bIgnoreTimer)
        {
          for (const auto& failedClient : failedClients)
          {
            if (failedClient == timer->m_iClientId)
            {
              bIgnoreTimer = true;
              break;
            }
          }
        }

        if (bIgnoreTimer)
        {
          ++it2;
          continue;
        }

        CLog::LogFC(LOGDEBUG, LOGPVR, "Deleted timer %d on client %d",
                    timer->m_iClientIndex, timer->m_iClientId);

        timerNotifications.push_back(std::make_pair(timer->m_iClientId, timer->GetDeletedNotificationText()));

        it2 = it->second.erase(it2);

        bChanged = true;
        bAddedOrDeleted = true;
      }
      else if ((timer->m_bStartAnyTime && it->first != CDateTime()) ||
               (!timer->m_bStartAnyTime && timer->StartAsUTC() != it->first))
      {
        /* timer start has changed */
        CLog::LogFC(LOGDEBUG, LOGPVR, "Changed start time timer %d on client %d",
                    timer->m_iClientIndex, timer->m_iClientId);

        /* remember timer */
        timersToMove.push_back(timer);

        /* remove timer for now, reinsert later */
        it2 = it->second.erase(it2);

        bChanged = true;
        bAddedOrDeleted = true;
      }
      else
      {
        ++it2;
      }
    }
    if (it->second.empty())
      it = m_tags.erase(it);
    else
      ++it;
  }

  /* reinsert timers with changed timer start */
  for (VecTimerInfoTag::const_iterator timerIt = timersToMove.begin(); timerIt != timersToMove.end(); ++timerIt)
  {
    InsertEntry(*timerIt);
  }

  /* update child information for all parent timers */
  for (const auto &tagsEntry : m_tags)
  {
    for (const auto &timersEntry : tagsEntry.second)
    {
      if (timersEntry->IsTimerRule())
        timersEntry->ResetChildState();
    }

    for (const auto& timersEntry : tagsEntry.second)
    {
      const std::shared_ptr<CPVRTimerInfoTag> parentTimer = GetTimerRule(timersEntry);
      if (parentTimer)
        parentTimer->UpdateChildState(timersEntry, true);
    }
  }

  m_bIsUpdating = false;
  if (bChanged)
  {
    UpdateChannels();
    lock.Leave();

    NotifyTimersEvent(bAddedOrDeleted);

    if (!timerNotifications.empty() && CServiceBroker::GetPVRManager().IsStarted())
    {
      CPVREventlogJob *job = new CPVREventlogJob;

      /* queue notifications / fill eventlog */
      for (const auto &entry : timerNotifications)
      {
        const CPVRClientPtr client = CServiceBroker::GetPVRManager().GetClient(entry.first);
        if (client)
        {
          job->AddEvent(m_settings.GetBoolValue(CSettings::SETTING_PVRRECORD_TIMERNOTIFICATIONS),
                        false, // info, no error
                        client->Name(),
                        entry.second,
                        client->Icon());
        }
      }

      CJobManager::GetInstance().AddJob(job, nullptr);
    }
  }

  return bChanged;
}

namespace
{
  std::vector<std::shared_ptr<CPVREpgInfoTag>> GetEpgTagsForTimerRule(const CPVRTimerRuleMatcher& matcher)
  {
    std::vector<std::shared_ptr<CPVREpgInfoTag>> matches;

    const std::shared_ptr<CPVRChannel> channel = matcher.GetChannel();
    if (channel)
    {
      // match single channel
      const std::shared_ptr<CPVREpg> epg = channel->GetEPG();
      if (epg)
      {
        std::vector<std::shared_ptr<CPVREpgInfoTag>> tags = epg->GetTags();
        for (const auto& tag : tags)
        {
          if (matcher.Matches(tag))
            matches.emplace_back(tag);
        }
      }
    }
    else
    {
      // match any channel
      const std::vector<std::shared_ptr<CPVREpg>> epgs = CServiceBroker::GetPVRManager().EpgContainer().GetAllEpgs();
      for (const auto& epg : epgs)
      {
        std::vector<std::shared_ptr<CPVREpgInfoTag>> tags = epg->GetTags();
        for (const auto& tag : tags)
        {
          if (matcher.Matches(tag))
            matches.emplace_back(tag);
        }
      }
    }

    return matches;
  }

  void AddTimerRuleToEpgMap(const std::shared_ptr<CPVRTimerInfoTag>& timer,
                            const CDateTime& now,
                            std::map<std::shared_ptr<CPVREpg>, std::vector<std::shared_ptr<CPVRTimerRuleMatcher>>>& epgMap,
                            bool& bFetchedAllEpgs)
  {
    const std::shared_ptr<CPVRChannel> channel = timer->Channel();
    if (channel)
    {
      const std::shared_ptr<CPVREpg> epg = channel->GetEPG();
      if (epg)
      {
        const std::shared_ptr<CPVRTimerRuleMatcher> matcher = std::make_shared<CPVRTimerRuleMatcher>(timer, now);
        auto it = epgMap.find(epg);
        if (it == epgMap.end())
          epgMap.insert({epg, {matcher}});
        else
          it->second.emplace_back(matcher);
      }
    }
    else
    {
      // rule matches "any channel" => we need to check all channels
      if (!bFetchedAllEpgs)
      {
        const std::vector<std::shared_ptr<CPVREpg>> epgs = CServiceBroker::GetPVRManager().EpgContainer().GetAllEpgs();
        for (const auto& epg : epgs)
        {
          const std::shared_ptr<CPVRTimerRuleMatcher> matcher = std::make_shared<CPVRTimerRuleMatcher>(timer, now);
          auto it = epgMap.find(epg);
          if (it == epgMap.end())
            epgMap.insert({epg, {matcher}});
          else
            it->second.emplace_back(matcher);
        }
        bFetchedAllEpgs = true;
      }
      else
      {
        for (auto& epgMapEntry : epgMap)
        {
          const std::shared_ptr<CPVRTimerRuleMatcher> matcher = std::make_shared<CPVRTimerRuleMatcher>(timer, now);
          epgMapEntry.second.emplace_back(matcher);
        }
      }
    }
  }
} // unnamed namespace

bool CPVRTimers::UpdateEntries(int iMaxNotificationDelay)
{
  std::vector<std::shared_ptr<CPVRTimerInfoTag>> timersToReinsert;
  std::vector<std::pair<std::shared_ptr<CPVRTimerInfoTag>, std::shared_ptr<CPVRTimerInfoTag>>> childTimersToInsert;
  bool bChanged = false;
  const CDateTime now = CDateTime::GetUTCDateTime();
  bool bFetchedAllEpgs = false;
  std::map<std::shared_ptr<CPVREpg>, std::vector<std::shared_ptr<CPVRTimerRuleMatcher>>> epgMap;

  CSingleLock lock(m_critSection);

  for (MapTags::iterator it = m_tags.begin(); it != m_tags.end();)
  {
    for (VecTimerInfoTag::iterator it2 = it->second.begin(); it2 != it->second.end();)
    {
      std::shared_ptr<CPVRTimerInfoTag> timer = *it2;
      bool bDeleteTimer = false;
      if (!timer->IsOwnedByClient())
      {
        if (timer->IsEpgBased())
        {
          // update data from current epg tag
          const std::shared_ptr<CPVREpgInfoTag> epgTag = timer->GetEpgInfoTag();
          if (epgTag)
          {
            bool bStartChanged = !timer->m_bStartAnyTime && epgTag->StartAsUTC() != timer->StartAsUTC();
            bool bEndChanged = !timer->m_bEndAnyTime && epgTag->EndAsUTC() != timer->EndAsUTC();
            if (bStartChanged || bEndChanged)
            {
              if (bStartChanged)
                timer->SetStartFromUTC(epgTag->StartAsUTC());
              if (bEndChanged)
                timer->SetEndFromUTC(epgTag->EndAsUTC());

              timer->UpdateSummary();
              bChanged = true;

              if (bStartChanged)
              {
                // start time changed. timer must be reinserted in timer map
                bDeleteTimer = true;
                timersToReinsert.emplace_back(timer); // remember and reinsert/save later
              }
              else
              {
                // save changes to database
                timer->Persist();
              }
            }
          }
        }

        // check for due timers and announce/delete them
        int iMarginStart = timer->GetTimerType()->SupportsStartEndMargin() ? timer->MarginStart() : 0;
        if (!timer->IsTimerRule() && (timer->StartAsUTC() - CDateTimeSpan(0, 0, iMarginStart, iMaxNotificationDelay)) < now)
        {
          if (timer->IsReminder() && timer->m_state != PVR_TIMER_STATE_DISABLED)
          {
            // reminder is due / over due. announce it.
            m_remindersToAnnounce.push(timer);
            CServiceBroker::GetPVRManager().PublishEvent(PVREvent::AnnounceReminder);
          }

          if (timer->EndAsUTC() >= now)
          {
            // disable timer until timer's end time is due
            if (timer->m_state != PVR_TIMER_STATE_DISABLED)
            {
              timer->m_state = PVR_TIMER_STATE_DISABLED;
              bChanged = true;
            }
          }
          else
          {
            // end time due. delete completed timer
            bChanged = true;
            bDeleteTimer = true;
            timer->DeleteFromDatabase();
          }
        }

        if (timer->IsTimerRule() && timer->IsReminder() && timer->IsActive())
        {
          if (timer->IsEpgBased())
          {
            if (m_bReminderRulesUpdatePending)
              AddTimerRuleToEpgMap(timer, now, epgMap, bFetchedAllEpgs);
          }
          else
          {
            // create new children of time-based reminder timer rules
            const CPVRTimerRuleMatcher matcher(timer, now);
            const CDateTime nextStart = matcher.GetNextTimerStart();
            if (nextStart.IsValid())
            {
              bool bCreate = false;
              const auto it = m_tags.find(nextStart);
              if (it == m_tags.end())
                bCreate = true;
              else
                bCreate = std::find_if(it->second.cbegin(), it->second.cend(),
                                       [&timer](const std::shared_ptr<CPVRTimerInfoTag>& tmr)
                                       {
                                         return tmr->m_iParentClientIndex == timer->m_iClientIndex;
                                       }) == it->second.cend();
              if (bCreate)
              {
                const CDateTimeSpan duration = timer->EndAsUTC() - timer->StartAsUTC();
                const std::shared_ptr<CPVRTimerInfoTag> childTimer
                  = CPVRTimerInfoTag::CreateReminderFromDate(nextStart, duration.GetSecondsTotal() / 60, timer);
                if (childTimer)
                {
                  bChanged = true;
                  childTimersToInsert.emplace_back(std::make_pair(timer, childTimer)); // remember and insert/save later
                }
              }
            }
          }
        }
      }

      if (bDeleteTimer)
      {
        const std::shared_ptr<CPVRTimerInfoTag> parent = GetTimerRule(timer);
        if (parent)
          parent->UpdateChildState(timer, false);

        it2 = it->second.erase(it2);
      }
      else
      {
        ++it2;
      }
    }

    if (it->second.empty())
      it = m_tags.erase(it);
    else
      ++it;
  }

  // create new children of local epg-based reminder timer rules
  for (const auto& epgMapEntry : epgMap)
  {
    const auto epgTags = epgMapEntry.first->GetTags();
    for (const auto& epgTag : epgTags)
    {
      if (GetTimerForEpgTag(epgTag))
        continue;

      for (const auto& matcher : epgMapEntry.second)
      {
        if (!matcher->Matches(epgTag))
          continue;

        const std::shared_ptr<CPVRTimerInfoTag> childTimer = CPVRTimerInfoTag::CreateReminderFromEpg(epgTag, matcher->GetTimerRule());
        if (childTimer)
        {
          bChanged = true;
          childTimersToInsert.emplace_back(std::make_pair(matcher->GetTimerRule(), childTimer)); // remember and insert/save later
        }
      }
    }
  }

  // reinsert timers with changed timer start
  for (const auto& timer : timersToReinsert)
  {
    InsertEntry(timer);
    timer->Persist();
  }

  // insert new children of time-based local timer rules
  for (const auto& timerPair : childTimersToInsert)
  {
    PersistAndUpdateLocalTimer(timerPair.second, timerPair.first);
  }

  m_bReminderRulesUpdatePending = false;

  // announce changes
  if (bChanged)
  {
    lock.Leave();
    NotifyTimersEvent();
  }

  return bChanged;
}

std::shared_ptr<CPVRTimerInfoTag> CPVRTimers::GetNextReminderToAnnnounce()
{
  std::shared_ptr<CPVRTimerInfoTag> ret;
  CSingleLock lock(m_critSection);
  if (!m_remindersToAnnounce.empty())
  {
    ret = m_remindersToAnnounce.front();
    m_remindersToAnnounce.pop();
  }
  return ret;
}

bool CPVRTimers::KindMatchesTag(const TimerKind &eKind, const CPVRTimerInfoTagPtr &tag) const
{
  return (eKind == TimerKindAny) ||
         (eKind == TimerKindTV && !tag->m_bIsRadio) ||
         (eKind == TimerKindRadio && tag->m_bIsRadio);
}

std::shared_ptr<CPVRTimerInfoTag> CPVRTimers::GetNextActiveTimer(const TimerKind &eKind, bool bIgnoreReminders) const
{
  CSingleLock lock(m_critSection);

  for (const auto &tagsEntry : m_tags)
  {
    for (const auto &timersEntry : tagsEntry.second)
    {
      if (bIgnoreReminders && timersEntry->IsReminder())
        continue;

      if (KindMatchesTag(eKind, timersEntry) &&
          timersEntry->IsActive() &&
          !timersEntry->IsRecording() &&
          !timersEntry->IsTimerRule() &&
          !timersEntry->IsBroken())
        return timersEntry;
    }
  }

  return std::shared_ptr<CPVRTimerInfoTag>();
}

std::shared_ptr<CPVRTimerInfoTag> CPVRTimers::GetNextActiveTimer(bool bIgnoreReminders /* = true */) const
{
  return GetNextActiveTimer(TimerKindAny, bIgnoreReminders);
}

std::shared_ptr<CPVRTimerInfoTag> CPVRTimers::GetNextActiveTVTimer(void) const
{
  return GetNextActiveTimer(TimerKindTV, true);
}

std::shared_ptr<CPVRTimerInfoTag> CPVRTimers::GetNextActiveRadioTimer(void) const
{
  return GetNextActiveTimer(TimerKindRadio, true);
}

std::vector<std::shared_ptr<CPVRTimerInfoTag>> CPVRTimers::GetActiveTimers(void) const
{
  std::vector<std::shared_ptr<CPVRTimerInfoTag>> tags;
  CSingleLock lock(m_critSection);

  for (MapTags::const_iterator it = m_tags.begin(); it != m_tags.end(); ++it)
  {
    for (VecTimerInfoTag::const_iterator timerIt = it->second.begin(); timerIt != it->second.end(); ++timerIt)
    {
      CPVRTimerInfoTagPtr current = *timerIt;
      if (current->IsActive() &&
          !current->IsBroken() &&
          !current->IsReminder() &&
          !current->IsTimerRule())
      {
        tags.emplace_back(current);
      }
    }
  }

  return tags;
}

int CPVRTimers::AmountActiveTimers(const TimerKind &eKind) const
{
  int iReturn = 0;
  CSingleLock lock(m_critSection);

  for (const auto &tagsEntry : m_tags)
  {
    for (const auto &timersEntry : tagsEntry.second)
    {
      if (KindMatchesTag(eKind, timersEntry) &&
          timersEntry->IsActive() &&
          !timersEntry->IsBroken() &&
          !timersEntry->IsReminder() &&
          !timersEntry->IsTimerRule())
        ++iReturn;
    }
  }

  return iReturn;
}

int CPVRTimers::AmountActiveTimers(void) const
{
  return AmountActiveTimers(TimerKindAny);
}

int CPVRTimers::AmountActiveTVTimers(void) const
{
  return AmountActiveTimers(TimerKindTV);
}

int CPVRTimers::AmountActiveRadioTimers(void) const
{
  return AmountActiveTimers(TimerKindRadio);
}

std::vector<std::shared_ptr<CPVRTimerInfoTag>> CPVRTimers::GetActiveRecordings(const TimerKind& eKind) const
{
  std::vector<std::shared_ptr<CPVRTimerInfoTag>> tags;
  CSingleLock lock(m_critSection);

  for (const auto &tagsEntry : m_tags)
  {
    for (const auto &timersEntry : tagsEntry.second)
    {
      if (KindMatchesTag(eKind, timersEntry) &&
          timersEntry->IsRecording() &&
          !timersEntry->IsTimerRule() &&
          !timersEntry->IsBroken() &&
          !timersEntry->IsReminder())
      {
        tags.emplace_back(timersEntry);
      }
    }
  }

  return tags;
}

std::vector<std::shared_ptr<CPVRTimerInfoTag>> CPVRTimers::GetActiveRecordings() const
{
  return GetActiveRecordings(TimerKindAny);
}

std::vector<std::shared_ptr<CPVRTimerInfoTag>> CPVRTimers::GetActiveTVRecordings() const
{
  return GetActiveRecordings(TimerKindTV);
}

std::vector<std::shared_ptr<CPVRTimerInfoTag>> CPVRTimers::GetActiveRadioRecordings() const
{
  return GetActiveRecordings(TimerKindRadio);
}

int CPVRTimers::AmountActiveRecordings(const TimerKind &eKind) const
{
  int iReturn = 0;
  CSingleLock lock(m_critSection);

  for (const auto &tagsEntry : m_tags)
  {
    for (const auto &timersEntry : tagsEntry.second)
    {
      if (KindMatchesTag(eKind, timersEntry) &&
          timersEntry->IsRecording() &&
          !timersEntry->IsTimerRule() &&
          !timersEntry->IsBroken() &&
          !timersEntry->IsReminder())
        ++iReturn;
    }
  }

  return iReturn;
}

int CPVRTimers::AmountActiveRecordings(void) const
{
  return AmountActiveRecordings(TimerKindAny);
}

int CPVRTimers::AmountActiveTVRecordings(void) const
{
  return AmountActiveRecordings(TimerKindTV);
}

int CPVRTimers::AmountActiveRadioRecordings(void) const
{
  return AmountActiveRecordings(TimerKindRadio);
}

bool CPVRTimers::GetRootDirectory(const CPVRTimersPath &path, CFileItemList &items) const
{
  CFileItemPtr item(new CFileItem(CPVRTimersPath::PATH_ADDTIMER, false));
  item->SetLabel(g_localizeStrings.Get(19026)); // "Add timer..."
  item->SetLabelPreformatted(true);
  item->SetSpecialSort(SortSpecialOnTop);
  item->SetIconImage("DefaultTVShows.png");
  items.Add(item);

  bool bRadio = path.IsRadio();
  bool bRules = path.IsRules();

  bool bHideDisabled = m_settings.GetBoolValue(CSettings::SETTING_PVRTIMERS_HIDEDISABLEDTIMERS);

  CSingleLock lock(m_critSection);
  for (const auto &tagsEntry : m_tags)
  {
    for (const auto &timer : tagsEntry.second)
    {
      if ((bRadio == timer->m_bIsRadio || (bRules && timer->m_iClientChannelUid == PVR_TIMER_ANY_CHANNEL)) &&
          (bRules == timer->IsTimerRule()) &&
          (!bHideDisabled || (timer->m_state != PVR_TIMER_STATE_DISABLED)))
      {
        item.reset(new CFileItem(timer));
        std::string strItemPath(
          CPVRTimersPath(path.GetPath(), timer->m_iClientId, timer->m_iClientIndex).GetPath());
        item->SetPath(strItemPath);
        items.Add(item);
      }
    }
  }
  return true;
}

bool CPVRTimers::GetSubDirectory(const CPVRTimersPath &path, CFileItemList &items) const
{
  bool bRadio = path.IsRadio();
  int iParentId = path.GetParentId();
  int iClientId = path.GetClientId();

  bool bHideDisabled = m_settings.GetBoolValue(CSettings::SETTING_PVRTIMERS_HIDEDISABLEDTIMERS);

  CFileItemPtr item;

  CSingleLock lock(m_critSection);
  for (const auto &tagsEntry : m_tags)
  {
    for (const auto &timer : tagsEntry.second)
    {
      if ((timer->m_bIsRadio == bRadio) &&
          (timer->m_iParentClientIndex != PVR_TIMER_NO_PARENT) &&
          (timer->m_iClientId == iClientId) &&
          (timer->m_iParentClientIndex == iParentId) &&
          (!bHideDisabled || (timer->m_state != PVR_TIMER_STATE_DISABLED)))
      {
        item.reset(new CFileItem(timer));
        std::string strItemPath(
          CPVRTimersPath(path.GetPath(), timer->m_iClientId, timer->m_iClientIndex).GetPath());
        item->SetPath(strItemPath);
        items.Add(item);
      }
    }
  }
  return true;
}

bool CPVRTimers::GetDirectory(const std::string& strPath, CFileItemList &items) const
{
  CPVRTimersPath path(strPath);
  if (path.IsValid())
  {
    if (path.IsTimersRoot())
    {
      /* Root folder containing either timer rules or timers. */
      return GetRootDirectory(path, items);
    }
    else if (path.IsTimerRule())
    {
      /* Sub folder containing the timers scheduled by the given timer rule. */
      return GetSubDirectory(path, items);
    }
  }

  CLog::LogF(LOGERROR,"Invalid URL %s", strPath.c_str());
  return false;
}

/********** channel methods **********/

bool CPVRTimers::DeleteTimersOnChannel(const CPVRChannelPtr &channel, bool bDeleteTimerRules /* = true */, bool bCurrentlyActiveOnly /* = false */)
{
  bool bReturn = false;
  bool bChanged = false;
  {
    CSingleLock lock(m_critSection);

    for (MapTags::reverse_iterator it = m_tags.rbegin(); it != m_tags.rend(); ++it)
    {
      for (VecTimerInfoTag::iterator timerIt = it->second.begin(); timerIt != it->second.end(); ++timerIt)
      {
        bool bDeleteActiveItem = !bCurrentlyActiveOnly || (*timerIt)->IsRecording();
        bool bDeleteTimerRuleItem = bDeleteTimerRules || !(*timerIt)->IsTimerRule();
        bool bChannelsMatch = (*timerIt)->HasChannel() && (*timerIt)->Channel() == channel;

        if (bDeleteActiveItem && bDeleteTimerRuleItem && bChannelsMatch)
        {
          CLog::LogFC(LOGDEBUG, LOGPVR, "Deleted timer %d on client %d",
                      (*timerIt)->m_iClientIndex, (*timerIt)->m_iClientId);
          bReturn = ((*timerIt)->DeleteFromClient(true) == TimerOperationResult::OK) || bReturn;
          bChanged = true;
        }
      }
    }
  }

  if (bChanged)
    NotifyTimersEvent();

  return bReturn;
}

std::shared_ptr<CPVRTimerInfoTag> CPVRTimers::UpdateEntry(const std::shared_ptr<CPVRTimerInfoTag>& timer)
{
  bool bChanged = false;

  CSingleLock lock(m_critSection);
  std::shared_ptr<CPVRTimerInfoTag> tag = GetByClient(timer->m_iClientId, timer->m_iClientIndex);
  if (tag)
  {
    bool bReinsert = tag->StartAsUTC() != timer->StartAsUTC();
    if (bReinsert)
    {
      RemoveEntry(tag);
    }

    bChanged = tag->UpdateEntry(timer);

    if (bReinsert)
    {
      InsertEntry(tag);
    }
  }
  else
  {
    tag.reset(new CPVRTimerInfoTag());
    if (tag->UpdateEntry(timer))
    {
      tag->m_iTimerId = ++m_iLastId;
      InsertEntry(tag);
      bChanged = true;
    }
  }

  return bChanged ? tag : std::shared_ptr<CPVRTimerInfoTag>();
}

bool CPVRTimers::AddTimer(const CPVRTimerInfoTagPtr &tag)
{
  bool bReturn = false;
  if (tag->IsOwnedByClient())
  {
    bReturn = tag->AddToClient();
  }
  else
  {
    bReturn = AddLocalTimer(tag, true);
  }
  return bReturn;
}

TimerOperationResult CPVRTimers::DeleteTimer(const CPVRTimerInfoTagPtr &tag, bool bForce /* = false */, bool bDeleteRule /* = false */)
{
  TimerOperationResult ret = TimerOperationResult::FAILED;
  if (!tag)
    return ret;

  std::shared_ptr<CPVRTimerInfoTag> tagToDelete = tag;

  if (bDeleteRule)
  {
    /* delete the timer rule that scheduled this timer. */
    const std::shared_ptr<CPVRTimerInfoTag> ruleTag = GetTimerRule(tagToDelete);
    if (!ruleTag)
    {
      CLog::LogF(LOGERROR, "Unable to obtain timer rule for given timer");
      return ret;
    }

    tagToDelete = ruleTag;
  }

  if (tagToDelete->IsOwnedByClient())
  {
    ret = tagToDelete->DeleteFromClient(bForce);
  }
  else
  {
    if (DeleteLocalTimer(tagToDelete, true))
      ret = TimerOperationResult::OK;
  }

  return ret;
}

bool CPVRTimers::RenameTimer(const CPVRTimerInfoTagPtr &tag, const std::string &strNewName)
{
  bool bReturn = false;
  if (tag->IsOwnedByClient())
  {
    bReturn = tag->RenameOnClient(strNewName);
  }
  else
  {
    bReturn = RenameLocalTimer(tag, strNewName);
  }
  return bReturn;
}

bool CPVRTimers::UpdateTimer(const CPVRTimerInfoTagPtr &tag)
{
  bool bReturn = false;
  if (tag->IsOwnedByClient())
  {
    bReturn = tag->UpdateOnClient();
  }
  else
  {
    bReturn = UpdateLocalTimer(tag);
  }
  return bReturn;
}

bool CPVRTimers::AddLocalTimer(const std::shared_ptr<CPVRTimerInfoTag>& tag, bool bNotify)
{
  CSingleLock lock(m_critSection);

  const std::shared_ptr<CPVRTimerInfoTag> persistedTimer = PersistAndUpdateLocalTimer(tag, nullptr);
  bool bReturn = !!persistedTimer;

  if (bReturn && persistedTimer->IsTimerRule() && persistedTimer->IsActive())
  {
    if (persistedTimer->IsEpgBased())
    {
      // create and persist children of local epg-based timer rule
      const std::vector<CPVREpgInfoTagPtr> epgTags = GetEpgTagsForTimerRule(CPVRTimerRuleMatcher(persistedTimer,
                                                                                                 CDateTime::GetUTCDateTime()));
      for (const auto& epgTag : epgTags)
      {
        const std::shared_ptr<CPVRTimerInfoTag> childTimer = CPVRTimerInfoTag::CreateReminderFromEpg(epgTag, persistedTimer);
        if (childTimer)
        {
          PersistAndUpdateLocalTimer(childTimer, persistedTimer);
        }
      }
    }
    else
    {
      // create and persist children of local time-based timer rule
      const CDateTime nextStart = CPVRTimerRuleMatcher(persistedTimer, CDateTime::GetUTCDateTime()).GetNextTimerStart();
      if (nextStart.IsValid())
      {
        const CDateTimeSpan duration = persistedTimer->EndAsUTC() - persistedTimer->StartAsUTC();
        const std::shared_ptr<CPVRTimerInfoTag> childTimer
          = CPVRTimerInfoTag::CreateReminderFromDate(nextStart, duration.GetSecondsTotal() / 60, persistedTimer);
        if (childTimer)
        {
          PersistAndUpdateLocalTimer(childTimer, persistedTimer);
        }
      }
    }
  }

  if (bNotify && bReturn)
  {
    lock.Leave();
    NotifyTimersEvent();
  }

  return bReturn;
}

bool CPVRTimers::DeleteLocalTimer(const std::shared_ptr<CPVRTimerInfoTag>& tag, bool bNotify)
{
  CSingleLock lock(m_critSection);

  RemoveEntry(tag);

  bool bReturn = tag->DeleteFromDatabase();

  if (bReturn && tag->IsTimerRule())
  {
    // delete children of local timer rule
    for (auto it = m_tags.begin(); it != m_tags.end();)
    {
      for (auto it2 = it->second.begin(); it2 != it->second.end();)
      {
        std::shared_ptr<CPVRTimerInfoTag> timer = *it2;
        if (timer->m_iParentClientIndex == tag->m_iClientIndex)
        {
          tag->UpdateChildState(timer, false);
          it2 = it->second.erase(it2);
          timer->DeleteFromDatabase();
        }
        else
        {
          ++it2;
        }
      }

      if (it->second.empty())
        it = m_tags.erase(it);
      else
        ++it;
    }
  }

  if (bNotify && bReturn)
  {
    lock.Leave();
    NotifyTimersEvent();
  }

  return bReturn;
}

bool CPVRTimers::RenameLocalTimer(const std::shared_ptr<CPVRTimerInfoTag>& tag, const std::string& strNewName)
{
  {
    CSingleLock lock(m_critSection);
    tag->m_strTitle = strNewName;
  }
  // no need to re-create timer and children. changed timer title does not invalidate any children.
  bool bReturn = !!PersistAndUpdateLocalTimer(tag, nullptr);

  if (bReturn)
    NotifyTimersEvent();

  return bReturn;
}

bool CPVRTimers::UpdateLocalTimer(const std::shared_ptr<CPVRTimerInfoTag>& tag)
{
  // delete and re-create timer and children, if any.
  bool bReturn = DeleteLocalTimer(tag, false);

  if (bReturn)
    bReturn = AddLocalTimer(tag, false);

  if (bReturn)
    NotifyTimersEvent();

  return bReturn;
}

std::shared_ptr<CPVRTimerInfoTag> CPVRTimers::PersistAndUpdateLocalTimer(
  const std::shared_ptr<CPVRTimerInfoTag>& timer, const std::shared_ptr<CPVRTimerInfoTag>& parentTimer)
{
  std::shared_ptr<CPVRTimerInfoTag> tag;
  bool bReturn = timer->Persist();
  if (bReturn)
  {
    tag = UpdateEntry(timer);
    if (tag && parentTimer)
      parentTimer->UpdateChildState(timer, true);
  }
  return bReturn ? tag : std::shared_ptr<CPVRTimerInfoTag>();
}

bool CPVRTimers::IsRecordingOnChannel(const CPVRChannel &channel) const
{
  CSingleLock lock(m_critSection);

  for (MapTags::const_iterator it = m_tags.begin(); it != m_tags.end(); ++it)
  {
    for (VecTimerInfoTag::const_iterator timerIt = it->second.begin(); timerIt != it->second.end(); ++timerIt)
    {
      if ((*timerIt)->IsRecording() &&
          (*timerIt)->m_iClientChannelUid == channel.UniqueID() &&
          (*timerIt)->m_iClientId == channel.ClientID())
        return true;
    }
  }

  return false;
}

CPVRTimerInfoTagPtr CPVRTimers::GetActiveTimerForChannel(const CPVRChannelPtr &channel) const
{
  CSingleLock lock(m_critSection);
  for (const auto &tagsEntry : m_tags)
  {
    for (const auto &timersEntry : tagsEntry.second)
    {
      if (timersEntry->IsRecording() &&
          timersEntry->m_iClientChannelUid == channel->UniqueID() &&
          timersEntry->m_iClientId == channel->ClientID())
        return timersEntry;
    }
  }

  return CPVRTimerInfoTagPtr();
}

CPVRTimerInfoTagPtr CPVRTimers::GetTimerForEpgTag(const CPVREpgInfoTagPtr &epgTag) const
{
  if (epgTag)
  {
    CSingleLock lock(m_critSection);

    for (const auto &tagsEntry : m_tags)
    {
      for (const auto &timersEntry : tagsEntry.second)
      {
        if (timersEntry->IsTimerRule())
          continue;

        if (timersEntry->GetEpgInfoTag(false) == epgTag)
          return timersEntry;

        if (timersEntry->m_iClientChannelUid != PVR_CHANNEL_INVALID_UID &&
            timersEntry->m_iClientChannelUid == epgTag->UniqueChannelID())
        {
          if (timersEntry->UniqueBroadcastID() != EPG_TAG_INVALID_UID &&
              timersEntry->UniqueBroadcastID() == epgTag->UniqueBroadcastID())
            return timersEntry;

          if (timersEntry->m_bIsRadio == epgTag->IsRadio() &&
              timersEntry->StartAsUTC() <= epgTag->StartAsUTC() &&
              timersEntry->EndAsUTC() >= epgTag->EndAsUTC())
            return timersEntry;
        }
      }
    }
  }

  return CPVRTimerInfoTagPtr();
}

CPVRTimerInfoTagPtr CPVRTimers::GetTimerRule(const CPVRTimerInfoTagPtr &timer) const
{
  if (timer)
  {
    int iRuleId = timer->GetTimerRuleId();
    if (iRuleId != PVR_TIMER_NO_PARENT)
    {
      int iClientId = timer->m_iClientId;

      CSingleLock lock(m_critSection);
      for (const auto &tagsEntry : m_tags)
      {
        for (const auto &timersEntry : tagsEntry.second)
        {
          if (timersEntry->m_iClientId == iClientId && timersEntry->m_iClientIndex == iRuleId)
            return timersEntry;
        }
      }
    }
  }
  return CPVRTimerInfoTagPtr();
}

CFileItemPtr CPVRTimers::GetTimerRule(const CFileItemPtr &item) const
{
  CPVRTimerInfoTagPtr timer;
  if (item && item->HasEPGInfoTag())
    timer = GetTimerForEpgTag(item->GetEPGInfoTag());
  else if (item && item->HasPVRTimerInfoTag())
    timer = item->GetPVRTimerInfoTag();

  if (timer)
  {
    timer = GetTimerRule(timer);
    if (timer)
      return CFileItemPtr(new CFileItem(timer));
  }
  return CFileItemPtr();
}

void CPVRTimers::Notify(const Observable &obs, const ObservableMessage msg)
{
  switch (msg)
  {
    case ObservableMessageEpgContainer:
      CServiceBroker::GetPVRManager().TriggerTimersUpdate();
      break;
    case ObservableMessageEpg:
    case ObservableMessageEpgItemUpdate:
    {
      CSingleLock lock(m_critSection);
      m_bReminderRulesUpdatePending = true;
      break;
    }
    default:
      break;
  }
}

CDateTime CPVRTimers::GetNextEventTime(void) const
{
  const bool dailywakup = m_settings.GetBoolValue(CSettings::SETTING_PVRPOWERMANAGEMENT_DAILYWAKEUP);
  const CDateTime now = CDateTime::GetUTCDateTime();
  const CDateTimeSpan prewakeup(0, 0, m_settings.GetIntValue(CSettings::SETTING_PVRPOWERMANAGEMENT_PREWAKEUP), 0);
  const CDateTimeSpan idle(0, 0, m_settings.GetIntValue(CSettings::SETTING_PVRPOWERMANAGEMENT_BACKENDIDLETIME), 0);

  CDateTime wakeuptime;

  /* Check next active time */
  const std::shared_ptr<CPVRTimerInfoTag> timer = GetNextActiveTimer(false);
  if (timer)
  {
    const CDateTimeSpan prestart(0, 0, timer->MarginStart(), 0);
    const CDateTime start = timer->StartAsUTC();
    wakeuptime = ((start - prestart - prewakeup - idle) > now) ?
        start - prestart - prewakeup :
        now + idle;
  }

  /* check daily wake up */
  if (dailywakup)
  {
    CDateTime dailywakeuptime;
    dailywakeuptime.SetFromDBTime(m_settings.GetStringValue(CSettings::SETTING_PVRPOWERMANAGEMENT_DAILYWAKEUPTIME));
    dailywakeuptime = dailywakeuptime.GetAsUTCDateTime();

    dailywakeuptime.SetDateTime(
      now.GetYear(), now.GetMonth(), now.GetDay(),
      dailywakeuptime.GetHour(), dailywakeuptime.GetMinute(), dailywakeuptime.GetSecond()
    );

    if ((dailywakeuptime - idle) < now)
    {
      const CDateTimeSpan oneDay(1,0,0,0);
      dailywakeuptime += oneDay;
    }
    if (!wakeuptime.IsValid() || dailywakeuptime < wakeuptime)
      wakeuptime = dailywakeuptime;
  }

  const CDateTime retVal(wakeuptime);
  return retVal;
}

void CPVRTimers::UpdateChannels(void)
{
  CSingleLock lock(m_critSection);
  for (MapTags::iterator it = m_tags.begin(); it != m_tags.end(); ++it)
  {
    for (VecTimerInfoTag::iterator timerIt = it->second.begin(); timerIt != it->second.end(); ++timerIt)
      (*timerIt)->UpdateChannel();
  }
}

void CPVRTimers::GetAll(CFileItemList& items) const
{
  CFileItemPtr item;
  CSingleLock lock(m_critSection);
  for (MapTags::const_iterator it = m_tags.begin(); it != m_tags.end(); ++it)
  {
    for (VecTimerInfoTag::const_iterator timerIt = it->second.begin(); timerIt != it->second.end(); ++timerIt)
    {
      item.reset(new CFileItem(*timerIt));
      items.Add(item);
    }
  }
}

CPVRTimerInfoTagPtr CPVRTimers::GetById(unsigned int iTimerId) const
{
  CPVRTimerInfoTagPtr item;
  CSingleLock lock(m_critSection);
  for (MapTags::const_iterator it = m_tags.begin(); !item && it != m_tags.end(); ++it)
  {
    for (VecTimerInfoTag::const_iterator timerIt = it->second.begin(); !item && timerIt != it->second.end(); ++timerIt)
    {
      if ((*timerIt)->m_iTimerId == iTimerId)
        item = *timerIt;
    }
  }
  return item;
}

void CPVRTimers::NotifyTimersEvent(bool bAddedOrDeleted /* = true */)
{
  CServiceBroker::GetPVRManager().SetChanged();
  CServiceBroker::GetPVRManager().NotifyObservers(bAddedOrDeleted
                                                  ? ObservableMessageTimersReset
                                                  : ObservableMessageTimers);

  if (bAddedOrDeleted)
    CServiceBroker::GetPVRManager().PublishEvent(PVREvent::TimersInvalidated);
}
