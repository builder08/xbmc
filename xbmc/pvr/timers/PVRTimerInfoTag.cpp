/*
 *  Copyright (C) 2012-2018 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#include "PVRTimerInfoTag.h"

#include <ctime>

#include "ServiceBroker.h"
#include "addons/PVRClient.h"
#include "guilib/LocalizeStrings.h"
#include "settings/AdvancedSettings.h"
#include "settings/Settings.h"
#include "settings/SettingsComponent.h"
#include "utils/StringUtils.h"
#include "utils/URIUtils.h"
#include "utils/Variant.h"
#include "utils/log.h"

#include "pvr/PVRDatabase.h"
#include "pvr/PVRManager.h"
#include "pvr/addons/PVRClients.h"
#include "pvr/channels/PVRChannelGroupsContainer.h"
#include "pvr/epg/Epg.h"
#include "pvr/timers/PVRTimersPath.h"

using namespace PVR;

CPVRTimerInfoTag::CPVRTimerInfoTag(bool bRadio /* = false */) :
  m_strTitle(g_localizeStrings.Get(19056)), // New Timer
  m_iClientId(CServiceBroker::GetPVRManager().Clients()->GetFirstCreatedClientID()),
  m_iClientIndex(PVR_TIMER_NO_CLIENT_INDEX),
  m_iParentClientIndex(PVR_TIMER_NO_PARENT),
  m_iClientChannelUid(PVR_CHANNEL_INVALID_UID),
  m_iPriority(DEFAULT_RECORDING_PRIORITY),
  m_iLifetime(DEFAULT_RECORDING_LIFETIME),
  m_iPreventDupEpisodes(DEFAULT_RECORDING_DUPLICATEHANDLING),
  m_bIsRadio(bRadio),
  m_iMarginStart(CServiceBroker::GetSettingsComponent()->GetSettings()->GetInt(CSettings::SETTING_PVRRECORD_MARGINSTART)),
  m_iMarginEnd(CServiceBroker::GetSettingsComponent()->GetSettings()->GetInt(CSettings::SETTING_PVRRECORD_MARGINEND)),
  m_iEpgUid(EPG_TAG_INVALID_UID),
  m_StartTime(CDateTime::GetUTCDateTime()),
  m_StopTime(m_StartTime + CDateTimeSpan(0, 2, 0, 0)),
  m_FirstDay(m_StartTime)
{
  static const unsigned int iMustHaveAttr = PVR_TIMER_TYPE_IS_MANUAL;
  static const unsigned int iMustNotHaveAttr = PVR_TIMER_TYPE_IS_REPEATING | PVR_TIMER_TYPE_FORBIDS_NEW_INSTANCES;

  std::shared_ptr<CPVRTimerType> type;

  const CPVRClientPtr client = CServiceBroker::GetPVRManager().GetClient(m_iClientId);
  if (client && client->GetClientCapabilities().SupportsTimers())
  {
    // default to first available type for given client
    type = CPVRTimerType::GetFirstAvailableType(client);
  }

  // fallback to manual one-shot reminder, which is always available
  if (!type)
    type = CPVRTimerType::CreateFromAttributes(iMustHaveAttr | PVR_TIMER_TYPE_IS_REMINDER, iMustNotHaveAttr, m_iClientId);

  if (type)
    SetTimerType(type);
  else
    CLog::LogF(LOGERROR, "Unable to obtain timer type!");

  m_iWeekdays = (m_timerType && m_timerType->IsTimerRule()) ? PVR_WEEKDAY_ALLDAYS : PVR_WEEKDAY_NONE;

  UpdateSummary();
}

CPVRTimerInfoTag::CPVRTimerInfoTag(const PVR_TIMER &timer, const CPVRChannelPtr &channel, unsigned int iClientId) :
  m_strTitle(timer.strTitle),
  m_strEpgSearchString(timer.strEpgSearchString),
  m_bFullTextEpgSearch(timer.bFullTextEpgSearch),
  m_strDirectory(timer.strDirectory),
  m_state(timer.state),
  m_iClientId(iClientId),
  m_iClientIndex(timer.iClientIndex),
  m_iParentClientIndex(timer.iParentClientIndex),
  m_iClientChannelUid(channel ? channel->UniqueID() : (timer.iClientChannelUid > 0) ? timer.iClientChannelUid : PVR_CHANNEL_INVALID_UID),
  m_bStartAnyTime(timer.bStartAnyTime),
  m_bEndAnyTime(timer.bEndAnyTime),
  m_iPriority(timer.iPriority),
  m_iLifetime(timer.iLifetime),
  m_iMaxRecordings(timer.iMaxRecordings),
  m_iWeekdays(timer.iWeekdays),
  m_iPreventDupEpisodes(timer.iPreventDuplicateEpisodes),
  m_iRecordingGroup(timer.iRecordingGroup),
  m_strFileNameAndPath(StringUtils::Format("pvr://client%i/timers/%i", m_iClientId, m_iClientIndex)),
  m_bIsRadio(channel && channel->IsRadio()),
  m_iMarginStart(timer.iMarginStart),
  m_iMarginEnd(timer.iMarginEnd),
  m_iEpgUid(timer.iEpgUid),
  m_strSeriesLink(timer.strSeriesLink),
  m_StartTime(timer.startTime + CServiceBroker::GetSettingsComponent()->GetAdvancedSettings()->m_iPVRTimeCorrection),
  m_StopTime(timer.endTime + CServiceBroker::GetSettingsComponent()->GetAdvancedSettings()->m_iPVRTimeCorrection),
  m_iGenreType(timer.iGenreType),
  m_iGenreSubType(timer.iGenreSubType),
  m_strGenreDescription(timer.strGenreDescription),
  m_strPlotOutline(timer.strPlotOutline),
  m_strPlot(timer.strPlot),
  m_iYear(timer.iYear),
  m_iSeriesNumber(timer.iSeriesNumber),
  m_iEpisodeNumber(timer.iEpisodeNumber),
  m_iEpisodePartNumber(timer.iEpisodePartNumber),
  m_channel(channel)
{
  if (timer.firstDay)
    m_FirstDay = CDateTime(timer.firstDay + CServiceBroker::GetSettingsComponent()->GetAdvancedSettings()->m_iPVRTimeCorrection);
  else
    m_FirstDay = CDateTime::GetUTCDateTime();

  if (m_iClientIndex == PVR_TIMER_NO_CLIENT_INDEX)
    CLog::LogF(LOGERROR, "Invalid client index supplied by client %d (must be > 0)!", m_iClientId);

  const CPVRClientPtr client = CServiceBroker::GetPVRManager().GetClient(m_iClientId);
  if (client && client->GetClientCapabilities().SupportsTimers())
  {
    // begin compat section

    // Create timer type according to certain timer values already available in Isengard.
    // This is for migration only and does not make changes to the addons obsolete. Addons should
    // work and benefit from some UI changes (e.g. some of the timer settings dialog enhancements),
    // but all old problems/bugs due to static attributes and values will remain the same as in
    // Isengard. Also, new features (like epg search) are not available to addons automatically.
    // This code can be removed once all addons actually support the respective PVR Addon API version.
    if (timer.iTimerType == PVR_TIMER_TYPE_NONE)
    {
      // Create type according to certain timer values.
      unsigned int iMustHave    = PVR_TIMER_TYPE_ATTRIBUTE_NONE;
      unsigned int iMustNotHave = PVR_TIMER_TYPE_FORBIDS_NEW_INSTANCES;

      if (timer.iEpgUid == PVR_TIMER_NO_EPG_UID && timer.iWeekdays != PVR_WEEKDAY_NONE)
        iMustHave |= PVR_TIMER_TYPE_IS_REPEATING;
      else
        iMustNotHave |= PVR_TIMER_TYPE_IS_REPEATING;

      if (timer.iEpgUid == PVR_TIMER_NO_EPG_UID)
        iMustHave |= PVR_TIMER_TYPE_IS_MANUAL;
      else
        iMustNotHave |= PVR_TIMER_TYPE_IS_MANUAL;

      const std::shared_ptr<CPVRTimerType> type = CPVRTimerType::CreateFromAttributes(iMustHave, iMustNotHave, m_iClientId);

      if (type)
        SetTimerType(type);
    }
    // end compat section
    else
    {
      SetTimerType(CPVRTimerType::CreateFromIds(timer.iTimerType, m_iClientId));
    }

    if (!m_timerType)
      CLog::LogF(LOGERROR, "No timer type, although timers are supported by client %d!", m_iClientId);
    else if (m_iEpgUid == EPG_TAG_INVALID_UID && m_timerType->IsEpgBasedOnetime())
      CLog::LogF(LOGERROR, "No epg tag given for epg based timer type (%d)!", m_timerType->GetTypeId());
  }

  UpdateSummary();
  UpdateEpgInfoTag();
}

