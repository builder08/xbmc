/*
 *      Copyright (C) 2005-2011 Team XBMC
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

#include "settings/GUISettings.h"
#include "guilib/GUIWindowManager.h"
#include "dialogs/GUIDialogYesNo.h"
#include "dialogs/GUIDialogOK.h"
#include "utils/log.h"

#include "PVRChannelGroupsContainer.h"
#include "pvr/PVRDatabase.h"
#include "pvr/PVRManager.h"
#include "pvr/timers/PVRTimers.h"
#include "pvr/addons/PVRClients.h"

CPVRChannelGroupInternal::CPVRChannelGroupInternal(bool bRadio) :
  CPVRChannelGroup(bRadio)
{
  m_iHiddenChannels = 0;
  m_iGroupId        = bRadio ? XBMC_INTERNAL_GROUP_RADIO : XBMC_INTERNAL_GROUP_TV;
  m_strGroupName    = g_localizeStrings.Get(bRadio ? 19216 : 19217);
  m_iSortOrder      = 0;
}

int CPVRChannelGroupInternal::Load(void)
{
  int iChannelCount = CPVRChannelGroup::Load();
  UpdateChannelPaths();
  CacheIcons();

  return iChannelCount;
}

void CPVRChannelGroupInternal::UpdateChannelPaths(void)
{
  for (unsigned int iChannelPtr = 0; iChannelPtr < size(); iChannelPtr++)
  {
    PVRChannelGroupMember member = at(iChannelPtr);
    member.channel->UpdatePath(member.iChannelNumber);
  }
}

void CPVRChannelGroupInternal::Unload()
{
  for (unsigned int iChannelPtr = 0; iChannelPtr < size(); iChannelPtr++)
  {
    delete at(iChannelPtr).channel;
  }

  CPVRChannelGroup::Unload();
}

bool CPVRChannelGroupInternal::UpdateFromClient(const CPVRChannel &channel)
{
  CSingleLock lock(m_critSection);
  CPVRChannel *realChannel = (CPVRChannel *) GetByClient(channel.UniqueID(), channel.ClientID());
  if (realChannel != NULL)
    realChannel->UpdateFromClient(channel);
  else
    realChannel = new CPVRChannel(channel);

  return CPVRChannelGroup::AddToGroup(realChannel, 0, false);
}

bool CPVRChannelGroupInternal::InsertInGroup(CPVRChannel *channel, int iChannelNumber /* = 0 */, bool bSortAndRenumber /* = true */)
{
  CSingleLock lock(m_critSection);
  return CPVRChannelGroup::AddToGroup(channel, iChannelNumber, bSortAndRenumber);
}

bool CPVRChannelGroupInternal::Update(void)
{
  CPVRChannelGroupInternal PVRChannels_tmp(m_bRadio);
  PVRChannels_tmp.LoadFromClients();

  return UpdateGroupEntries(PVRChannels_tmp);
}

bool CPVRChannelGroupInternal::UpdateTimers(void)
{
  CSingleLock lock(m_critSection);

  /* update the timers with the new channel numbers */
  CPVRTimers *timers = CPVRManager::GetTimers();
  for (unsigned int ptr = 0; ptr < timers->size(); ptr++)
  {
    CPVRTimerInfoTag *timer = timers->at(ptr);
    const CPVRChannel *tag = GetByClient(timer->m_iClientChannelUid, timer->m_iClientId);
    if (tag)
      timer->m_channel = tag;
  }

  return true;
}

bool CPVRChannelGroupInternal::AddToGroup(CPVRChannel *channel, int iChannelNumber /* = 0 */)
{
  CSingleLock lock(m_critSection);

  /* get the actual channel since this is called from a fileitemlist copy */
  CPVRChannel *realChannel = (CPVRChannel *) GetByChannelID(channel->ChannelID());
  if (!realChannel)
    return false;

  /* switch the hidden flag */
  if (realChannel->IsHidden())
  {
    realChannel->SetHidden(false, true);
    m_iHiddenChannels--;
  }

  /* renumber this list */
  Renumber();

  /* move this channel and persist */
  if (iChannelNumber > 0)
    return MoveChannel(realChannel->ChannelNumber(), iChannelNumber, true);
  else
    return MoveChannel(realChannel->ChannelNumber(), size() - m_iHiddenChannels, true);
}

bool CPVRChannelGroupInternal::RemoveFromGroup(CPVRChannel *channel)
{
  if (!channel)
    return false;

  CSingleLock lock(m_critSection);

  /* check if this channel is currently playing if we are hiding it */
  CPVRChannel currentChannel;
  if (CPVRManager::Get()->GetCurrentChannel(&currentChannel) && currentChannel == *channel)
  {
    CGUIDialogOK::ShowAndGetInput(19098,19101,0,19102);
    return false;
  }

  /* get the actual channel since this is called from a fileitemlist copy */
  CPVRChannel *realChannel = (CPVRChannel *) GetByChannelID(channel->ChannelID());
  if (!realChannel)
    return false;

  /* switch the hidden flag */
  realChannel->SetHidden(true, true);
  ++m_iHiddenChannels;

  /* renumber this list */
  Renumber();

  /* and persist */
  return Persist();
}

