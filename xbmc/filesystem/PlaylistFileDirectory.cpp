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

#ifndef FILESYSTEM_PLAYLISTFILEDIRECTORY_H_INCLUDED
#define FILESYSTEM_PLAYLISTFILEDIRECTORY_H_INCLUDED
#include "PlaylistFileDirectory.h"
#endif

#ifndef FILESYSTEM_UTILS_LOG_H_INCLUDED
#define FILESYSTEM_UTILS_LOG_H_INCLUDED
#include "utils/log.h"
#endif

#ifndef FILESYSTEM_PLAYLISTS_PLAYLISTFACTORY_H_INCLUDED
#define FILESYSTEM_PLAYLISTS_PLAYLISTFACTORY_H_INCLUDED
#include "playlists/PlayListFactory.h"
#endif

#ifndef FILESYSTEM_FILE_H_INCLUDED
#define FILESYSTEM_FILE_H_INCLUDED
#include "File.h"
#endif

#ifndef FILESYSTEM_PLAYLISTS_PLAYLIST_H_INCLUDED
#define FILESYSTEM_PLAYLISTS_PLAYLIST_H_INCLUDED
#include "playlists/PlayList.h"
#endif


using namespace std;
using namespace PLAYLIST;

namespace XFILE
{
  CPlaylistFileDirectory::CPlaylistFileDirectory()
  {
  }

  CPlaylistFileDirectory::~CPlaylistFileDirectory()
  {
  }

  bool CPlaylistFileDirectory::GetDirectory(const CStdString& strPath, CFileItemList& items)
  {
    auto_ptr<CPlayList> pPlayList (CPlayListFactory::Create(strPath));
    if ( NULL != pPlayList.get())
    {
      // load it
      if (!pPlayList->Load(strPath))
        return false; //hmmm unable to load playlist?

      CPlayList playlist = *pPlayList;
      // convert playlist items to songs
      for (int i = 0; i < (int)playlist.size(); ++i)
      {
        CFileItemPtr item = playlist[i];
        item->m_iprogramCount = i;  // hack for playlist order
        items.Add(item);
      }
    }
    return true;
  }

  bool CPlaylistFileDirectory::ContainsFiles(const CStdString& strPath)
  {
    auto_ptr<CPlayList> pPlayList (CPlayListFactory::Create(strPath));
    if ( NULL != pPlayList.get())
    {
      // load it
      if (!pPlayList->Load(strPath))
        return false; //hmmm unable to load playlist?

      return (pPlayList->size() > 1);
    }
    return false;
  }

  bool CPlaylistFileDirectory::Remove(const char *strPath)
  {
    return XFILE::CFile::Delete(strPath);
  }
}