bool CPVRTimerInfoTag::operator ==(const CPVRTimerInfoTag& right) const
{
  bool bChannelsMatch = true;
  if (m_channel && right.m_channel)
    bChannelsMatch = *m_channel == *right.m_channel;
  else if (m_channel != right.m_channel)
    bChannelsMatch = false;

  return (bChannelsMatch &&
          m_iClientIndex        == right.m_iClientIndex &&
          m_iParentClientIndex  == right.m_iParentClientIndex &&
          m_strSummary          == right.m_strSummary &&
          m_iClientChannelUid   == right.m_iClientChannelUid &&
          m_bIsRadio            == right.m_bIsRadio &&
          m_iPreventDupEpisodes == right.m_iPreventDupEpisodes &&
          m_iRecordingGroup     == right.m_iRecordingGroup &&
          m_StartTime           == right.m_StartTime &&
          m_StopTime            == right.m_StopTime &&
          m_bStartAnyTime       == right.m_bStartAnyTime &&
          m_bEndAnyTime         == right.m_bEndAnyTime &&
          m_FirstDay            == right.m_FirstDay &&
          m_iWeekdays           == right.m_iWeekdays &&
          m_iPriority           == right.m_iPriority &&
          m_iLifetime           == right.m_iLifetime &&
          m_iMaxRecordings      == right.m_iMaxRecordings &&
          m_strFileNameAndPath  == right.m_strFileNameAndPath &&
          m_strTitle            == right.m_strTitle &&
          m_strEpgSearchString  == right.m_strEpgSearchString &&
          m_bFullTextEpgSearch  == right.m_bFullTextEpgSearch &&
          m_strDirectory        == right.m_strDirectory &&
          m_iClientId           == right.m_iClientId &&
          m_iMarginStart        == right.m_iMarginStart &&
          m_iMarginEnd          == right.m_iMarginEnd &&
          m_state               == right.m_state &&
          m_timerType           == right.m_timerType &&
          m_iTimerId            == right.m_iTimerId &&
          m_strSeriesLink       == right.m_strSeriesLink &&
          m_iEpgUid             == right.m_iEpgUid &&
          m_iGenreType          == right.m_iGenreType &&
          m_iGenreSubType       == right.m_iGenreSubType &&
          m_strGenreDescription == right.m_strGenreDescription &&
          m_strPlotOutline      == right.m_strPlotOutline &&
          m_strPlot             == right.m_strPlot &&
          m_iYear               == right.m_iYear &&
          m_iSeriesNumber       == right.m_iSeriesNumber &&
          m_iEpisodeNumber      == right.m_iEpisodeNumber &&
          m_iEpisodePartNumber  == right.m_iEpisodePartNumber &&
          m_iTVChildTimersActive == right.m_iTVChildTimersActive &&
          m_iTVChildTimersConflictNOK == right.m_iTVChildTimersConflictNOK &&
          m_iTVChildTimersRecording == right.m_iTVChildTimersRecording &&
          m_iTVChildTimersErrors == right.m_iTVChildTimersErrors &&
          m_iRadioChildTimersActive == right.m_iRadioChildTimersActive &&
          m_iRadioChildTimersConflictNOK == right.m_iRadioChildTimersConflictNOK &&
          m_iRadioChildTimersRecording == right.m_iRadioChildTimersRecording &&
          m_iRadioChildTimersErrors == right.m_iRadioChildTimersErrors);
}

/**
 * Compare not equal for two CPVRTimerInfoTag
 */
bool CPVRTimerInfoTag::operator !=(const CPVRTimerInfoTag& right) const
{
  return !(*this == right);
}

