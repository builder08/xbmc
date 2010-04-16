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

#include "GUIViewStateAddonBrowser.h"
#include "GUIBaseContainer.h"
#include "FileItem.h"
#include "Key.h"
#include "ViewState.h"
#include "Settings.h"
#include "FileSystem/Directory.h"
#include "addons/Addon.h"
#include "addons/AddonManager.h"

using namespace XFILE;
using namespace ADDON;

CGUIViewStateAddonBrowser::CGUIViewStateAddonBrowser(const CFileItemList& items) : CGUIViewState(items)
{
  AddSortMethod(SORT_METHOD_LABEL, 551, LABEL_MASKS("%L", "%I", "%L", ""));  // Filename, Size | Foldername, empty
  AddSortMethod(SORT_METHOD_DATE, 552, LABEL_MASKS("%L", "%J", "%L", "%J"));  // Filename, Date | Foldername, Date
  SetSortMethod(SORT_METHOD_LABEL);

  SetViewAsControl(DEFAULT_VIEW_LIST);

  SetSortOrder(SORT_ORDER_ASC);
  LoadViewState(items.m_strPath, WINDOW_ADDON_BROWSER);
}

void CGUIViewStateAddonBrowser::SaveViewState()
{
  SaveViewToDb(m_items.m_strPath, WINDOW_ADDON_BROWSER);
}

CStdString CGUIViewStateAddonBrowser::GetExtensions()
{
  return "";
}

VECSOURCES& CGUIViewStateAddonBrowser::GetSources()
{
  m_sources.clear();

  VECADDONS addons;
  CAddonMgr::Get()->GetAllAddons(addons);
  if (addons.size())
  {
    CMediaSource share;
    share.strPath = "addons://enabled/";
    share.m_iDriveType = CMediaSource::SOURCE_TYPE_LOCAL;
    share.strName = g_localizeStrings.Get(24062);
    share.m_strThumbnailImage = "DefaultHardDisk.png";
    m_sources.push_back(share);
  }
  addons.clear();
  CAddonMgr::Get()->GetAddons(ADDON_REPOSITORY,addons,CONTENT_NONE,true);
  for (unsigned int i=0;i<addons.size();++i)
  {
    CMediaSource share;
    share.strPath = "addons://"+addons[i]->ID();
    share.m_iDriveType = CMediaSource::SOURCE_TYPE_LOCAL;
    share.strName = addons[i]->Name();
    m_sources.push_back(share);
  }

  return CGUIViewState::GetSources();
}

