/*
 *      Copyright (C) 2013 Team XBMC
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

#pragma once

#include <string>
#ifndef LISTPROVIDERS_ILISTPROVIDER_H_INCLUDED
#define LISTPROVIDERS_ILISTPROVIDER_H_INCLUDED
#include "IListProvider.h"
#endif

#ifndef LISTPROVIDERS_GUILIB_GUISTATICITEM_H_INCLUDED
#define LISTPROVIDERS_GUILIB_GUISTATICITEM_H_INCLUDED
#include "guilib/GUIStaticItem.h"
#endif

#ifndef LISTPROVIDERS_UTILS_JOB_H_INCLUDED
#define LISTPROVIDERS_UTILS_JOB_H_INCLUDED
#include "utils/Job.h"
#endif

#ifndef LISTPROVIDERS_THREADS_CRITICALSECTION_H_INCLUDED
#define LISTPROVIDERS_THREADS_CRITICALSECTION_H_INCLUDED
#include "threads/CriticalSection.h"
#endif

#ifndef LISTPROVIDERS_INTERFACES_IANNOUNCER_H_INCLUDED
#define LISTPROVIDERS_INTERFACES_IANNOUNCER_H_INCLUDED
#include "interfaces/IAnnouncer.h"
#endif


class TiXmlElement;

typedef enum
{
  VIDEO,
  AUDIO,
  PICTURE,
  PROGRAM
} InfoTagType;

class CDirectoryProvider :
  public IListProvider,
  public IJobCallback,
  public ANNOUNCEMENT::IAnnouncer
{
public:
  typedef enum
  {
    OK,
    PENDING,
    DONE
  } UpdateState;

  CDirectoryProvider(const TiXmlElement *element, int parentID);
  virtual ~CDirectoryProvider();

  virtual bool Update(bool forceRefresh);
  virtual void Announce(ANNOUNCEMENT::AnnouncementFlag flag, const char *sender, const char *message, const CVariant &data);
  virtual void Fetch(std::vector<CGUIListItemPtr> &items) const;
  virtual void Reset(bool immediately = false);
  virtual bool OnClick(const CGUIListItemPtr &item);
  virtual bool IsUpdating() const;

  // callback from directory job
  virtual void OnJobComplete(unsigned int jobID, bool success, CJob *job);
private:
  unsigned int     m_updateTime;
  UpdateState      m_updateState;
  bool             m_isDbUpdating;
  bool             m_isAnnounced;
  unsigned int     m_jobID;
  CGUIInfoLabel    m_url;
  CGUIInfoLabel    m_target;
  std::string      m_currentUrl;
  std::string      m_currentTarget;   ///< \brief node.target property on the list as a whole
  std::vector<CGUIStaticItemPtr> m_items;
  std::vector<InfoTagType> m_itemTypes;
  CCriticalSection m_section;

  void FireJob();
  void RegisterListProvider(bool hasLibraryContent);
  bool UpdateURL();
  static bool HasLibraryContent(const std::string &url);
};