void CPVRTimerInfoTag::Serialize(CVariant &value) const
{
  value["channelid"] = m_channel != NULL ? m_channel->ChannelID() : -1;
  value["summary"] = m_strSummary;
  value["isradio"] = m_bIsRadio;
  value["preventduplicateepisodes"] = m_iPreventDupEpisodes;
  value["starttime"] = m_StartTime.IsValid() ? m_StartTime.GetAsDBDateTime() : "";
  value["endtime"] = m_StopTime.IsValid() ? m_StopTime.GetAsDBDateTime() : "";
  value["startanytime"] = m_bStartAnyTime;
  value["endanytime"] = m_bEndAnyTime;
  value["runtime"] = m_StartTime.IsValid() && m_StopTime.IsValid() ? (m_StopTime - m_StartTime).GetSecondsTotal() : 0;
  value["firstday"] = m_FirstDay.IsValid() ? m_FirstDay.GetAsDBDate() : "";

  CVariant weekdays(CVariant::VariantTypeArray);
  if (m_iWeekdays & PVR_WEEKDAY_MONDAY)
    weekdays.push_back("monday");
  if (m_iWeekdays & PVR_WEEKDAY_TUESDAY)
    weekdays.push_back("tuesday");
  if (m_iWeekdays & PVR_WEEKDAY_WEDNESDAY)
    weekdays.push_back("wednesday");
  if (m_iWeekdays & PVR_WEEKDAY_THURSDAY)
    weekdays.push_back("thursday");
  if (m_iWeekdays & PVR_WEEKDAY_FRIDAY)
    weekdays.push_back("friday");
  if (m_iWeekdays & PVR_WEEKDAY_SATURDAY)
    weekdays.push_back("saturday");
  if (m_iWeekdays & PVR_WEEKDAY_SUNDAY)
    weekdays.push_back("sunday");
  value["weekdays"] = weekdays;

  value["priority"] = m_iPriority;
  value["lifetime"] = m_iLifetime;
  value["title"] = m_strTitle;
  value["directory"] = m_strDirectory;
  value["startmargin"] = m_iMarginStart;
  value["endmargin"] = m_iMarginEnd;

  value["timerid"] = m_iTimerId;

  switch (m_state)
  {
  case PVR_TIMER_STATE_NEW:
    value["state"] = "new";
    break;
  case PVR_TIMER_STATE_SCHEDULED:
    value["state"] = "scheduled";
    break;
  case PVR_TIMER_STATE_RECORDING:
    value["state"] = "recording";
    break;
  case PVR_TIMER_STATE_COMPLETED:
    value["state"] = "completed";
    break;
  case PVR_TIMER_STATE_ABORTED:
    value["state"] = "aborted";
    break;
  case PVR_TIMER_STATE_CANCELLED:
    value["state"] = "cancelled";
    break;
  case PVR_TIMER_STATE_CONFLICT_OK:
    value["state"] = "conflict_ok";
    break;
  case PVR_TIMER_STATE_CONFLICT_NOK:
    value["state"] = "conflict_notok";
    break;
  case PVR_TIMER_STATE_ERROR:
    value["state"] = "error";
    break;
  case PVR_TIMER_STATE_DISABLED:
    value["state"] = "disabled";
    break;
  default:
    value["state"] = "unknown";
    break;
  }

  value["istimerrule"] = m_timerType && m_timerType->IsTimerRule();
  value["ismanual"] = m_timerType && m_timerType->IsManual();
  value["isreadonly"] = m_timerType && m_timerType->IsReadOnly();

  value["epgsearchstring"]   = m_strEpgSearchString;
  value["fulltextepgsearch"] = m_bFullTextEpgSearch;
  value["recordinggroup"]    = m_iRecordingGroup;
  value["maxrecordings"]     = m_iMaxRecordings;
  value["epguid"]            = m_iEpgUid;
  value["serieslink"]        = m_strSeriesLink;

  value["genretype"]         = m_iGenreType;
  value["genresubtype"]      = m_iGenreSubType;
  value["genredescription"]  = m_strGenreDescription;
  value["plotoutline"]       = m_strPlotOutline;
  value["plot"]              = m_strPlot;
  value["year"]              = m_iYear;
  value["seriesnumber"]      = m_iSeriesNumber;
  value["episodenumber"]     = m_iEpisodeNumber;
  value["episodepartnumber"] = m_iEpisodePartNumber;
}

void CPVRTimerInfoTag::UpdateSummary(void)
{
  CSingleLock lock(m_critSection);
  m_strSummary.clear();

  const std::string startDate(StartAsLocalTime().GetAsLocalizedDate());
  const std::string endDate(EndAsLocalTime().GetAsLocalizedDate());

  if (m_bEndAnyTime)
  {
    m_strSummary = StringUtils::Format("%s %s %s",
        m_iWeekdays != PVR_WEEKDAY_NONE ?
          GetWeekdaysString().c_str() : startDate.c_str(), //for "Any day" set PVR_WEEKDAY_ALLDAYS
        g_localizeStrings.Get(19107).c_str(), // "at"
        m_bStartAnyTime ?
          g_localizeStrings.Get(19161).c_str() /* "any time" */ : StartAsLocalTime().GetAsLocalizedTime("", false).c_str());
  }
  else if ((m_iWeekdays != PVR_WEEKDAY_NONE) || (startDate == endDate))
  {
    m_strSummary = StringUtils::Format("%s %s %s %s %s",
        m_iWeekdays != PVR_WEEKDAY_NONE ?
          GetWeekdaysString().c_str() : startDate.c_str(), //for "Any day" set PVR_WEEKDAY_ALLDAYS
        g_localizeStrings.Get(19159).c_str(), // "from"
        m_bStartAnyTime ?
          g_localizeStrings.Get(19161).c_str() /* "any time" */ : StartAsLocalTime().GetAsLocalizedTime("", false).c_str(),
        g_localizeStrings.Get(19160).c_str(), // "to"
        m_bEndAnyTime ?
          g_localizeStrings.Get(19161).c_str() /* "any time" */ : EndAsLocalTime().GetAsLocalizedTime("", false).c_str());
  }
  else
  {
    m_strSummary = StringUtils::Format("%s %s %s %s %s %s",
        startDate.c_str(),
        g_localizeStrings.Get(19159).c_str(), // "from"
        m_bStartAnyTime ?
          g_localizeStrings.Get(19161).c_str() /* "any time" */ : StartAsLocalTime().GetAsLocalizedTime("", false).c_str(),
        g_localizeStrings.Get(19160).c_str(), // "to"
        endDate.c_str(),
        m_bEndAnyTime ?
          g_localizeStrings.Get(19161).c_str() /* "any time" */ : EndAsLocalTime().GetAsLocalizedTime("", false).c_str());
  }
}

void CPVRTimerInfoTag::SetTimerType(const CPVRTimerTypePtr &type)
{
  CSingleLock lock(m_critSection);
  m_timerType = type;

  if (m_timerType && m_iClientIndex == PVR_TIMER_NO_CLIENT_INDEX)
  {
    m_iPriority           = m_timerType->GetPriorityDefault();
    m_iLifetime           = m_timerType->GetLifetimeDefault();
    m_iMaxRecordings      = m_timerType->GetMaxRecordingsDefault();
    m_iPreventDupEpisodes = m_timerType->GetPreventDuplicateEpisodesDefault();
    m_iRecordingGroup     = m_timerType->GetRecordingGroupDefault();
  }

  if (m_timerType && !m_timerType->IsTimerRule())
    m_iWeekdays = PVR_WEEKDAY_NONE;
}

std::string CPVRTimerInfoTag::GetStatus(bool bRadio) const
{
  std::string strReturn = g_localizeStrings.Get(305);
  CSingleLock lock(m_critSection);
  if (URIUtils::PathEquals(m_strFileNameAndPath, CPVRTimersPath::PATH_ADDTIMER))
    strReturn = g_localizeStrings.Get(19026);
  else if (m_state == PVR_TIMER_STATE_CANCELLED || m_state == PVR_TIMER_STATE_ABORTED)
    strReturn = g_localizeStrings.Get(13106);
  else if (m_state == PVR_TIMER_STATE_RECORDING)
    strReturn = g_localizeStrings.Get(19162);
  else if (m_state == PVR_TIMER_STATE_CONFLICT_OK)
    strReturn = g_localizeStrings.Get(19275);
  else if (m_state == PVR_TIMER_STATE_CONFLICT_NOK)
    strReturn = g_localizeStrings.Get(19276);
  else if (m_state == PVR_TIMER_STATE_ERROR)
    strReturn = g_localizeStrings.Get(257);
  else if (m_state == PVR_TIMER_STATE_DISABLED)
    strReturn = g_localizeStrings.Get(13106);
  else if (m_state == PVR_TIMER_STATE_COMPLETED)
  {
    if ((m_iTVChildTimersRecording > 0 && !bRadio) || (m_iRadioChildTimersRecording > 0 && bRadio))
      strReturn = g_localizeStrings.Get(19162); // "Recording active"
    else
      strReturn = g_localizeStrings.Get(19256); // "Completed"
  }
  else if (m_state == PVR_TIMER_STATE_SCHEDULED || m_state == PVR_TIMER_STATE_NEW)
  {
    if ((m_iTVChildTimersRecording > 0 && !bRadio) || (m_iRadioChildTimersRecording > 0 && bRadio))
      strReturn = g_localizeStrings.Get(19162); // "Recording active"
    else if ((m_iTVChildTimersErrors > 0 && !bRadio) || (m_iRadioChildTimersErrors > 0 && bRadio))
      strReturn = g_localizeStrings.Get(257);   // "Error"
    else if ((m_iTVChildTimersConflictNOK > 0 && !bRadio) || (m_iRadioChildTimersConflictNOK > 0 && bRadio))
      strReturn = g_localizeStrings.Get(19276); // "Conflict error"
    else if ((m_iTVChildTimersActive > 0 && !bRadio) || (m_iRadioChildTimersActive > 0 && bRadio))
      strReturn = StringUtils::Format(g_localizeStrings.Get(19255).c_str(),
                                      bRadio ? m_iRadioChildTimersActive : m_iTVChildTimersActive); // "%d scheduled"
  }

  return strReturn;
}