bool CPVRChannelGroupInternal::MoveChannel(unsigned int iOldChannelNumber, unsigned int iNewChannelNumber, bool bSaveInDb /* = true */)
{
  CSingleLock lock(m_critSection);
  /* new channel number out of range */
  if (iNewChannelNumber > size() - m_iHiddenChannels)
    iNewChannelNumber = size() - m_iHiddenChannels;

  return CPVRChannelGroup::MoveChannel(iOldChannelNumber, iNewChannelNumber, bSaveInDb);
}

int CPVRChannelGroupInternal::GetMembers(CFileItemList *results, bool bGroupMembers /* = true */) const
{
  int iOrigSize = results->Size();
  CSingleLock lock(m_critSection);

  for (unsigned int iChannelPtr = 0; iChannelPtr < size(); iChannelPtr++)
  {
    CPVRChannel *channel = at(iChannelPtr).channel;
    if (!channel)
      continue;

    if (bGroupMembers != channel->IsHidden())
    {
      CFileItemPtr pFileItem(new CFileItem(*channel));
      results->Add(pFileItem);
    }
  }

  return results->Size() - iOrigSize;
}

int CPVRChannelGroupInternal::LoadFromDb(bool bCompress /* = false */)
{
  CPVRDatabase *database = CPVRManager::Get()->GetTVDatabase();
  if (!database || !database->Open())
    return -1;

  int iChannelCount = size();

  if (database->GetChannels(this, m_bRadio) > 0)
  {
    if (bCompress)
      database->Compress(true);
  }
  else
  {
    CLog::Log(LOGINFO, "PVRChannelGroupInternal - %s - no channels in the database",
        __FUNCTION__);
  }

  database->Close();

  SortByChannelNumber();

  return size() - iChannelCount;
}

int CPVRChannelGroupInternal::LoadFromClients(void)
{
  int iCurSize = size();

  /* get the channels from the backends */
  PVR_ERROR error;
  CPVRManager::GetClients()->GetChannels(this, &error);
  if (error != PVR_ERROR_NO_ERROR)
    CLog::Log(LOGWARNING, "CPVRChannelGroupInternal - %s - got bad error (%d) on call to GetChannels", __FUNCTION__, error);

  return size() - iCurSize;
}

void CPVRChannelGroupInternal::Renumber(void)
{
  int iChannelNumber = 0;
  m_iHiddenChannels = 0;
  for (unsigned int ptr = 0; ptr < size();  ptr++)
  {
    if (at(ptr).channel->IsHidden())
    {
      at(ptr).iChannelNumber = 0;
      m_iHiddenChannels++;
    }
    else
    {
      at(ptr).iChannelNumber = ++iChannelNumber;
      at(ptr).channel->UpdatePath(iChannelNumber);
    }
  }
}

bool CPVRChannelGroupInternal::IsGroupMember(const CPVRChannel &channel) const
{
  return !channel.IsHidden();
}

bool CPVRChannelGroupInternal::UpdateChannel(const CPVRChannel &channel)
{
  CSingleLock lock(m_critSection);
  CPVRChannel *updateChannel = (CPVRChannel *) GetByUniqueID(channel.UniqueID());

  if (!updateChannel)
  {
    updateChannel = new CPVRChannel(channel.IsRadio());
    PVRChannelGroupMember newMember = { updateChannel, 0 };
    push_back(newMember);
    updateChannel->SetUniqueID(channel.UniqueID());
  }
  updateChannel->UpdateFromClient(channel);

  return updateChannel->Persist(!m_bLoaded);
}

