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

#include "addons/Addon.h"
#include "GUIMediaWindow.h"
#include "utils/CriticalSection.h"
#include "utils/Job.h"
#include "PictureThumbLoader.h"

class CFileItem;
class CFileItemList;
class CFileOperationJob;

class CGUIWindowAddonBrowser :
      public CGUIMediaWindow,
      public IJobCallback
{
public:
  CGUIWindowAddonBrowser(void);
  virtual ~CGUIWindowAddonBrowser(void);
  virtual bool OnMessage(CGUIMessage& message);
  virtual bool OnAction(const CAction &action);

  void RegisterJob(const CStdString& id, CFileOperationJob* job,
                   unsigned int jobid);

  // job callback
  void OnJobComplete(unsigned int jobID, bool success, CJob* job);

  static std::pair<CFileOperationJob*,unsigned int> AddJob(const CStdString& path);

  /*! \brief Popup a selection dialog with a list of addons satisfying content and type
   \param type the type of addon wanted
   \param content the content of the addon - if set to CONTENT_NONE all addons will be retrieved
   \param addonID [out] the addon ID of the selected item
   \return true if an addon was selected, false if an error occurred or if the selection process was cancelled
   */
  static bool SelectAddonID(ADDON::TYPE type, CONTENT_TYPE content, CStdString &addonID);
protected:
  void UnRegisterJob(CFileOperationJob* job);
  virtual void GetContextButtons(int itemNumber, CContextButtons &buttons);
  virtual bool OnContextButton(int itemNumber, CONTEXT_BUTTON button);
  virtual bool OnClick(int iItem);
  virtual void UpdateButtons();
  virtual bool GetDirectory(const CStdString &strDirectory, CFileItemList &items);
  virtual bool Update(const CStdString &strDirectory);
  std::map<CStdString,CFileOperationJob*> m_idtojob;
  std::map<CStdString,unsigned int> m_idtojobid;
  std::map<CFileOperationJob*,CStdString> m_jobtoid;
  CCriticalSection m_critSection;
  CPictureThumbLoader m_thumbLoader;
  CStdString m_startDirectory;
};