/**
 * Get the type string of this timer
 */
std::string CPVRTimerInfoTag::GetTypeAsString() const
{
  CSingleLock lock(m_critSection);
  return m_timerType ? m_timerType->GetDescription() : "";
}

namespace
{
void AppendDay(std::string &strReturn, unsigned int iId)
{
  if (!strReturn.empty())
    strReturn += "-";

  if (iId > 0)
    strReturn += g_localizeStrings.Get(iId).c_str();
  else
    strReturn += "__";
}
} // unnamed namespace

std::string CPVRTimerInfoTag::GetWeekdaysString(unsigned int iWeekdays, bool bEpgBased, bool bLongMultiDaysFormat)
{
  std::string strReturn;

  if (iWeekdays == PVR_WEEKDAY_NONE)
    return strReturn;
  else if (iWeekdays == PVR_WEEKDAY_ALLDAYS)
    strReturn = bEpgBased
              ? g_localizeStrings.Get(807)  // "Any day"
              : g_localizeStrings.Get(808); // "Every day"
  else if (iWeekdays == PVR_WEEKDAY_MONDAY)
    strReturn = g_localizeStrings.Get(831); // "Mondays"
  else if (iWeekdays == PVR_WEEKDAY_TUESDAY)
    strReturn = g_localizeStrings.Get(832); // "Tuesdays"
  else if (iWeekdays == PVR_WEEKDAY_WEDNESDAY)
    strReturn = g_localizeStrings.Get(833); // "Wednesdays"
  else if (iWeekdays == PVR_WEEKDAY_THURSDAY)
    strReturn = g_localizeStrings.Get(834); // "Thursdays"
  else if (iWeekdays == PVR_WEEKDAY_FRIDAY)
    strReturn = g_localizeStrings.Get(835); // "Fridays"
  else if (iWeekdays == PVR_WEEKDAY_SATURDAY)
    strReturn = g_localizeStrings.Get(836); // "Saturdays"
  else if (iWeekdays == PVR_WEEKDAY_SUNDAY)
    strReturn = g_localizeStrings.Get(837); // "Sundays"
  else
  {
    // Any other combination. Assemble custom string.
    if (iWeekdays & PVR_WEEKDAY_MONDAY)
      AppendDay(strReturn, 19149); // Mo
    else if (bLongMultiDaysFormat)
      AppendDay(strReturn, 0);

    if (iWeekdays & PVR_WEEKDAY_TUESDAY)
      AppendDay(strReturn, 19150); // Tu
    else if (bLongMultiDaysFormat)
      AppendDay(strReturn, 0);

    if (iWeekdays & PVR_WEEKDAY_WEDNESDAY)
      AppendDay(strReturn, 19151); // We
    else if (bLongMultiDaysFormat)
      AppendDay(strReturn, 0);

    if (iWeekdays & PVR_WEEKDAY_THURSDAY)
      AppendDay(strReturn, 19152); // Th
    else if (bLongMultiDaysFormat)
      AppendDay(strReturn, 0);

    if (iWeekdays & PVR_WEEKDAY_FRIDAY)
      AppendDay(strReturn, 19153); // Fr
    else if (bLongMultiDaysFormat)
      AppendDay(strReturn, 0);

    if (iWeekdays & PVR_WEEKDAY_SATURDAY)
      AppendDay(strReturn, 19154); // Sa
    else if (bLongMultiDaysFormat)
      AppendDay(strReturn, 0);

    if (iWeekdays & PVR_WEEKDAY_SUNDAY)
      AppendDay(strReturn, 19155); // So
    else if (bLongMultiDaysFormat)
      AppendDay(strReturn, 0);
  }
  return strReturn;
}

std::string CPVRTimerInfoTag::GetWeekdaysString() const
{
  CSingleLock lock(m_critSection);
  return GetWeekdaysString(m_iWeekdays, m_timerType ? m_timerType->IsEpgBased() : false, false);
}

bool CPVRTimerInfoTag::IsOwnedByClient() const
{
  return m_timerType && m_timerType->GetClientId() > -1;
}

bool CPVRTimerInfoTag::AddToClient(void) const
{
  const CPVRClientPtr client = CServiceBroker::GetPVRManager().GetClient(m_iClientId);
  if (client)
    return client->AddTimer(*this) == PVR_ERROR_NO_ERROR;
  return false;
}

TimerOperationResult CPVRTimerInfoTag::DeleteFromClient(bool bForce /* = false */) const
{
  const CPVRClientPtr client = CServiceBroker::GetPVRManager().GetClient(m_iClientId);
  PVR_ERROR error = PVR_ERROR_UNKNOWN;

  if (client)
    error = client->DeleteTimer(*this, bForce);

  if (error == PVR_ERROR_RECORDING_RUNNING)
    return TimerOperationResult::RECORDING;

  return (error == PVR_ERROR_NO_ERROR) ? TimerOperationResult::OK : TimerOperationResult::FAILED;
}

bool CPVRTimerInfoTag::RenameOnClient(const std::string &strNewName)
{
  {
    // set the new timer title locally
    CSingleLock lock(m_critSection);
    m_strTitle = strNewName;
  }

  // update timer data in the backend
  return UpdateOnClient();
}

bool CPVRTimerInfoTag::Persist()
{
  const std::shared_ptr<CPVRDatabase> database = CServiceBroker::GetPVRManager().GetTVDatabase();
  if (database)
    return database->Persist(*this);

  return false;
}

bool CPVRTimerInfoTag::DeleteFromDatabase()
{
  const std::shared_ptr<CPVRDatabase> database = CServiceBroker::GetPVRManager().GetTVDatabase();
  if (database)
    return database->Delete(*this);

  return false;
}

