/*
 *      Copyright (C) 2005-2008 Team XBMC
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

#include "FileItem.h"
#include "settings/GUISettings.h"
#include "guilib/GUIWindowManager.h"
#include "dialogs/GUIDialogYesNo.h"
#include "dialogs/GUIDialogOK.h"
#include "guilib/LocalizeStrings.h"
#include "utils/log.h"
#include "Util.h"
#include "URL.h"
#include "filesystem/File.h"
#include "music/tags/MusicInfoTag.h"

#include "PVRChannelGroupInternal.h"
#include "PVRChannelGroupsContainer.h"
#include "pvr/PVRDatabase.h"
#include "pvr/PVRManager.h"

using namespace XFILE;
using namespace MUSIC_INFO;

CPVRChannelGroups::CPVRChannelGroups(bool bRadio)
{
  m_bRadio = bRadio;
}

CPVRChannelGroups::~CPVRChannelGroups(void)
{
  Clear();
}

void CPVRChannelGroups::Clear(void)
{
  for (unsigned int iGroupPtr = 0; iGroupPtr < size(); iGroupPtr++)
    delete at(iGroupPtr);

  clear();
}

bool CPVRChannelGroups::Update(const CPVRChannelGroup &group)
{
  int iIndex = GetIndexForGroupID(group.GroupID());
  CPVRChannelGroup *updateGroup = NULL;

  if (iIndex < 0)
  {
    CLog::Log(LOGDEBUG, "PVRChannelGroups - %s - new %s channel group '%s'",
        __FUNCTION__, m_bRadio ? "radio" : "TV", group.GroupName().c_str());

    updateGroup = new CPVRChannelGroup(m_bRadio);
    push_back(updateGroup);
  }
  else
  {
    CLog::Log(LOGDEBUG, "PVRChannelGroups - %s - updating %s channel group '%s'",
        __FUNCTION__, m_bRadio ? "radio" : "TV", group.GroupName().c_str());

    updateGroup = at(iIndex);
  }

  updateGroup->SetGroupName(group.GroupName());
  updateGroup->SetSortOrder(group.SortOrder());

  return true;
}

const CPVRChannelGroup *CPVRChannelGroups::GetById(int iGroupId) const
{
  const CPVRChannelGroup *group = NULL;

  if (iGroupId == (m_bRadio ? XBMC_INTERNAL_GROUP_RADIO : XBMC_INTERNAL_GROUP_TV))
  {
    group = GetGroupAll();
  }
  else if (iGroupId > -1)
  {
    int iGroupIndex = GetIndexForGroupID(iGroupId);
    if (iGroupIndex != -1)
      group = at(iGroupIndex);
  }

  return group;
}

const CPVRChannelGroup *CPVRChannelGroups::GetByName(const CStdString &strName) const
{
  CPVRChannelGroup *group = NULL;

  for (unsigned int iGroupPtr = 0; iGroupPtr < size(); iGroupPtr++)
  {
    if (at(iGroupPtr)->GroupName() == strName)
    {
      group = at(iGroupPtr);
      break;
    }
  }

  return group;
}

int CPVRChannelGroups::GetIndexForGroupID(int iGroupId) const
{
  int iReturn = -1;

  for (unsigned int iGroupPtr = 0; iGroupPtr < size(); iGroupPtr++)
  {
    if (at(iGroupPtr)->GroupID() == iGroupId)
    {
      iReturn = iGroupPtr;
      break;
    }
  }

  return iReturn;
}

bool CPVRChannelGroups::Update(void)
{
  bool bReturn = true;

  for (unsigned int iGroupPtr = 0; iGroupPtr < size(); iGroupPtr++)
    bReturn = at(iGroupPtr)->Update() && bReturn;

  return bReturn;
}

bool CPVRChannelGroups::Load(void)
{
  CLog::Log(LOGDEBUG, "PVRChannelGroups - %s - loading all %s channel groups",
      __FUNCTION__, m_bRadio ? "radio" : "TV");

  Clear();

  /* create internal channel group */
  CPVRChannelGroupInternal *internalChannels = new CPVRChannelGroupInternal(m_bRadio);
  push_back(internalChannels);
  internalChannels->Load();

  /* load the other groups from the database */
  CPVRDatabase *database = CPVRManager::Get()->GetTVDatabase();
  database->Open();

  database->GetChannelGroupList(*this, m_bRadio);
  database->Close();

  CLog::Log(LOGDEBUG, "PVRChannelGroups - %s - %d %s channel groups loaded",
      __FUNCTION__, size(), m_bRadio ? "radio" : "TV");

  return true;
}

const CPVRChannelGroup *CPVRChannelGroups::GetGroupAll(void) const
{
  if (size() > 0)
    return at(0);
  else
    return NULL;
}

int CPVRChannelGroups::GetGroupList(CFileItemList* results) const
{
  int iReturn = 0;

  for (unsigned int iGroupPtr = 0; iGroupPtr < size(); iGroupPtr++)
  {
    CFileItemPtr group(new CFileItem(at(iGroupPtr)->GroupName()));
    group->m_strTitle = at(iGroupPtr)->GroupName();
    group->m_strPath.Format("%i", at(iGroupPtr)->GroupID());
    results->Add(group);
    ++iReturn;
  }

  return iReturn;
}

