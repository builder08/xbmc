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

#include "DirectoryNodeTitleTvShows.h"
#include "DatabaseManager.h"
#include "QueryParams.h"
#include "video/VideoDatabase.h"

using namespace XFILE::VIDEODATABASEDIRECTORY;

CDirectoryNodeTitleTvShows::CDirectoryNodeTitleTvShows(const std::string& strName, CDirectoryNode* pParent)
  : CDirectoryNode(NODE_TYPE_TITLE_TVSHOWS, strName, pParent)
{

}

NODE_TYPE CDirectoryNodeTitleTvShows::GetChildType() const
{
  return NODE_TYPE_SEASONS;
}

std::string CDirectoryNodeTitleTvShows::GetLocalizedName() const
{
  CVideoDatabase *database = CDatabaseManager::Get().GetVideoDatabase();
  return database->GetTvShowTitleById(GetID());
}

bool CDirectoryNodeTitleTvShows::GetContent(CFileItemList& items) const
{
  CVideoDatabase *database = CDatabaseManager::Get().GetVideoDatabase();
  CQueryParams params;
  CollectQueryParams(params);

  return database->GetTvShowsNav(BuildPath(), items, params.GetGenreId(), params.GetYear(), params.GetActorId(), params.GetDirectorId(), params.GetStudioId(), params.GetTagId());
}