bool CPVRTimerInfoTag::UpdateEntry(const CPVRTimerInfoTagPtr &tag)
{
  CSingleLock lock(m_critSection);

  m_iClientId           = tag->m_iClientId;
  m_iClientIndex        = tag->m_iClientIndex;
  m_iParentClientIndex  = tag->m_iParentClientIndex;
  m_strTitle            = tag->m_strTitle;
  m_strEpgSearchString  = tag->m_strEpgSearchString;
  m_bFullTextEpgSearch  = tag->m_bFullTextEpgSearch;
  m_strDirectory        = tag->m_strDirectory;
  m_iClientChannelUid   = tag->m_iClientChannelUid;
  m_StartTime           = tag->m_StartTime;
  m_StopTime            = tag->m_StopTime;
  m_bStartAnyTime       = tag->m_bStartAnyTime;
  m_bEndAnyTime         = tag->m_bEndAnyTime;
  m_FirstDay            = tag->m_FirstDay;
  m_iPriority           = tag->m_iPriority;
  m_iLifetime           = tag->m_iLifetime;
  m_iMaxRecordings      = tag->m_iMaxRecordings;
  m_state               = tag->m_state;
  m_iPreventDupEpisodes = tag->m_iPreventDupEpisodes;
  m_iRecordingGroup     = tag->m_iRecordingGroup;
  m_iWeekdays           = tag->m_iWeekdays;
  m_bIsRadio            = tag->m_bIsRadio;
  m_iMarginStart        = tag->m_iMarginStart;
  m_iMarginEnd          = tag->m_iMarginEnd;
  m_strSeriesLink       = tag->m_strSeriesLink;
  m_iEpgUid             = tag->m_iEpgUid;
  m_epgTag              = tag->m_epgTag;
  m_strSummary          = tag->m_strSummary;
  m_channel             = tag->m_channel;
  m_iGenreType          = tag->m_iGenreType;
  m_iGenreSubType       = tag->m_iGenreSubType;
  m_strGenreDescription = tag->m_strGenreDescription;
  m_strPlotOutline      = tag->m_strPlotOutline;
  m_strPlot             = tag->m_strPlot;
  m_iYear               = tag->m_iYear;
  m_iSeriesNumber       = tag->m_iSeriesNumber;
  m_iEpisodeNumber      = tag->m_iEpisodeNumber;
  m_iEpisodePartNumber  = tag->m_iEpisodePartNumber;

  m_iTVChildTimersActive = tag->m_iTVChildTimersActive;
  m_iTVChildTimersConflictNOK = tag->m_iTVChildTimersConflictNOK;
  m_iTVChildTimersRecording = tag->m_iTVChildTimersRecording;
  m_iTVChildTimersErrors = tag->m_iTVChildTimersErrors;
  m_iRadioChildTimersActive = tag->m_iRadioChildTimersActive;
  m_iRadioChildTimersConflictNOK = tag->m_iRadioChildTimersConflictNOK;
  m_iRadioChildTimersRecording = tag->m_iRadioChildTimersRecording;
  m_iRadioChildTimersErrors = tag->m_iRadioChildTimersErrors;

  SetTimerType(tag->m_timerType);

  if (m_strSummary.empty())
    UpdateSummary();

  UpdateEpgInfoTag();

  return true;
}

bool CPVRTimerInfoTag::UpdateChildState(const std::shared_ptr<CPVRTimerInfoTag>& childTimer, bool bAdd)
{
  if (!childTimer || childTimer->m_iParentClientIndex != m_iClientIndex)
    return false;

  int iDelta = bAdd ? +1 : -1;
  switch (childTimer->m_state)
  {
  case PVR_TIMER_STATE_NEW:
  case PVR_TIMER_STATE_SCHEDULED:
  case PVR_TIMER_STATE_CONFLICT_OK:
    if (childTimer->m_bIsRadio)
      m_iRadioChildTimersActive += iDelta;
    else
      m_iTVChildTimersActive += iDelta;
    break;
  case PVR_TIMER_STATE_RECORDING:
    if (childTimer->m_bIsRadio)
    {
      m_iRadioChildTimersActive += iDelta;
      m_iRadioChildTimersRecording += iDelta;
    }
    else
    {
      m_iTVChildTimersActive += iDelta;
      m_iTVChildTimersRecording += iDelta;
    }
    break;
  case PVR_TIMER_STATE_CONFLICT_NOK:
    if (childTimer->m_bIsRadio)
      m_iRadioChildTimersConflictNOK += iDelta;
    else
      m_iTVChildTimersConflictNOK += iDelta;
    break;
  case PVR_TIMER_STATE_ERROR:
    if (childTimer->m_bIsRadio)
      m_iRadioChildTimersErrors += iDelta;
    else
      m_iTVChildTimersErrors += iDelta;
    break;
  case PVR_TIMER_STATE_COMPLETED:
  case PVR_TIMER_STATE_ABORTED:
  case PVR_TIMER_STATE_CANCELLED:
  case PVR_TIMER_STATE_DISABLED:
    //these are not the child timers we are looking for
    break;
  }
  return true;
}

void CPVRTimerInfoTag::ResetChildState()
{
  m_iTVChildTimersActive = 0;
  m_iTVChildTimersRecording = 0;
  m_iTVChildTimersConflictNOK = 0;
  m_iTVChildTimersErrors = 0;
  m_iRadioChildTimersActive = 0;
  m_iRadioChildTimersRecording = 0;
  m_iRadioChildTimersConflictNOK = 0;
  m_iRadioChildTimersErrors = 0;
}

bool CPVRTimerInfoTag::UpdateOnClient()
{
  const CPVRClientPtr client = CServiceBroker::GetPVRManager().GetClient(m_iClientId);
  return client && (client->UpdateTimer(*this) == PVR_ERROR_NO_ERROR);
}

std::string CPVRTimerInfoTag::ChannelName() const
{
  std::string strReturn;
  CPVRChannelPtr channeltag = Channel();
  if (channeltag)
    strReturn = channeltag->ChannelName();
  else if (m_timerType && m_timerType->IsEpgBasedTimerRule())
    strReturn = StringUtils::Format("(%s)", g_localizeStrings.Get(809).c_str()); // "Any channel"

  return strReturn;
}

std::string CPVRTimerInfoTag::ChannelIcon() const
{
  std::string strReturn;
  CPVRChannelPtr channeltag = Channel();
  if (channeltag)
    strReturn = channeltag->IconPath();
  return strReturn;
}

std::shared_ptr<CPVRTimerInfoTag> CPVRTimerInfoTag::CreateReminderFromDate(
  const CDateTime& start, int iDuration,
  const std::shared_ptr<CPVRTimerInfoTag>& parent /* = std::shared_ptr<CPVRTimerInfoTag>() */)
{
  bool bReadOnly = !!parent; // children of reminder rules are always read-only
  const std::shared_ptr<CPVRTimerInfoTag> newTimer = CreateFromDate(parent->Channel(), start, iDuration, true, bReadOnly);
  if (newTimer && parent)
  {
    // set parent
    newTimer->m_iParentClientIndex = parent->m_iClientIndex;

    // set relevant props for the new one-time reminder timer
    newTimer->m_strTitle = parent->Title();
  }
  return newTimer;
}

static const time_t INSTANT_TIMER_START = 0; // PVR addon API: special start time value to denote an instant timer

std::shared_ptr<CPVRTimerInfoTag> CPVRTimerInfoTag::CreateInstantTimerTag(const std::shared_ptr<CPVRChannel>& channel, int iDuration /* = DEFAULT_PVRRECORD_INSTANTRECORDTIME */)
{
  return CreateFromDate(channel, CDateTime(INSTANT_TIMER_START), iDuration, false, false);
}

std::shared_ptr<CPVRTimerInfoTag> CPVRTimerInfoTag::CreateTimerTag(const std::shared_ptr<CPVRChannel>& channel, const CDateTime& start, int iDuration)
{
  return CreateFromDate(channel, start, iDuration, false, false);
}