int CPVRChannelGroups::GetFirstChannelForGroupID(int iGroupId) const
{
  int iReturn = 1;

  const CPVRChannelGroup *group = GetById(iGroupId);

  if (group)
    iReturn = group->GetFirstChannel()->ChannelID();

  return iReturn;
}

int CPVRChannelGroups::GetPreviousGroupID(int iGroupId) const
{
  int iReturn = m_bRadio ? XBMC_INTERNAL_GROUP_RADIO : XBMC_INTERNAL_GROUP_TV;

  int iCurrentGroupIndex = GetIndexForGroupID(iGroupId);
  if (iCurrentGroupIndex != -1)
  {
    int iGroupIndex = iCurrentGroupIndex - 1;
    if (iGroupIndex < 0) iGroupIndex = size() - 1;

    iReturn = at(iGroupIndex)->GroupID();
  }

  return iReturn;
}

const CPVRChannelGroup *CPVRChannelGroups::GetPreviousGroup(const CPVRChannelGroup &group) const
{
  const CPVRChannelGroup *returnGroup = NULL;

  int iCurrentGroupIndex = GetIndexForGroupID(group.GroupID());
  if (iCurrentGroupIndex != -1)
  {
    int iGroupIndex = iCurrentGroupIndex - 1;
    if (iGroupIndex < 0) iGroupIndex = size() - 1;

    returnGroup = at(iGroupIndex);
  }

  if (!returnGroup)
    returnGroup = GetGroupAll();

  return returnGroup;
}

int CPVRChannelGroups::GetNextGroupID(int iGroupId) const
{
  int iReturn = m_bRadio ? XBMC_INTERNAL_GROUP_RADIO : XBMC_INTERNAL_GROUP_TV;

  int iCurrentGroupIndex = GetIndexForGroupID(iGroupId);
  if (iCurrentGroupIndex != -1)
  {
    int iGroupIndex = iCurrentGroupIndex + 1;
    if (iGroupIndex == (int) size()) iGroupIndex = 0;

    iReturn = at(iGroupIndex)->GroupID();
  }

  return iReturn;
}

const CPVRChannelGroup *CPVRChannelGroups::GetNextGroup(const CPVRChannelGroup &group) const
{
  const CPVRChannelGroup *returnGroup = NULL;

  int iCurrentGroupIndex = GetIndexForGroupID(group.GroupID());
  if (iCurrentGroupIndex != -1)
  {
    int iGroupIndex = iCurrentGroupIndex + 1;
    if (iGroupIndex == (int) size()) iGroupIndex = 0;

    returnGroup = at(iGroupIndex);
  }

  if (!returnGroup)
    returnGroup = GetGroupAll();

  return returnGroup;
}

bool CPVRChannelGroups::AddGroup(const CStdString &strName)
{
  bool bReturn = false;

  CPVRChannelGroup *group = (CPVRChannelGroup *) GetByName(strName);
  if (!group)
  {
    group = new CPVRChannelGroup(m_bRadio);
    group->SetGroupName(strName);
    push_back(group);

    bReturn = group->Persist();
  }
  else
  {
    bReturn = true;
  }

  return bReturn;
}

bool CPVRChannelGroups::DeleteGroup(const CPVRChannelGroup &group)
{
  bool bReturn = false;

  CPVRDatabase *database = CPVRManager::Get()->GetTVDatabase();
  if (!database->Open())
    return bReturn;

  /* delete the group from the database */
  bReturn = database->DeleteChannelGroup(group.GroupID(), m_bRadio);

  database->Close();

  /* delete the group */
  for (unsigned int iGroupPtr = 0; iGroupPtr < size(); iGroupPtr++)
  {
    if (at(iGroupPtr)->GroupID() == group.GroupID())
    {
      delete at(iGroupPtr);
      break;
    }
  }

  return bReturn;
}

const CStdString &CPVRChannelGroups::GetGroupName(int iGroupId) const
{
  for (unsigned int iGroupPtr = 0; iGroupPtr < size(); iGroupPtr++)
  {
    if (iGroupId == at(iGroupPtr)->GroupID())
      return at(iGroupPtr)->GroupName();
  }

  return g_localizeStrings.Get(13205);
}

int CPVRChannelGroups::GetGroupId(CStdString strGroupName) const
{
  for (unsigned int iGroupPtr = 0; iGroupPtr < size(); iGroupPtr++)
  {
    if (strGroupName == at(iGroupPtr)->GroupName())
      return at(iGroupPtr)->GroupID();
  }
  return -1;
}

bool CPVRChannelGroups::AddChannelToGroup(CPVRChannel *channel, int iGroupId)
{
  bool bReturn = false;
  CPVRChannelGroup *group = (CPVRChannelGroup *) GetById(iGroupId);
  if (group)
  {
    bReturn = group->AddToGroup(channel);
  }

  return bReturn;
}
