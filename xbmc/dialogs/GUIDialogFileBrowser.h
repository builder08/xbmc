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

#ifndef DIALOGS_GUILIB_GUIDIALOG_H_INCLUDED
#define DIALOGS_GUILIB_GUIDIALOG_H_INCLUDED
#include "guilib/GUIDialog.h"
#endif

#ifndef DIALOGS_FILESYSTEM_VIRTUALDIRECTORY_H_INCLUDED
#define DIALOGS_FILESYSTEM_VIRTUALDIRECTORY_H_INCLUDED
#include "filesystem/VirtualDirectory.h"
#endif

#ifndef DIALOGS_FILESYSTEM_DIRECTORYHISTORY_H_INCLUDED
#define DIALOGS_FILESYSTEM_DIRECTORYHISTORY_H_INCLUDED
#include "filesystem/DirectoryHistory.h"
#endif

#ifndef DIALOGS_VIEW_GUIVIEWCONTROL_H_INCLUDED
#define DIALOGS_VIEW_GUIVIEWCONTROL_H_INCLUDED
#include "view/GUIViewControl.h"
#endif

#ifndef DIALOGS_PICTURES_PICTURETHUMBLOADER_H_INCLUDED
#define DIALOGS_PICTURES_PICTURETHUMBLOADER_H_INCLUDED
#include "pictures/PictureThumbLoader.h"
#endif


class CFileItem;
class CFileItemList;

class CGUIDialogFileBrowser : public CGUIDialog, public IBackgroundLoaderObserver
{
public:
  CGUIDialogFileBrowser(void);
  virtual ~CGUIDialogFileBrowser(void);
  virtual bool OnMessage(CGUIMessage& message);
  virtual bool OnAction(const CAction &action);
  virtual bool OnBack(int actionID);
  virtual void FrameMove();
  virtual void OnWindowLoaded();
  virtual void OnWindowUnload();
  bool IsConfirmed() { return m_bConfirmed; };
  void SetHeading(const CStdString &heading);

  static bool ShowAndGetDirectory(const VECSOURCES &shares, const CStdString &heading, CStdString &path, bool bWriteOnly=false);
  static bool ShowAndGetFile(const VECSOURCES &shares, const CStdString &mask, const CStdString &heading, CStdString &path, bool useThumbs = false, bool useFileDirectories = false);
  static bool ShowAndGetFile(const CStdString &directory, const CStdString &mask, const CStdString &heading, CStdString &path, bool useThumbs = false, bool useFileDirectories = false, bool singleList = false);
  static bool ShowAndGetSource(CStdString &path, bool allowNetworkShares, VECSOURCES* additionalShare = NULL, const CStdString& strType="");
  static bool ShowAndGetFileList(const VECSOURCES &shares, const CStdString &mask, const CStdString &heading, CStdStringArray &path, bool useThumbs = false, bool useFileDirectories = false);
  static bool ShowAndGetImage(const VECSOURCES &shares, const CStdString &heading, CStdString &path);
  static bool ShowAndGetImage(const CFileItemList &items, const VECSOURCES &shares, const CStdString &heading, CStdString &path, bool* flip=NULL, int label=21371);
  static bool ShowAndGetImageList(const VECSOURCES &shares, const CStdString &heading, CStdStringArray &path);

  void SetSources(const VECSOURCES &shares);

  virtual void OnItemLoaded(CFileItem *item) {};

  virtual bool HasListItems() const { return true; };
  virtual CFileItemPtr GetCurrentListItem(int offset = 0);
  int GetViewContainerID() const { return m_viewControl.GetCurrentControl(); };

protected:
  void GoParentFolder();
  void OnClick(int iItem);
  void OnSort();
  void ClearFileItems();
  void Update(const CStdString &strDirectory);
  bool HaveDiscOrConnection( int iDriveType );
  bool OnPopupMenu(int iItem);
  void OnAddNetworkLocation();
  void OnAddMediaSource();
  void OnEditMediaSource(CFileItem* pItem);
  CGUIControl *GetFirstFocusableControl(int id);

  VECSOURCES m_shares;
  XFILE::CVirtualDirectory m_rootDir;
  CFileItemList* m_vecItems;
  CFileItem* m_Directory;
  CStdString m_strParentPath;
  CStdString m_selectedPath;
  CDirectoryHistory m_history;
  int m_browsingForFolders; // 0 - no, 1 - yes, 2 - yes, only writable
  bool m_bConfirmed;
  int m_bFlip;
  bool m_addNetworkShareEnabled;
  bool m_flipEnabled;
  CStdString m_addSourceType;
  bool m_browsingForImages;
  bool m_useFileDirectories;
  bool m_singleList;              // if true, we have no shares or anything
  bool m_multipleSelection;
  CStdStringArray m_markedPath;

  CPictureThumbLoader m_thumbLoader;
  CGUIViewControl m_viewControl;
};
