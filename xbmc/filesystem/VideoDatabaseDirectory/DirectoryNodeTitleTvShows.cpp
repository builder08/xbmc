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

#ifndef VIDEODATABASEDIRECTORY_DIRECTORYNODETITLETVSHOWS_H_INCLUDED
#define VIDEODATABASEDIRECTORY_DIRECTORYNODETITLETVSHOWS_H_INCLUDED
#include "DirectoryNodeTitleTvShows.h"
#endif

#ifndef VIDEODATABASEDIRECTORY_QUERYPARAMS_H_INCLUDED
#define VIDEODATABASEDIRECTORY_QUERYPARAMS_H_INCLUDED
#include "QueryParams.h"
#endif

#ifndef VIDEODATABASEDIRECTORY_VIDEO_VIDEODATABASE_H_INCLUDED
#define VIDEODATABASEDIRECTORY_VIDEO_VIDEODATABASE_H_INCLUDED
#include "video/VideoDatabase.h"
#endif


using namespace XFILE::VIDEODATABASEDIRECTORY;

CDirectoryNodeTitleTvShows::CDirectoryNodeTitleTvShows(const CStdString& strName, CDirectoryNode* pParent)
  : CDirectoryNode(NODE_TYPE_TITLE_TVSHOWS, strName, pParent)
{

}

NODE_TYPE CDirectoryNodeTitleTvShows::GetChildType() const
{
  return NODE_TYPE_SEASONS;
}

CStdString CDirectoryNodeTitleTvShows::GetLocalizedName() const
{
  CVideoDatabase db;
  if (db.Open())
    return db.GetTvShowTitleById(GetID());
  return "";
}

bool CDirectoryNodeTitleTvShows::GetContent(CFileItemList& items) const
{
  CVideoDatabase videodatabase;
  if (!videodatabase.Open())
    return false;

  CQueryParams params;
  CollectQueryParams(params);

  bool bSuccess=videodatabase.GetTvShowsNav(BuildPath(), items, params.GetGenreId(), params.GetYear(), params.GetActorId(), params.GetDirectorId(), params.GetStudioId(), params.GetTagId());

  videodatabase.Close();

  return bSuccess;
}