bool CPVRChannelGroupInternal::UpdateGroupEntries(const CPVRChannelGroup &channels)
{
  bool bChanged = false;
  int iCurSize = size();

  CPVRChannelGroup *newChannels = new CPVRChannelGroup(m_bRadio);

  CPVRDatabase *database = CPVRManager::Get()->GetTVDatabase();
  if (!database || !database->Open())
    return bChanged;

  /* go through the channel list and check for updated or new channels */
  for (unsigned int iChannelPtr = 0; iChannelPtr < channels.size(); iChannelPtr++)
  {
    const CPVRChannel *channel = channels.at(iChannelPtr).channel;

    /* check if this channel is present in this container */
    CPVRChannel *existingChannel = (CPVRChannel *) GetByClient(channel->UniqueID(), channel->ClientID());
    if (existingChannel)
    {
      /* if it's present, update the current tag */
      if (existingChannel->UpdateFromClient(*channel))
      {
        bChanged = true;

        CLog::Log(LOGINFO,"PVRChannelGroupInternal - %s - updated %s channel '%s'",
            __FUNCTION__, m_bRadio ? "radio" : "TV", channel->ChannelName().c_str());
      }
    }
    else
    {
      /* new channel */
      CPVRChannel *newChannel = new CPVRChannel(m_bRadio);
      newChannel->SetUniqueID(channel->UniqueID(), false);
      newChannel->UpdateFromClient(*channel);
      newChannels->AddToGroup(newChannel);
      int iChannelNumber = iCurSize == 0 ? channel->ClientChannelNumber() : 0;
      InsertInGroup(newChannel, iChannelNumber, false);
      bChanged = true;

      CLog::Log(LOGINFO,"PVRChannelGroupInternal - %s - added %s channel '%s' at position %d",
          __FUNCTION__, m_bRadio ? "radio" : "TV", channel->ChannelName().c_str(), iChannelNumber);
    }
  }

  /* persist changes */
  for (unsigned int iChannelPtr = 0; iChannelPtr < newChannels->size(); iChannelPtr++)
    ((CPVRChannel *) newChannels->GetByIndex(iChannelPtr))->Persist(false); /* write immediately to get a db id */
  delete newChannels;

  /* check for deleted channels */
  unsigned int iSize = size();
  for (unsigned int iChannelPtr = 0; iChannelPtr < iSize; iChannelPtr++)
  {
    CPVRChannel *channel = (CPVRChannel *) GetByIndex(iChannelPtr);
    if (!channel)
      continue;
    if (channels.GetByClient(channel->UniqueID(), channel->ClientID()) == NULL)
    {
      /* channel was not found */
      CLog::Log(LOGINFO,"PVRChannelGroupInternal - %s - deleted %s channel '%s'",
          __FUNCTION__, m_bRadio ? "radio" : "TV", channel->ChannelName().c_str());

      /* remove this channel from all non-system groups */
      ((CPVRChannelGroups *) CPVRManager::GetChannelGroups()->Get(m_bRadio))->RemoveFromAllGroups(channel);

      delete at(iChannelPtr).channel;
      erase(begin() + iChannelPtr);
      iChannelPtr--;
      iSize--;
      bChanged = true;
    }
  }

  database->Close();

  /* try to find channel icons */
  SearchAndSetChannelIcons();
  CacheIcons();

  if (bChanged || HasChanges())
  {
    /* remove invalid channels */
    RemoveInvalidChannels();

    /* sort by client channel number if this is the first time */
    if (iCurSize == 0)
      SortByClientChannelNumber();

    /* renumber to make sure all channels have a channel number.
       new channels were added at the back, so they'll get the highest numbers */
    Renumber();

    return Persist();
  }
  else
  {
    return true;
  }
}

bool CPVRChannelGroupInternal::Persist(void)
{
  bool bReturn(false);
  CSingleLock lock(m_critSection);

  CLog::Log(LOGDEBUG, "CPVRChannelGroupInternal - %s - persisting %d channels",
      __FUNCTION__, (int) size());

  if (HasNewChannels())
  {
    CLog::Log(LOGDEBUG, "CPVRChannelGroupInternal - %s - group '%s' has new channels. writing changes directly",
        __FUNCTION__, GroupName().c_str());
    /* write directly to get channel ids */
    bReturn = true;
    for (unsigned int iChannelPtr = 0; iChannelPtr < size(); iChannelPtr++)
    {
      if (!at(iChannelPtr).channel->Persist())
      {
        CLog::Log(LOGERROR, "CPVRChannelGroupInternal - %s - failed to persist channel '%s'",
            __FUNCTION__, at(iChannelPtr).channel->ChannelName().c_str());
        bReturn = false;
      }
    }
  }
  else if (HasChangedChannels())
  {
    /* open the database */
    CPVRDatabase *database = CPVRManager::Get()->GetTVDatabase();
    if (!database || !database->Open())
    {
      CLog::Log(LOGERROR, "CPVRChannelGroupInternal - %s - failed to open the database", __FUNCTION__);
      return false;
    }

    /* queue queries */
    for (unsigned int iChannelPtr = 0; iChannelPtr < size(); iChannelPtr++)
      at(iChannelPtr).channel->Persist(true);

    /* and commit them */
    bReturn = database->CommitInsertQueries();
    if (!bReturn)
      CLog::Log(LOGERROR, "CPVRChannelGroupInternal - %s - failed to persist channels", __FUNCTION__);

    database->Close();
  }

  if (bReturn)
    bReturn = CPVRChannelGroup::Persist();

  return bReturn;
}
