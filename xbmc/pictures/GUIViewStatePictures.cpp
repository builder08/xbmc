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

#ifndef PICTURES_GUIVIEWSTATEPICTURES_H_INCLUDED
#define PICTURES_GUIVIEWSTATEPICTURES_H_INCLUDED
#include "GUIViewStatePictures.h"
#endif

#ifndef PICTURES_FILEITEM_H_INCLUDED
#define PICTURES_FILEITEM_H_INCLUDED
#include "FileItem.h"
#endif

#ifndef PICTURES_VIEW_VIEWSTATE_H_INCLUDED
#define PICTURES_VIEW_VIEWSTATE_H_INCLUDED
#include "view/ViewState.h"
#endif

#ifndef PICTURES_SETTINGS_ADVANCEDSETTINGS_H_INCLUDED
#define PICTURES_SETTINGS_ADVANCEDSETTINGS_H_INCLUDED
#include "settings/AdvancedSettings.h"
#endif

#ifndef PICTURES_SETTINGS_MEDIASOURCESETTINGS_H_INCLUDED
#define PICTURES_SETTINGS_MEDIASOURCESETTINGS_H_INCLUDED
#include "settings/MediaSourceSettings.h"
#endif

#ifndef PICTURES_SETTINGS_SETTINGS_H_INCLUDED
#define PICTURES_SETTINGS_SETTINGS_H_INCLUDED
#include "settings/Settings.h"
#endif

#ifndef PICTURES_FILESYSTEM_DIRECTORY_H_INCLUDED
#define PICTURES_FILESYSTEM_DIRECTORY_H_INCLUDED
#include "filesystem/Directory.h"
#endif

#ifndef PICTURES_FILESYSTEM_PLUGINDIRECTORY_H_INCLUDED
#define PICTURES_FILESYSTEM_PLUGINDIRECTORY_H_INCLUDED
#include "filesystem/PluginDirectory.h"
#endif

#ifndef PICTURES_GUILIB_LOCALIZESTRINGS_H_INCLUDED
#define PICTURES_GUILIB_LOCALIZESTRINGS_H_INCLUDED
#include "guilib/LocalizeStrings.h"
#endif

#ifndef PICTURES_GUILIB_WINDOWIDS_H_INCLUDED
#define PICTURES_GUILIB_WINDOWIDS_H_INCLUDED
#include "guilib/WindowIDs.h"
#endif

#ifndef PICTURES_VIEW_VIEWSTATESETTINGS_H_INCLUDED
#define PICTURES_VIEW_VIEWSTATESETTINGS_H_INCLUDED
#include "view/ViewStateSettings.h"
#endif


using namespace XFILE;
using namespace ADDON;

CGUIViewStateWindowPictures::CGUIViewStateWindowPictures(const CFileItemList& items) : CGUIViewState(items)
{
  if (items.IsVirtualDirectoryRoot())
  {
    AddSortMethod(SortByLabel, 551, LABEL_MASKS());
    AddSortMethod(SortByDriveType, 564, LABEL_MASKS());
    SetSortMethod(SortByLabel);

    SetViewAsControl(DEFAULT_VIEW_LIST);

    SetSortOrder(SortOrderAscending);
  }
  else
  {
    AddSortMethod(SortByLabel, 551, LABEL_MASKS("%L", "%I", "%L", ""));  // Filename, Size | Foldername, empty
    AddSortMethod(SortBySize, 553, LABEL_MASKS("%L", "%I", "%L", "%I"));  // Filename, Size | Foldername, Size
    AddSortMethod(SortByDate, 552, LABEL_MASKS("%L", "%J", "%L", "%J"));  // Filename, Date | Foldername, Date
    AddSortMethod(SortByDateTaken, 577, LABEL_MASKS("%L", "%t", "%L", "%J"));  // Filename, DateTaken | Foldername, Date
    AddSortMethod(SortByFile, 561, LABEL_MASKS("%L", "%I", "%L", ""));  // Filename, Size | FolderName, empty

    const CViewState *viewState = CViewStateSettings::Get().Get("pictures");
    SetSortMethod(viewState->m_sortDescription);
    SetViewAsControl(viewState->m_viewMode);
    SetSortOrder(viewState->m_sortDescription.sortOrder);
  }
  LoadViewState(items.GetPath(), WINDOW_PICTURES);
}

void CGUIViewStateWindowPictures::SaveViewState()
{
  SaveViewToDb(m_items.GetPath(), WINDOW_PICTURES, CViewStateSettings::Get().Get("pictures"));
}

CStdString CGUIViewStateWindowPictures::GetLockType()
{
  return "pictures";
}

CStdString CGUIViewStateWindowPictures::GetExtensions()
{
  if (CSettings::Get().GetBool("pictures.showvideos"))
    return g_advancedSettings.m_pictureExtensions+"|"+g_advancedSettings.m_videoExtensions;

  return g_advancedSettings.m_pictureExtensions;
}

VECSOURCES& CGUIViewStateWindowPictures::GetSources()
{
  VECSOURCES *pictureSources = CMediaSourceSettings::Get().GetSources("pictures");
  AddAddonsSource("image", g_localizeStrings.Get(1039), "DefaultAddonPicture.png");
  AddOrReplace(*pictureSources, CGUIViewState::GetSources());
  return *pictureSources;
}

