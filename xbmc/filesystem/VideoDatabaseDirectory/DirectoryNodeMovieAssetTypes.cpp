/*
 *  Copyright (C) 2005-2025 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#include "DirectoryNodeMovieAssetTypes.h"

//#include "FileItem.h"
//#include "FileItemList.h"
#include "QueryParams.h"
//#include "video/VideoDatabase.h"
#include "video/VideoManagerTypes.h"

using namespace XFILE::VIDEODATABASEDIRECTORY;

CDirectoryNodeMovieAssetTypes::CDirectoryNodeMovieAssetTypes(const std::string& strName,
                                                             CDirectoryNode* pParent)
  : CDirectoryNode(NodeType::MOVIE_ASSET_TYPES, strName, pParent)
{
}

/*

//! @todo: implement some navigation that returns the available asset types for the movie

bool CDirectoryNodeMovieAssetTypes::GetContent(CFileItemList& items) const
{
  CVideoDatabase videodatabase;
  if (!videodatabase.Open())
    return false;

  CQueryParams params;
  CollectQueryParams(params);

  int details = items.HasProperty("set_videodb_details")
                    ? items.GetProperty("set_videodb_details").asInteger32()
                    : VideoDbDetailsNone;

  bool bSuccess = videodatabase.GetMoviesNav(
      BuildPath(), items, params.GetGenreId(), params.GetYear(), params.GetActorId(),
      params.GetDirectorId(), params.GetStudioId(), params.GetCountryId(), params.GetSetId(),
      params.GetTagId(), SortDescription(), details);

  videodatabase.Close();

  return bSuccess;
}
*/

NodeType CDirectoryNodeMovieAssetTypes::GetChildType() const
{
  CQueryParams params;
  CollectQueryParams(params);

  switch (params.GetVideoAssetType())
  {
    case -2: // special value for all assets + extras virtual folder
    case static_cast<int>(VideoAssetType::VERSION):
      return NodeType::MOVIE_ASSETS_VERSIONS;
    case static_cast<int>(VideoAssetType::EXTRA):
      return NodeType::MOVIE_ASSETS_EXTRAS;
    case 0:
    default:
      return NodeType::MOVIE_ASSETS;
  }
}