std::shared_ptr<CPVRTimerInfoTag> CPVRTimerInfoTag::CreateFromDate(
  const std::shared_ptr<CPVRChannel>& channel, const CDateTime& start, int iDuration, bool bCreateReminder, bool bReadOnly)
{
  if (!channel)
  {
    CLog::LogF(LOGERROR, "No channel");
    return CPVRTimerInfoTagPtr();
  }

  bool bInstantStart = (start == CDateTime(INSTANT_TIMER_START));

  std::shared_ptr<CPVREpgInfoTag> epgTag;
  if (!bCreateReminder) // time-based reminders never have epg tags
  {
    if (bInstantStart)
      epgTag = channel->GetEPGNow();
    else if (channel->GetEPG())
      epgTag = channel->GetEPG()->GetTagBetween(start, start + CDateTimeSpan(0, 0, iDuration, 0));
  }

  CPVRTimerInfoTagPtr newTimer;
  if (epgTag)
  {
    if (epgTag->IsRecordable())
    {
      newTimer = CreateFromEpg(epgTag, false, bCreateReminder, bReadOnly);
    }
    else
    {
      CLog::LogF(LOGERROR, "EPG tag is not recordable");
      return CPVRTimerInfoTagPtr();
    }
  }

  if (!newTimer)
  {
    newTimer.reset(new CPVRTimerInfoTag);

    newTimer->m_iClientIndex       = PVR_TIMER_NO_CLIENT_INDEX;
    newTimer->m_iParentClientIndex = PVR_TIMER_NO_PARENT;
    newTimer->m_channel            = channel;
    newTimer->m_strTitle           = channel->ChannelName();
    newTimer->m_iClientChannelUid  = channel->UniqueID();
    newTimer->m_iClientId          = channel->ClientID();
    newTimer->m_bIsRadio           = channel->IsRadio();

    int iMustHaveAttribs = PVR_TIMER_TYPE_IS_MANUAL;
    if (bCreateReminder)
      iMustHaveAttribs |= PVR_TIMER_TYPE_IS_REMINDER;
    if (bReadOnly)
      iMustHaveAttribs |= PVR_TIMER_TYPE_IS_READONLY;

    // timertype: manual one-shot timer for given client
    const std::shared_ptr<CPVRTimerType> timerType = CPVRTimerType::CreateFromAttributes(
      iMustHaveAttribs, PVR_TIMER_TYPE_IS_REPEATING | PVR_TIMER_TYPE_FORBIDS_NEW_INSTANCES, channel->ClientID());
    if (!timerType)
    {
      CLog::LogF(LOGERROR, "Unable to create one shot manual timer type");
      return CPVRTimerInfoTagPtr();
    }

    newTimer->SetTimerType(timerType);

    if (epgTag)
      newTimer->SetEpgInfoTag(epgTag);
    else
      newTimer->UpdateEpgInfoTag();
  }

  /* no matter the timer was created from an epg tag, overwrite timer start and end times. */
  CDateTime now(CDateTime::GetUTCDateTime());
  if (bInstantStart)
    newTimer->SetStartFromUTC(now);
  else
    newTimer->SetStartFromUTC(start);

  if (iDuration == DEFAULT_PVRRECORD_INSTANTRECORDTIME)
    iDuration = CServiceBroker::GetSettingsComponent()->GetSettings()->GetInt(CSettings::SETTING_PVRRECORD_INSTANTRECORDTIME);

  if (bInstantStart)
  {
    CDateTime endTime = now + CDateTimeSpan(0, 0, iDuration ? iDuration : 120, 0);
    newTimer->SetEndFromUTC(endTime);
  }
  else
  {
    CDateTime endTime = start + CDateTimeSpan(0, 0, iDuration ? iDuration : 120, 0);
    newTimer->SetEndFromUTC(endTime);
  }

  /* update summary string according to instant recording start/end time */
  newTimer->UpdateSummary();

  if (bInstantStart)
  {
    // "Instant recording: <summary>
    newTimer->m_strSummary = StringUtils::Format(g_localizeStrings.Get(19093).c_str(), newTimer->Summary().c_str());

    // now that we have a nice summary, we can set the "special" start time value that indicates an instant recording
    newTimer->SetStartFromUTC(start);
  }

  newTimer->m_iMarginStart = 0;
  newTimer->m_iMarginEnd = 0;

  /* unused only for reference */
  newTimer->m_strFileNameAndPath = CPVRTimersPath::PATH_NEW;

  return newTimer;
}

std::shared_ptr<CPVRTimerInfoTag> CPVRTimerInfoTag::CreateReminderFromEpg(
  const std::shared_ptr<CPVREpgInfoTag>& tag,
  const std::shared_ptr<CPVRTimerInfoTag>& parent /* = std::shared_ptr<CPVRTimerInfoTag>() */)
{
  bool bReadOnly = !!parent; // children of reminder rules are always read-only
  const std::shared_ptr<CPVRTimerInfoTag> newTimer = CreateFromEpg(tag, false, true, bReadOnly);
  if (newTimer && parent)
  {
    // set parent
    newTimer->m_iParentClientIndex = parent->m_iClientIndex;

    // set relevant props for the new one-time reminder timer (everything not epg-related)
    newTimer->m_iMarginStart = parent->m_iMarginStart;
    newTimer->m_iMarginEnd = parent->m_iMarginEnd;
  }
  return newTimer;
}

std::shared_ptr<CPVRTimerInfoTag> CPVRTimerInfoTag::CreateFromEpg(
  const std::shared_ptr<CPVREpgInfoTag>& tag, bool bCreateRule /* = false */)
{
  return CreateFromEpg(tag, bCreateRule, false, false);
}

