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

#ifndef MUSICDATABASEDIRECTORY_DIRECTORYNODEALBUMTOP100_H_INCLUDED
#define MUSICDATABASEDIRECTORY_DIRECTORYNODEALBUMTOP100_H_INCLUDED
#include "DirectoryNodeAlbumTop100.h"
#endif

#ifndef MUSICDATABASEDIRECTORY_MUSIC_MUSICDATABASE_H_INCLUDED
#define MUSICDATABASEDIRECTORY_MUSIC_MUSICDATABASE_H_INCLUDED
#include "music/MusicDatabase.h"
#endif

#ifndef MUSICDATABASEDIRECTORY_FILEITEM_H_INCLUDED
#define MUSICDATABASEDIRECTORY_FILEITEM_H_INCLUDED
#include "FileItem.h"
#endif

#ifndef MUSICDATABASEDIRECTORY_UTILS_STRINGUTILS_H_INCLUDED
#define MUSICDATABASEDIRECTORY_UTILS_STRINGUTILS_H_INCLUDED
#include "utils/StringUtils.h"
#endif


using namespace XFILE::MUSICDATABASEDIRECTORY;

CDirectoryNodeAlbumTop100::CDirectoryNodeAlbumTop100(const CStdString& strName, CDirectoryNode* pParent)
  : CDirectoryNode(NODE_TYPE_ALBUM_TOP100, strName, pParent)
{

}

NODE_TYPE CDirectoryNodeAlbumTop100::GetChildType() const
{
  if (GetName()=="-1")
    return NODE_TYPE_ALBUM_TOP100_SONGS;

  return NODE_TYPE_SONG;
}

CStdString CDirectoryNodeAlbumTop100::GetLocalizedName() const
{
  CMusicDatabase db;
  if (db.Open())
    return db.GetAlbumById(GetID());
  return "";
}

bool CDirectoryNodeAlbumTop100::GetContent(CFileItemList& items) const
{
  CMusicDatabase musicdatabase;
  if (!musicdatabase.Open())
    return false;

  VECALBUMS albums;
  if (!musicdatabase.GetTop100Albums(albums))
  {
    musicdatabase.Close();
    return false;
  }

  for (int i=0; i<(int)albums.size(); ++i)
  {
    CAlbum& album=albums[i];
    CStdString strDir = StringUtils::Format("%s%ld/", BuildPath().c_str(), album.idAlbum);
    CFileItemPtr pItem(new CFileItem(strDir, album));
    items.Add(pItem);
  }

  musicdatabase.Close();

  return true;
}
