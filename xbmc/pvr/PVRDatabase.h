#pragma once
/*
 *      Copyright (C) 2005-2010 Team XBMC
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

#include "dbwrappers/Database.h"
#include "DateTime.h"

class CPVRChannelGroup;
class CPVRChannelGroupInternal;
class CPVRChannelsContainer;
class CPVRChannel;
class CPVRChannelGroups;
class CVideoSettings;

/** The PVR database */

class CPVRDatabase : public CDatabase
{
public:
  /*!
   * @brief Create a new instance of the PVR database.
   */
  CPVRDatabase(void);
  virtual ~CPVRDatabase(void);

  /*!
   * @brief Open the database.
   * @return True if it was opened successfully, false otherwise.
   */
  virtual bool Open();

  /*!
   * @brief Get the minimal database version that is required to operate correctly.
   * @return The minimal database version.
   */
  virtual int GetMinVersion() const { return 12; };

  /*!
   * @brief Get the default sqlite database filename.
   * @return The default filename.
   */
  const char *GetBaseDBName() const { return "TV"; };

  /*! @name Channel methods */
  //@{

  /*!
   * @brief Remove all channels from the database.
   * @return True if all channels were removed, false otherwise.
   */
  bool DeleteChannels();

  /*!
   * @brief Remove all channels from a client from the database.
   * @param iClientId The ID of the client to delete the channels for.
   * @return True if the channels were deleted, false otherwise.
   */
  bool DeleteClientChannels(int iClientId);

  /*!
   * @brief Add or update a channel entry in the database
   * @param channel The channel to persist.
   * @param bQueueWrite If true, don't write immediately
   * @return The database ID of the channel.
   */
  int Persist(const CPVRChannel &channel, bool bQueueWrite = false);

  /*!
   * @brief Remove a channel entry from the database
   * @param channel The channel to remove.
   * @return True if the channel was removed, false otherwise.
   */
  bool Delete(const CPVRChannel &channel);

  /*!
   * @brief Get the list of channels from the database
   * @param results The channel group to store the results in.
   * @param bIsRadio Get the radio channels if true. Get the TV channels otherwise.
   * @return The amount of channels that were added.
   */
  int GetChannels(CPVRChannelGroupInternal *results, bool bIsRadio);

  //@}

  /*! @name Channel settings methods */
  //@{

  /*!
   * @brief Remove all channel settings from the database.
   * @return True if all channels were removed successfully, false if not.
   */
  bool DeleteChannelSettings();

  /*!
   * @brief Remove channel settings from the database.
   * @return True if channel were removed successfully, false if not.
   */
  bool DeleteChannelSettings(const CPVRChannel &channel);

  /*!
   * @brief Get the channel settings from the database.
   * @param channel The channel to get the settings for.
   * @param settings Store the settings in here.
   * @return True if the settings were fetched successfully, false if not.
   */
  bool GetChannelSettings(const CPVRChannel &channel, CVideoSettings &settings);

  /*!
   * @brief Store channel settings in the database.
   * @param channel The channel to store the settings for.
   * @param settings The settings to store.
   * @return True if the settings were stored successfully, false if not.
   */
  bool PersistChannelSettings(const CPVRChannel &channel, const CVideoSettings &settings);

  //@}

  /*! @name Channel group methods */
  //@{

  bool RemoveChannelsFromGroup(int iGroupId);

  /*!
   * @brief Remove all channel groups from the database
   * @param bRadio Remove all radio channel groups if true. Remove TV channel groups otherwise.
   * @return True if all channel groups were removed.
   */
  bool DeleteChannelGroups(bool bRadio = false);

  /*!
   * @brief Delete a channel group from the database.
   * @param iGroupId The id of the group to delete.
   * @param bRadio True if it's a radio channel group, false otherwise.
   * @return True if the group was deleted successfully, false otherwise.
   */
  bool DeleteChannelGroup(int iGroupId, bool bRadio = false);

  /*!
   * @brief Get the channel groups.
   * @param results The container to store the results in.
   * @param bRadio Get radio channel groups if true.
   * @return True if the list was fetched successfully, false otherwise.
   */
  bool GetChannelGroupList(CPVRChannelGroups &results, bool bRadio);

  /*!
   * @brief Add the group members to a group.
   * @param group The group to get the channels for.
   * @return The amount of channels that were added.
   */
  int GetChannelsInGroup(CPVRChannelGroup *group);

  /*!
   * @brief Add or update a channel group entry in the database.
   * @param group The group to persist.
   * @return The database ID of the group.
   */
  int Persist(CPVRChannelGroup *group);

protected:
  /*!
   * @brief Get the Id of a channel group
   * @param strGroupName The name of the group.
   * @param bRadio True if it's a radio channel group, false otherwise.
   * @return
   */
  int GetChannelGroupId(const CStdString &strGroupName, bool bRadio = false);
  //@}

  /*! @name Client methods */
  //@{
public:
  /*!
   * @brief Remove all client information from the database.
   * @return True if all clients were removed successfully.
   */
  bool DeleteClients();

  /*!
   * @brief Add a client to the database if it's not already in there.
   * @param strClientName The name of the client.
   * @param strGuid The unique ID of the client.
   * @return The database ID of the client.
   */
  int AddClient(const CStdString &strClientName, const CStdString &strGuid);

  /*!
   * @brief Remove a client from the database
   * @param strGuid The unique ID of the client.
   * @return True if the client was removed successfully, false otherwise.
   */
  bool DeleteClient(const CStdString &strGuid);

protected:
  /*!
   * @brief Get the database ID of a client.
   * @param strClientUid The unique ID of the client.
   * @return The database ID of the client or -1 if it wasn't found.
   */
  int GetClientId(const CStdString &strClientUid);

  //@}

private:
  /*!
   * @brief Create the PVR database tables.
   * @return True if the tables were created successfully, false otherwise.
   */
  virtual bool CreateTables();

  /*!
   * @brief Update an old version of the database.
   * @param version The version to update the database from.
   * @return True if it was updated successfully, false otherwise.
   */
  virtual bool UpdateOldVersion(int version);
};