std::shared_ptr<CPVRTimerInfoTag> CPVRTimerInfoTag::CreateFromEpg(
  const std::shared_ptr<CPVREpgInfoTag>& tag, bool bCreateRule, bool bCreateReminder, bool bReadOnly)
{
  CPVRTimerInfoTagPtr newTag(new CPVRTimerInfoTag());

  /* check if a valid channel is set */
  const std::shared_ptr<CPVRChannel> channel = CServiceBroker::GetPVRManager().ChannelGroups()->GetChannelForEpgTag(tag);
  if (!channel)
  {
    CLog::LogF(LOGERROR, "EPG tag has no channel");
    return CPVRTimerInfoTagPtr();
  }

  newTag->m_iClientIndex       = PVR_TIMER_NO_CLIENT_INDEX;
  newTag->m_iParentClientIndex = PVR_TIMER_NO_PARENT;
  if (!CServiceBroker::GetPVRManager().IsParentalLocked(tag))
    newTag->m_strTitle = tag->Title();
  if (newTag->m_strTitle.empty())
    newTag->m_strTitle = channel->ChannelName();
  newTag->m_iClientChannelUid  = channel->UniqueID();
  newTag->m_iClientId          = channel->ClientID();
  newTag->m_bIsRadio           = channel->IsRadio();
  newTag->m_channel            = channel;
  newTag->m_strSeriesLink      = tag->SeriesLink();
  newTag->m_iEpgUid            = tag->UniqueBroadcastID();
  newTag->SetStartFromUTC(tag->StartAsUTC());
  newTag->SetEndFromUTC(tag->EndAsUTC());

  const int iMustNotHaveAttribs = PVR_TIMER_TYPE_IS_MANUAL |
                                  PVR_TIMER_TYPE_FORBIDS_NEW_INSTANCES |
                                  PVR_TIMER_TYPE_FORBIDS_EPG_TAG_ON_CREATE;
  CPVRTimerTypePtr timerType;
  if (bCreateRule)
  {
    // create epg-based timer rule, prefer rule using series link if available.

    int iMustHaveAttribs = PVR_TIMER_TYPE_IS_REPEATING;
    if (bCreateReminder)
      iMustHaveAttribs |= PVR_TIMER_TYPE_IS_REMINDER;
    if (bReadOnly)
      iMustHaveAttribs |= PVR_TIMER_TYPE_IS_READONLY;

    if (!tag->SeriesLink().empty())
      timerType = CPVRTimerType::CreateFromAttributes(
        iMustHaveAttribs | PVR_TIMER_TYPE_REQUIRES_EPG_SERIESLINK_ON_CREATE, iMustNotHaveAttribs, channel->ClientID());
    if (!timerType)
      timerType = CPVRTimerType::CreateFromAttributes(
        iMustHaveAttribs, iMustNotHaveAttribs | PVR_TIMER_TYPE_REQUIRES_EPG_SERIESLINK_ON_CREATE, channel->ClientID());
    if (timerType)
    {
      if (timerType->SupportsEpgTitleMatch())
        newTag->m_strEpgSearchString = newTag->m_strTitle;

      if (timerType->SupportsWeekdays())
        newTag->m_iWeekdays = PVR_WEEKDAY_ALLDAYS;

      if (timerType->SupportsStartAnyTime())
        newTag->m_bStartAnyTime = true;

      if (timerType->SupportsEndAnyTime())
        newTag->m_bEndAnyTime = true;
    }
  }
  else
  {
    // create one-shot epg-based timer

    int iMustHaveAttribs = PVR_TIMER_TYPE_ATTRIBUTE_NONE;
    if (bCreateReminder)
      iMustHaveAttribs |= PVR_TIMER_TYPE_IS_REMINDER;
    if (bReadOnly)
      iMustHaveAttribs |= PVR_TIMER_TYPE_IS_READONLY;

    timerType = CPVRTimerType::CreateFromAttributes(
      iMustHaveAttribs, PVR_TIMER_TYPE_IS_REPEATING | iMustNotHaveAttribs, channel->ClientID());
  }

  if (!timerType)
  {
    CLog::LogF(LOGERROR, "Unable to create any epg-based timer type");
    return CPVRTimerInfoTagPtr();
  }

  newTag->SetTimerType(timerType);
  newTag->UpdateSummary();
  newTag->SetEpgInfoTag(tag);

  /* unused only for reference */
  newTag->m_strFileNameAndPath = CPVRTimersPath::PATH_NEW;

  return newTag;
}

//! @todo CDateTime class does not handle daylight saving timezone bias correctly (and cannot easily
//  be changed to do so due to performance and platform specific issues). In most cases this only
//  causes GUI presentation glitches, but reminder timer rules rely on correct local time values.

namespace
{
  #define IsLeapYear(y) ((!(y % 4)) ? (((!(y % 400)) && (y % 100)) ? 1 : 0) : 0)

  int days_from_0(int year)
  {
    year--;
    return 365 * year + (year / 400) - (year / 100) + (year / 4);
  }

  int days_from_1970(int32_t year)
  {
    static const int days_from_0_to_1970 = days_from_0(1970);
    return days_from_0(year) - days_from_0_to_1970;
  }

  int days_from_1jan(int year, int month, int day)
  {
    static const int days[2][12] =
    {
      { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 },
      { 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335 }
    };
    return days[IsLeapYear(year)][month - 1] + day - 1;
  }

  time_t mytimegm(struct tm* time)
  {
    int year = time->tm_year + 1900;
    int month = time->tm_mon;

    if (month > 11)
    {
      year += month / 12;
      month %= 12;
    }
    else if (month < 0)
    {
      int years_diff = (-month + 11) / 12;
      year -= years_diff;
      month += 12 * years_diff;
    }

    month++;

    int day_of_year = days_from_1jan(year, month, time->tm_mday);
    int days_since_epoch = days_from_1970(year) + day_of_year;

    return 3600 * 24 * days_since_epoch + 3600 * time->tm_hour + 60 * time->tm_min + time->tm_sec;
  }
}

CDateTime CPVRTimerInfoTag::ConvertUTCToLocalTime(const CDateTime& utc)
{
  time_t time = 0;
  utc.GetAsTime(time);

  struct tm* tms;
#ifdef HAVE_LOCALTIME_R
  struct tm gbuf;
  tms = localtime_r(&time, &gbuf);
#else
  tms = localtime(&time);
#endif

  return CDateTime(mytimegm(tms));
}

CDateTime CPVRTimerInfoTag::ConvertLocalTimeToUTC(const CDateTime& local)
{
  time_t time = 0;
  local.GetAsTime(time);

  struct tm* tms;
#ifdef HAVE_GMTIME_R
  struct tm gbuf;
  tms = gmtime_r(&time, &gbuf);
#else
  tms = gmtime(&time);
#endif

  return CDateTime(mktime(tms));
}

CDateTime CPVRTimerInfoTag::StartAsUTC(void) const
{
  return m_StartTime;
}

CDateTime CPVRTimerInfoTag::StartAsLocalTime(void) const
{
  return ConvertUTCToLocalTime(m_StartTime);
}

void CPVRTimerInfoTag::SetStartFromUTC(const CDateTime& start)
{
  m_StartTime = start;
}

void CPVRTimerInfoTag::SetStartFromLocalTime(const CDateTime& start)
{
  m_StartTime = ConvertLocalTimeToUTC(start);
}

CDateTime CPVRTimerInfoTag::EndAsUTC(void) const
{
  return m_StopTime;
}

CDateTime CPVRTimerInfoTag::EndAsLocalTime(void) const
{
  return ConvertUTCToLocalTime(m_StopTime);
}

void CPVRTimerInfoTag::SetEndFromUTC(const CDateTime& end)
{
  m_StopTime = end;
}

void CPVRTimerInfoTag::SetEndFromLocalTime(const CDateTime& end)
{
  m_StopTime = ConvertLocalTimeToUTC(end);
}

int CPVRTimerInfoTag::GetDuration() const
{
  time_t start, end;
  m_StartTime.GetAsTime(start);
  m_StopTime.GetAsTime(end);
  return end - start > 0 ? end - start : 3600;
}

CDateTime CPVRTimerInfoTag::FirstDayAsUTC(void) const
{
  return m_FirstDay;
}

CDateTime CPVRTimerInfoTag::FirstDayAsLocalTime(void) const
{
  return ConvertUTCToLocalTime(m_FirstDay);
}

void CPVRTimerInfoTag::SetFirstDayFromUTC(const CDateTime& firstDay)
{
  m_FirstDay = firstDay;
}

void CPVRTimerInfoTag::SetFirstDayFromLocalTime(const CDateTime& firstDay)
{
  m_FirstDay = ConvertLocalTimeToUTC(firstDay);
}

