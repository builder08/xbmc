#pragma once

/*
 *      Copyright (C) 2005-2013 Team XBMC
 *      http://xbmc.org
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
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include <string>
#ifndef DIALOGS_GUILIB_GUIDIALOG_H_INCLUDED
#define DIALOGS_GUILIB_GUIDIALOG_H_INCLUDED
#include "guilib/GUIDialog.h"
#endif

#ifndef DIALOGS_THREADS_CRITICALSECTION_H_INCLUDED
#define DIALOGS_THREADS_CRITICALSECTION_H_INCLUDED
#include "threads/CriticalSection.h"
#endif

#ifndef DIALOGS_UTILS_JOBMANAGER_H_INCLUDED
#define DIALOGS_UTILS_JOBMANAGER_H_INCLUDED
#include "utils/JobManager.h"
#endif


enum SUBTITLE_STORAGEMODE
{
  SUBTITLE_STORAGEMODE_MOVIEPATH = 0,
  SUBTITLE_STORAGEMODE_CUSTOMPATH
};

class CFileItem;
class CFileItemList;

class CGUIDialogSubtitles : public CGUIDialog, CJobQueue
{
public:
  CGUIDialogSubtitles(void);
  virtual ~CGUIDialogSubtitles(void);
  virtual bool OnMessage(CGUIMessage& message);
  virtual void OnInitWindow();

protected:
  virtual void Process(unsigned int currentTime, CDirtyRegionList &dirtyregions);
  virtual void OnJobComplete(unsigned int jobID, bool success, CJob *job);

  bool SetService(const std::string &service);
  const CFileItemPtr GetService() const;
  void FillServices();
  void ClearServices();
  void ClearSubtitles();

  enum STATUS { NO_SERVICES = 0, SEARCHING, SEARCH_COMPLETE, DOWNLOADING };
  void UpdateStatus(STATUS status);

  void Search(const std::string &search="");
  void OnSearchComplete(const CFileItemList *items);

  void Download(const CFileItem &subtitle);
  void OnDownloadComplete(const CFileItemList *items, const std::string &language);

  void SetSubtitles(const std::string &subtitle);

  CCriticalSection m_critsection;
  CFileItemList* m_subtitles;
  CFileItemList* m_serviceItems;
  std::string    m_currentService;
  std::string    m_status;
  CStdString     m_strManualSearch;
  bool           m_pausedOnRun;
  bool           m_updateSubsList; ///< true if we need to update our subs list
};