void CPVRTimerInfoTag::GetNotificationText(std::string &strText) const
{
  CSingleLock lock(m_critSection);

  int stringID = 0;

  switch (m_state)
  {
  case PVR_TIMER_STATE_ABORTED:
  case PVR_TIMER_STATE_CANCELLED:
      stringID = 19224; // Recording aborted
    break;
  case PVR_TIMER_STATE_SCHEDULED:
    if (IsTimerRule())
      stringID = 19058; // Timer enabled
    else
      stringID = 19225; // Recording scheduled
    break;
  case PVR_TIMER_STATE_RECORDING:
    stringID = 19226; // Recording started
    break;
  case PVR_TIMER_STATE_COMPLETED:
    stringID = 19227; // Recording completed
    break;
  case PVR_TIMER_STATE_CONFLICT_OK:
  case PVR_TIMER_STATE_CONFLICT_NOK:
    stringID = 19277; // Recording conflict
    break;
  case PVR_TIMER_STATE_ERROR:
    stringID = 19278; // Recording error
    break;
  case PVR_TIMER_STATE_DISABLED:
    stringID = 19057; // Timer disabled
    break;
  default:
    break;
  }
  if (stringID != 0)
    strText = StringUtils::Format("%s: '%s'", g_localizeStrings.Get(stringID).c_str(), m_strTitle.c_str());
}

std::string CPVRTimerInfoTag::GetDeletedNotificationText() const
{
  CSingleLock lock(m_critSection);

  int stringID = 0;
  // The state in this case is the state the timer had when it was last seen
  switch (m_state)
  {
  case PVR_TIMER_STATE_RECORDING:
    stringID = 19227; // Recording completed
    break;
  case PVR_TIMER_STATE_SCHEDULED:
  default:
    if (IsTimerRule())
      stringID = 828; // Timer rule deleted
    else
      stringID = 19228; // Timer deleted
  }

  return StringUtils::Format("%s: '%s'", g_localizeStrings.Get(stringID).c_str(), m_strTitle.c_str());
}

void CPVRTimerInfoTag::SetEpgInfoTag(const std::shared_ptr<CPVREpgInfoTag>& tag)
{
  CSingleLock lock(m_critSection);
  m_epgTag = tag;
}

void CPVRTimerInfoTag::UpdateEpgInfoTag()
{
  CSingleLock lock(m_critSection);
  m_epgTag.reset();
  GetEpgInfoTag();
}

CPVREpgInfoTagPtr CPVRTimerInfoTag::GetEpgInfoTag(bool bCreate /* = true */) const
{
  if (!m_epgTag && bCreate && CServiceBroker::GetPVRManager().EpgsCreated())
  {
    CPVRChannelPtr channel(m_channel);
    if (!channel)
    {
      channel = CServiceBroker::GetPVRManager().ChannelGroups()->Get(m_bIsRadio)->GetGroupAll()->GetByUniqueID(m_iClientChannelUid, m_iClientId);

      CSingleLock lock(m_critSection);
      m_channel = channel;
    }

    if (channel)
    {
      const CPVREpgPtr epg(channel->GetEPG());
      if (epg)
      {
        CSingleLock lock(m_critSection);
        if (!m_epgTag)
        {
          if (m_iEpgUid != EPG_TAG_INVALID_UID)
          {
            m_epgTag = epg->GetTagByBroadcastId(m_iEpgUid);
          }
        }

        if (!IsTimerRule() && !m_epgTag && m_epTagRefetchTimeout.IsTimePast() && IsOwnedByClient())
        {
          m_epTagRefetchTimeout.Set(30000); // try to fetch missing epg tag from backend at most every 30 secs

          time_t startTime = 0;
          time_t endTime = 0;

          StartAsUTC().GetAsTime(startTime);
          if (startTime > 0)
            EndAsUTC().GetAsTime(endTime);

          if (startTime > 0 && endTime > 0)
          {
            // try to fetch missing epg tag from backend
            m_epgTag = epg->GetTagBetween(StartAsUTC() - CDateTimeSpan(0, 0, 2, 0), EndAsUTC() + CDateTimeSpan(0, 0, 2, 0), true);
            if (m_epgTag)
              m_iEpgUid = m_epgTag->UniqueBroadcastID();
          }
        }
      }
    }
  }

  if (!m_epgTag && HasFallbackEpgData() && !IsTimerRule())
  {
    CPVRChannelPtr channel(m_channel);
    if (!channel)
    {
      channel = CServiceBroker::GetPVRManager().ChannelGroups()->Get(m_bIsRadio)->GetGroupAll()->GetByUniqueID(m_iClientChannelUid, m_iClientId);

      CSingleLock lock(m_critSection);
      m_channel = channel;
    }

    if (channel)
    {    
      m_epgTag = CreateFallbackEpgInfoTag();
    }
  }  
  return m_epgTag;
}

bool CPVRTimerInfoTag::HasFallbackEpgData() const
{
  return !m_strPlotOutline.empty() || !m_strPlot.empty();
}

CPVREpgInfoTagPtr CPVRTimerInfoTag::CreateFallbackEpgInfoTag() const
{
  EPG_TAG tag = {0};
  tag.strPlotOutline = m_strPlot.c_str();
  tag.strPlot = m_strPlotOutline.c_str();
  tag.strTitle = m_strTitle.c_str();
  tag.iUniqueChannelId = m_iClientChannelUid;
  tag.iUniqueBroadcastId = m_iEpgUid;
  tag.iYear = m_iYear;
  tag.iGenreType = m_iGenreType;
  tag.iGenreSubType = m_iGenreSubType;
  tag.strGenreDescription = m_strGenreDescription.c_str();
  tag.iSeriesNumber = m_iSeriesNumber;
  tag.iEpisodeNumber = m_iEpisodeNumber;
  tag.iEpisodePartNumber = m_iEpisodePartNumber;

  time_t startTime;
  m_StartTime.GetAsTime(startTime);
  tag.startTime = startTime;

  time_t endTime;
  m_StopTime.GetAsTime(endTime);
  tag.endTime = endTime;

  return std::make_shared<CPVREpgInfoTag>(tag, m_iClientId, nullptr, m_iEpgUid);
}

bool CPVRTimerInfoTag::HasChannel() const
{
  return m_channel.get() != nullptr;
}

CPVRChannelPtr CPVRTimerInfoTag::Channel() const
{
  return m_channel;
}

CPVRChannelPtr CPVRTimerInfoTag::UpdateChannel(void)
{
  const CPVRChannelPtr channel(CServiceBroker::GetPVRManager().ChannelGroups()->Get(m_bIsRadio)->GetGroupAll()->GetByUniqueID(m_iClientChannelUid, m_iClientId));

  CSingleLock lock(m_critSection);
  m_channel = channel;
  return m_channel;
}

const std::string& CPVRTimerInfoTag::Title(void) const
{
  return m_strTitle;
}

const std::string& CPVRTimerInfoTag::Summary(void) const
{
  return m_strSummary;
}

const std::string& CPVRTimerInfoTag::Path(void) const
{
  return m_strFileNameAndPath;
}

const std::string& CPVRTimerInfoTag::SeriesLink() const
{
  return m_strSeriesLink;
}
