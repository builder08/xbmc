/*
 *      Copyright (C) 2005-2013 Team XBMC
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
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include "GUIViewStateMusic.h"
#include "PlayListPlayer.h"
#include "video/VideoDatabase.h"
#include "settings/AdvancedSettings.h"
#include "settings/MediaSourceSettings.h"
#include "settings/Settings.h"
#include "FileItem.h"
#include "guilib/WindowIDs.h"
#include "Util.h"
#include "guilib/LocalizeStrings.h"
#include "utils/log.h"
#include "view/ViewStateSettings.h"

#include "filesystem/Directory.h"
#include "filesystem/MusicDatabaseDirectory.h"
#include "filesystem/VideoDatabaseDirectory.h"

using namespace XFILE;
using namespace MUSICDATABASEDIRECTORY;

int CGUIViewStateWindowMusic::GetPlaylist()
{
  //return PLAYLIST_MUSIC_TEMP;
  return PLAYLIST_MUSIC;
}

bool CGUIViewStateWindowMusic::AutoPlayNextItem()
{
  return CSettings::Get().GetBool("musicplayer.autoplaynextitem") &&
         !CSettings::Get().GetBool("musicplayer.queuebydefault");
}

CStdString CGUIViewStateWindowMusic::GetLockType()
{
  return "music";
}

CStdString CGUIViewStateWindowMusic::GetExtensions()
{
  return g_advancedSettings.m_musicExtensions;
}

VECSOURCES& CGUIViewStateWindowMusic::GetSources()
{
  AddAddonsSource("audio", g_localizeStrings.Get(1038), "DefaultAddonMusic.png");
  return CGUIViewState::GetSources();
}

CGUIViewStateMusicSearch::CGUIViewStateMusicSearch(const CFileItemList& items) : CGUIViewStateWindowMusic(items)
{
  CStdString strTrackLeft=CSettings::Get().GetString("musicfiles.librarytrackformat");
  if (strTrackLeft.IsEmpty())
    strTrackLeft = CSettings::Get().GetString("musicfiles.trackformat");
  CStdString strTrackRight=CSettings::Get().GetString("musicfiles.librarytrackformatright");
  if (strTrackRight.IsEmpty())
    strTrackRight = CSettings::Get().GetString("musicfiles.trackformatright");

  CStdString strAlbumLeft = g_advancedSettings.m_strMusicLibraryAlbumFormat;
  if (strAlbumLeft.IsEmpty())
    strAlbumLeft = "%B"; // album
  CStdString strAlbumRight = g_advancedSettings.m_strMusicLibraryAlbumFormatRight;
  if (strAlbumRight.IsEmpty())
    strAlbumRight = "%A"; // artist

  if (CSettings::Get().GetBool("filelists.ignorethewhensorting"))
  {
    AddSortMethod(SORT_METHOD_TITLE_IGNORE_THE, 556, LABEL_MASKS("%T - %A", "%D", "%L", "%A"));  // Title, Artist, Duration| empty, empty
    SetSortMethod(SORT_METHOD_TITLE_IGNORE_THE);
  }
  else
  {
    AddSortMethod(SORT_METHOD_TITLE, 556, LABEL_MASKS("%T - %A", "%D", "%L", "%A"));  // Title, Artist, Duration| empty, empty
    SetSortMethod(SORT_METHOD_TITLE);
  }

  const CViewState *viewState = CViewStateSettings::Get().Get("musicnavsongs");
  SetViewAsControl(viewState->m_viewMode);
  SetSortOrder(viewState->m_sortOrder);

  LoadViewState(items.GetPath(), WINDOW_MUSIC_NAV);
}

void CGUIViewStateMusicSearch::SaveViewState()
{
  SaveViewToDb(m_items.GetPath(), WINDOW_MUSIC_NAV, CViewStateSettings::Get().Get("musicnavsongs"));
}

CGUIViewStateMusicDatabase::CGUIViewStateMusicDatabase(const CFileItemList& items) : CGUIViewStateWindowMusic(items)
{
  CMusicDatabaseDirectory dir;
  NODE_TYPE NodeType=dir.GetDirectoryChildType(items.GetPath());

  CStdString strTrackLeft=CSettings::Get().GetString("musicfiles.librarytrackformat");
  if (strTrackLeft.IsEmpty())
    strTrackLeft = CSettings::Get().GetString("musicfiles.trackformat");
  CStdString strTrackRight=CSettings::Get().GetString("musicfiles.librarytrackformatright");
  if (strTrackRight.IsEmpty())
    strTrackRight = CSettings::Get().GetString("musicfiles.trackformatright");

  CStdString strAlbumLeft = g_advancedSettings.m_strMusicLibraryAlbumFormat;
  if (strAlbumLeft.IsEmpty())
    strAlbumLeft = "%B"; // album
  CStdString strAlbumRight = g_advancedSettings.m_strMusicLibraryAlbumFormatRight;
  if (strAlbumRight.IsEmpty())
    strAlbumRight = "%A"; // artist

  CLog::Log(LOGDEBUG,"Album format left  = [%s]", strAlbumLeft.c_str());
  CLog::Log(LOGDEBUG,"Album format right = [%s]", strAlbumRight.c_str());

  switch (NodeType)
  {
  case NODE_TYPE_OVERVIEW:
    {
      AddSortMethod(SORT_METHOD_NONE, 551, LABEL_MASKS("%F", "", "%L", ""));  // Filename, empty | Foldername, empty
      SetSortMethod(SORT_METHOD_NONE);

      SetViewAsControl(DEFAULT_VIEW_LIST);

      SetSortOrder(SortOrderNone);
    }
    break;
  case NODE_TYPE_TOP100:
    {
      AddSortMethod(SORT_METHOD_NONE, 551, LABEL_MASKS("%F", "", "%L", ""));  // Filename, empty | Foldername, empty
      SetSortMethod(SORT_METHOD_NONE);

      SetViewAsControl(DEFAULT_VIEW_LIST);

      SetSortOrder(SortOrderNone);
    }
    break;
  case NODE_TYPE_GENRE:
    {
      AddSortMethod(SORT_METHOD_GENRE, 515, LABEL_MASKS("%F", "", "%G", ""));  // Filename, empty | Genre, empty
      SetSortMethod(SORT_METHOD_GENRE);

      SetViewAsControl(DEFAULT_VIEW_LIST);

      SetSortOrder(SortOrderAscending);
    }
    break;
  case NODE_TYPE_YEAR:
    {
      AddSortMethod(SORT_METHOD_LABEL, 551, LABEL_MASKS("%F", "", "%Y", ""));  // Filename, empty | Year, empty
      SetSortMethod(SORT_METHOD_LABEL);

      SetViewAsControl(DEFAULT_VIEW_LIST);

      SetSortOrder(SortOrderAscending);
    }
    break;
  case NODE_TYPE_ARTIST:
    {
      if (CSettings::Get().GetBool("filelists.ignorethewhensorting"))
      {
        AddSortMethod(SORT_METHOD_ARTIST_IGNORE_THE, 557, LABEL_MASKS("%F", "", "%A", ""));  // Filename, empty | Artist, empty
        SetSortMethod(SORT_METHOD_ARTIST_IGNORE_THE);
      }
      else
      {
        AddSortMethod(SORT_METHOD_ARTIST, 557, LABEL_MASKS("%F", "", "%A", ""));  // Filename, empty | Artist, empty
        SetSortMethod(SORT_METHOD_ARTIST);
      }

      const CViewState *viewState = CViewStateSettings::Get().Get("musicnavartists");
      SetViewAsControl(viewState->m_viewMode);
      SetSortOrder(viewState->m_sortOrder);
    }
    break;
  case NODE_TYPE_ALBUM_COMPILATIONS:
  case NODE_TYPE_ALBUM:
  case NODE_TYPE_YEAR_ALBUM:
    {
      // album
      if (CSettings::Get().GetBool("filelists.ignorethewhensorting"))
        AddSortMethod(SORT_METHOD_ALBUM_IGNORE_THE, 558, LABEL_MASKS("%F", "", strAlbumLeft, strAlbumRight));  // Filename, empty | Userdefined, Userdefined
      else
        AddSortMethod(SORT_METHOD_ALBUM, 558, LABEL_MASKS("%F", "", strAlbumLeft, strAlbumRight));  // Filename, empty | Userdefined, Userdefined

      // artist
      if (CSettings::Get().GetBool("filelists.ignorethewhensorting"))
        AddSortMethod(SORT_METHOD_ARTIST_IGNORE_THE, 557, LABEL_MASKS("%F", "", strAlbumLeft, strAlbumRight));  // Filename, empty | Userdefined, Userdefined
      else
        AddSortMethod(SORT_METHOD_ARTIST, 557, LABEL_MASKS("%F", "", strAlbumLeft, strAlbumRight));  // Filename, empty | Userdefined, Userdefined

      // year
      AddSortMethod(SORT_METHOD_YEAR, 562, LABEL_MASKS("%F", "", strAlbumLeft, strAlbumRight));

      const CViewState *viewState = CViewStateSettings::Get().Get("musicnavalbums");
      SetSortMethod(viewState->m_sortMethod);
      SetViewAsControl(viewState->m_viewMode);
      SetSortOrder(viewState->m_sortOrder);
    }
    break;
  case NODE_TYPE_ALBUM_RECENTLY_ADDED:
    {
      AddSortMethod(SORT_METHOD_NONE, 552, LABEL_MASKS("%F", "", strAlbumLeft, strAlbumRight));  // Filename, empty | Userdefined, Userdefined
      SetSortMethod(SORT_METHOD_NONE);

      SetViewAsControl(CViewStateSettings::Get().Get("musicnavalbums")->m_viewMode);

      SetSortOrder(SortOrderNone);
    }
    break;
  case NODE_TYPE_ALBUM_RECENTLY_ADDED_SONGS:
    {
      AddSortMethod(SORT_METHOD_NONE, 552, LABEL_MASKS(strTrackLeft, strTrackRight));  // Userdefined, Userdefined | empty, empty
      SetSortMethod(SORT_METHOD_NONE);

      SetViewAsControl(CViewStateSettings::Get().Get("musicnavsongs")->m_viewMode);

      SetSortOrder(SortOrderNone);
    }
    break;
  case NODE_TYPE_ALBUM_RECENTLY_PLAYED:
    {
      AddSortMethod(SORT_METHOD_NONE, 551, LABEL_MASKS("%F", "", strAlbumLeft, strAlbumRight));  // Filename, empty | Userdefined, Userdefined
      SetSortMethod(SORT_METHOD_NONE);

      SetViewAsControl(CViewStateSettings::Get().Get("musicnavalbums")->m_viewMode);

      SetSortOrder(SortOrderNone);
    }
    break;
  case NODE_TYPE_ALBUM_RECENTLY_PLAYED_SONGS:
    {
      AddSortMethod(SORT_METHOD_NONE, 551, LABEL_MASKS(strTrackLeft, strTrackRight));  // Userdefined, Userdefined | empty, empty
      SetSortMethod(SORT_METHOD_NONE);

      SetViewAsControl(CViewStateSettings::Get().Get("musicnavalbums")->m_viewMode);

      SetSortOrder(SortOrderNone);
    }
    break;
  case NODE_TYPE_ALBUM_TOP100:
    {
      AddSortMethod(SORT_METHOD_NONE, 551, LABEL_MASKS("%F", "", strAlbumLeft, strAlbumRight));  // Filename, empty | Userdefined, Userdefined
      SetSortMethod(SORT_METHOD_NONE);

      SetViewAsControl(DEFAULT_VIEW_LIST);
      SetSortOrder(SortOrderNone);
    }
    break;
  case NODE_TYPE_SINGLES:
    {
      if (CSettings::Get().GetBool("filelists.ignorethewhensorting"))
      {
        AddSortMethod(SORT_METHOD_ARTIST_IGNORE_THE, 557, LABEL_MASKS("%A - %T", "%D"));  // Artist, Title, Duration| empty, empty
        AddSortMethod(SORT_METHOD_TITLE_IGNORE_THE, 556, LABEL_MASKS("%T - %A", "%D"));  // Title, Artist, Duration| empty, empty
        AddSortMethod(SORT_METHOD_LABEL_IGNORE_THE, 551, LABEL_MASKS(strTrackLeft, strTrackRight));
      }
      else
      {
        AddSortMethod(SORT_METHOD_ARTIST, 557, LABEL_MASKS("%A - %T", "%D"));  // Artist, Title, Duration| empty, empty
        AddSortMethod(SORT_METHOD_TITLE, 556, LABEL_MASKS("%T - %A", "%D"));  // Title, Artist, Duration| empty, empty
        AddSortMethod(SORT_METHOD_LABEL, 551, LABEL_MASKS(strTrackLeft, strTrackRight));
      }
      AddSortMethod(SORT_METHOD_DURATION, 180, LABEL_MASKS("%T - %A", "%D"));  // Titel, Artist, Duration| empty, empty
      AddSortMethod(SORT_METHOD_SONG_RATING, 563, LABEL_MASKS("%T - %A", "%R"));  // Title - Artist, Rating

      const CViewState *viewState = CViewStateSettings::Get().Get("musicnavsongs");
      SetSortMethod(viewState->m_sortMethod);
      SetViewAsControl(viewState->m_viewMode);
      SetSortOrder(viewState->m_sortOrder);
    }
    break;
  case NODE_TYPE_ALBUM_COMPILATIONS_SONGS:
  case NODE_TYPE_ALBUM_TOP100_SONGS:
  case NODE_TYPE_YEAR_SONG:
  case NODE_TYPE_SONG:
    {
      AddSortMethod(SORT_METHOD_TRACKNUM, 554, LABEL_MASKS(strTrackLeft, strTrackRight));  // Userdefined, Userdefined| empty, empty
      if (CSettings::Get().GetBool("filelists.ignorethewhensorting"))
      {
        AddSortMethod(SORT_METHOD_TITLE_IGNORE_THE, 556, LABEL_MASKS("%T - %A", "%D"));  // Title, Artist, Duration| empty, empty
        AddSortMethod(SORT_METHOD_ALBUM_IGNORE_THE, 558, LABEL_MASKS("%B - %T - %A", "%D"));  // Album, Title, Artist, Duration| empty, empty
        AddSortMethod(SORT_METHOD_ARTIST_IGNORE_THE, 557, LABEL_MASKS("%A - %T", "%D"));  // Artist, Title, Duration| empty, empty
        AddSortMethod(SORT_METHOD_LABEL_IGNORE_THE, 551, LABEL_MASKS(strTrackLeft, strTrackRight));
      }
      else
      {
        AddSortMethod(SORT_METHOD_TITLE, 556, LABEL_MASKS("%T - %A", "%D"));  // Title, Artist, Duration| empty, empty
        AddSortMethod(SORT_METHOD_ALBUM, 558, LABEL_MASKS("%B - %T - %A", "%D"));  // Album, Title, Artist, Duration| empty, empty
        AddSortMethod(SORT_METHOD_ARTIST, 557, LABEL_MASKS("%A - %T", "%D"));  // Artist, Title, Duration| empty, empty
        AddSortMethod(SORT_METHOD_LABEL, 551, LABEL_MASKS(strTrackLeft, strTrackRight));
      }
      AddSortMethod(SORT_METHOD_DURATION, 180, LABEL_MASKS("%T - %A", "%D"));  // Titel, Artist, Duration| empty, empty
      AddSortMethod(SORT_METHOD_SONG_RATING, 563, LABEL_MASKS("%T - %A", "%R"));  // Title - Artist, Rating
      AddSortMethod(SORT_METHOD_YEAR, 562, LABEL_MASKS("%T - %A", "%Y")); // Title, Artist, Year

      const CViewState *viewState = CViewStateSettings::Get().Get("musicnavsongs");
      // the "All Albums" entries always default to SORT_METHOD_ALBUM as this is most logical - user can always
      // change it and the change will be saved for this particular path
      if (dir.IsAllItem(items.GetPath()))
        SetSortMethod(CSettings::Get().GetBool("filelists.ignorethewhensorting") ? SORT_METHOD_ALBUM_IGNORE_THE : SORT_METHOD_ALBUM);
      else
        SetSortMethod(viewState->m_sortMethod);

      AddSortMethod(SORT_METHOD_PLAYCOUNT, 567, LABEL_MASKS("%T - %A", "%V"));  // Titel - Artist, PlayCount

      SetViewAsControl(viewState->m_viewMode);
      SetSortOrder(viewState->m_sortOrder);
    }
    break;
  case NODE_TYPE_SONG_TOP100:
    {
      AddSortMethod(SORT_METHOD_NONE, 576, LABEL_MASKS("%T - %A", "%V"));
      SetSortMethod(SORT_METHOD_PLAYCOUNT);

      SetViewAsControl(CViewStateSettings::Get().Get("musicnavsongs")->m_viewMode);

      SetSortOrder(SortOrderNone);
    }
    break;
  default:
    break;
  }

  LoadViewState(items.GetPath(), WINDOW_MUSIC_NAV);
}

void CGUIViewStateMusicDatabase::SaveViewState()
{
  CMusicDatabaseDirectory dir;
  NODE_TYPE NodeType=dir.GetDirectoryChildType(m_items.GetPath());

  switch (NodeType)
  {
    case NODE_TYPE_ARTIST:
      SaveViewToDb(m_items.GetPath(), WINDOW_MUSIC_NAV, CViewStateSettings::Get().Get("musicnavartists"));
      break;
    case NODE_TYPE_ALBUM_COMPILATIONS:
    case NODE_TYPE_ALBUM:
    case NODE_TYPE_YEAR_ALBUM:
      SaveViewToDb(m_items.GetPath(), WINDOW_MUSIC_NAV, CViewStateSettings::Get().Get("musicnavalbums"));
      break;
    case NODE_TYPE_ALBUM_RECENTLY_ADDED:
    case NODE_TYPE_ALBUM_TOP100:
    case NODE_TYPE_ALBUM_RECENTLY_PLAYED:
      SaveViewToDb(m_items.GetPath(), WINDOW_MUSIC_NAV);
      break;
    case NODE_TYPE_SINGLES:
    case NODE_TYPE_ALBUM_COMPILATIONS_SONGS:
    case NODE_TYPE_SONG:
    case NODE_TYPE_YEAR_SONG:
      SaveViewToDb(m_items.GetPath(), WINDOW_MUSIC_NAV, CViewStateSettings::Get().Get("musicnavsongs"));
      break;
    case NODE_TYPE_ALBUM_RECENTLY_PLAYED_SONGS:
    case NODE_TYPE_ALBUM_RECENTLY_ADDED_SONGS:
    case NODE_TYPE_SONG_TOP100:
      SaveViewToDb(m_items.GetPath(), WINDOW_MUSIC_NAV);
      break;
    default:
      SaveViewToDb(m_items.GetPath(), WINDOW_MUSIC_NAV);
      break;
  }
}


CGUIViewStateMusicSmartPlaylist::CGUIViewStateMusicSmartPlaylist(const CFileItemList& items) : CGUIViewStateWindowMusic(items)
{
  if (items.GetContent() == "songs" || items.GetContent() == "mixed") 
  {
    CStdString strTrackLeft=CSettings::Get().GetString("musicfiles.trackformat");
    CStdString strTrackRight=CSettings::Get().GetString("musicfiles.trackformatright");

    AddSortMethod(SORT_METHOD_TRACKNUM, 554, LABEL_MASKS(strTrackLeft, strTrackRight));  // Userdefined, Userdefined| empty, empty
    if (CSettings::Get().GetBool("filelists.ignorethewhensorting"))
    {
      AddSortMethod(SORT_METHOD_TITLE_IGNORE_THE, 556, LABEL_MASKS("%T - %A", "%D"));  // Title, Artist, Duration| empty, empty
      AddSortMethod(SORT_METHOD_ALBUM_IGNORE_THE, 558, LABEL_MASKS("%B - %T - %A", "%D"));  // Album, Titel, Artist, Duration| empty, empty
      AddSortMethod(SORT_METHOD_ARTIST_IGNORE_THE, 557, LABEL_MASKS("%A - %T", "%D"));  // Artist, Titel, Duration| empty, empty
      AddSortMethod(SORT_METHOD_LABEL_IGNORE_THE, 551, LABEL_MASKS(strTrackLeft, strTrackRight));
    }
    else
    {
      AddSortMethod(SORT_METHOD_TITLE, 556, LABEL_MASKS("%T - %A", "%D"));  // Title, Artist, Duration| empty, empty
      AddSortMethod(SORT_METHOD_ALBUM, 558, LABEL_MASKS("%B - %T - %A", "%D"));  // Album, Titel, Artist, Duration| empty, empty
      AddSortMethod(SORT_METHOD_ARTIST, 557, LABEL_MASKS("%A - %T", "%D"));  // Artist, Titel, Duration| empty, empty
      AddSortMethod(SORT_METHOD_LABEL, 551, LABEL_MASKS(strTrackLeft, strTrackRight));
    }
    AddSortMethod(SORT_METHOD_DURATION, 180, LABEL_MASKS("%T - %A", "%D"));  // Titel, Artist, Duration| empty, empty
    AddSortMethod(SORT_METHOD_SONG_RATING, 563, LABEL_MASKS("%T - %A", "%R"));  // Titel, Artist, Rating| empty, empty
    AddPlaylistOrder(items, LABEL_MASKS(strTrackLeft, strTrackRight));

    SetViewAsControl(CViewStateSettings::Get().Get("musicnavsongs")->m_viewMode);
  } 
  else if (items.GetContent() == "albums") 
  {
    CStdString strAlbumLeft = g_advancedSettings.m_strMusicLibraryAlbumFormat;
    if (strAlbumLeft.IsEmpty())
      strAlbumLeft = "%B"; // album
    CStdString strAlbumRight = g_advancedSettings.m_strMusicLibraryAlbumFormatRight;
    if (strAlbumRight.IsEmpty())
      strAlbumRight = "%A"; // artist

    // album
    if (CSettings::Get().GetBool("filelists.ignorethewhensorting"))
      AddSortMethod(SORT_METHOD_ALBUM_IGNORE_THE, 558, LABEL_MASKS("%F", "", strAlbumLeft, strAlbumRight));  // Filename, empty | Userdefined, Userdefined
    else
      AddSortMethod(SORT_METHOD_ALBUM, 558, LABEL_MASKS("%F", "", strAlbumLeft, strAlbumRight));  // Filename, empty | Userdefined, Userdefined
    
    // artist
    if (CSettings::Get().GetBool("filelists.ignorethewhensorting"))
      AddSortMethod(SORT_METHOD_ARTIST_IGNORE_THE, 557, LABEL_MASKS("%F", "", strAlbumLeft, strAlbumRight));  // Filename, empty | Userdefined, Userdefined
    else
      AddSortMethod(SORT_METHOD_ARTIST, 557, LABEL_MASKS("%F", "", strAlbumLeft, strAlbumRight));  // Filename, empty | Userdefined, Userdefined
    
    // year
    AddSortMethod(SORT_METHOD_YEAR, 562, LABEL_MASKS("%F", "", strAlbumLeft, strAlbumRight));

    AddPlaylistOrder(items, LABEL_MASKS("%F", "", strAlbumLeft, strAlbumRight));

    SetViewAsControl(CViewStateSettings::Get().Get("musicnavalbums")->m_viewMode);
  } 
  else 
  {
    CLog::Log(LOGERROR,"Music Smart Playlist must be one of songs, mixed or albums");
  }
  
  LoadViewState(items.GetPath(), WINDOW_MUSIC_NAV);
}

void CGUIViewStateMusicSmartPlaylist::SaveViewState()
{
  SaveViewToDb(m_items.GetPath(), WINDOW_MUSIC_NAV, CViewStateSettings::Get().Get("musicnavsongs"));
}

CGUIViewStateMusicPlaylist::CGUIViewStateMusicPlaylist(const CFileItemList& items) : CGUIViewStateWindowMusic(items)
{
  CStdString strTrackLeft=CSettings::Get().GetString("musicfiles.trackformat");
  CStdString strTrackRight=CSettings::Get().GetString("musicfiles.trackformatright");

  AddSortMethod(SORT_METHOD_PLAYLIST_ORDER, 559, LABEL_MASKS(strTrackLeft, strTrackRight));
  AddSortMethod(SORT_METHOD_TRACKNUM, 554, LABEL_MASKS(strTrackLeft, strTrackRight));  // Userdefined, Userdefined| empty, empty
  if (CSettings::Get().GetBool("filelists.ignorethewhensorting"))
  {
    AddSortMethod(SORT_METHOD_TITLE_IGNORE_THE, 556, LABEL_MASKS("%T - %A", "%D"));  // Title, Artist, Duration| empty, empty
    AddSortMethod(SORT_METHOD_ALBUM_IGNORE_THE, 558, LABEL_MASKS("%B - %T - %A", "%D"));  // Album, Titel, Artist, Duration| empty, empty
    AddSortMethod(SORT_METHOD_ARTIST_IGNORE_THE, 557, LABEL_MASKS("%A - %T", "%D"));  // Artist, Titel, Duration| empty, empty
    AddSortMethod(SORT_METHOD_LABEL_IGNORE_THE, 551, LABEL_MASKS(strTrackLeft, strTrackRight));
  }
  else
  {
    AddSortMethod(SORT_METHOD_TITLE, 556, LABEL_MASKS("%T - %A", "%D"));  // Title, Artist, Duration| empty, empty
    AddSortMethod(SORT_METHOD_ALBUM, 558, LABEL_MASKS("%B - %T - %A", "%D"));  // Album, Titel, Artist, Duration| empty, empty
    AddSortMethod(SORT_METHOD_ARTIST, 557, LABEL_MASKS("%A - %T", "%D"));  // Artist, Titel, Duration| empty, empty
    AddSortMethod(SORT_METHOD_LABEL, 551, LABEL_MASKS(strTrackLeft, strTrackRight));
  }
  AddSortMethod(SORT_METHOD_DURATION, 180, LABEL_MASKS("%T - %A", "%D"));  // Titel, Artist, Duration| empty, empty
  AddSortMethod(SORT_METHOD_SONG_RATING, 563, LABEL_MASKS("%T - %A", "%R"));  // Titel, Artist, Rating| empty, empty

  SetSortMethod(SORT_METHOD_PLAYLIST_ORDER);

  const CViewState *viewState = CViewStateSettings::Get().Get("musicfiles");
  SetViewAsControl(viewState->m_viewMode);
  SetSortOrder(viewState->m_sortOrder);

  LoadViewState(items.GetPath(), WINDOW_MUSIC_FILES);
}

void CGUIViewStateMusicPlaylist::SaveViewState()
{
  SaveViewToDb(m_items.GetPath(), WINDOW_MUSIC_FILES);
}


CGUIViewStateWindowMusicNav::CGUIViewStateWindowMusicNav(const CFileItemList& items) : CGUIViewStateWindowMusic(items)
{
  if (items.IsVirtualDirectoryRoot())
  {
    AddSortMethod(SORT_METHOD_NONE, 551, LABEL_MASKS("%F", "%I", "%L", ""));  // Filename, Size | Foldername, empty
    SetSortMethod(SORT_METHOD_NONE);

    SetViewAsControl(DEFAULT_VIEW_LIST);

    SetSortOrder(SortOrderNone);
  }
  else
  {
    if (items.IsVideoDb() && items.Size() > (CSettings::Get().GetBool("filelists.showparentdiritems")?1:0))
    {
      XFILE::VIDEODATABASEDIRECTORY::CQueryParams params;
      XFILE::CVideoDatabaseDirectory::GetQueryParams(items[CSettings::Get().GetBool("filelists.showparentdiritems")?1:0]->GetPath(),params);
      if (params.GetMVideoId() != -1)
      {
        if (CSettings::Get().GetBool("filelists.ignorethewhensorting"))
          AddSortMethod(SORT_METHOD_LABEL_IGNORE_THE, 551, LABEL_MASKS("%T", "%Y"));  // Filename, Duration | Foldername, empty
        else
          AddSortMethod(SORT_METHOD_LABEL, 551, LABEL_MASKS("%T", "%Y"));  // Filename, Duration | Foldername, empty
        AddSortMethod(SORT_METHOD_YEAR,562, LABEL_MASKS("%T", "%Y"));
        if (CSettings::Get().GetBool("filelists.ignorethewhensorting"))
        {
          AddSortMethod(SORT_METHOD_ARTIST_IGNORE_THE,557, LABEL_MASKS("%A - %T", "%Y"));
          AddSortMethod(SORT_METHOD_ALBUM_IGNORE_THE,558, LABEL_MASKS("%B - %T", "%Y"));
        }
        else
        {
          AddSortMethod(SORT_METHOD_ARTIST,557, LABEL_MASKS("%A - %T", "%Y"));
          AddSortMethod(SORT_METHOD_ALBUM,558, LABEL_MASKS("%B - %T", "%Y"));
        }
        CStdString strTrackLeft=CSettings::Get().GetString("musicfiles.trackformat");
        CStdString strTrackRight=CSettings::Get().GetString("musicfiles.trackformatright");
        AddSortMethod(SORT_METHOD_TRACKNUM, 554, LABEL_MASKS(strTrackLeft, strTrackRight));  // Userdefined, Userdefined| empty, empty
      }
      else
      {
        AddSortMethod(SORT_METHOD_LABEL, 551, LABEL_MASKS("%F", "%D", "%L", ""));  // Filename, Duration | Foldername, empty
        SetSortMethod(SORT_METHOD_LABEL);
      }
    }
    else
    {
      AddSortMethod(SORT_METHOD_LABEL, 551, LABEL_MASKS("%F", "%D", "%L", ""));  // Filename, Duration | Foldername, empty
      SetSortMethod(SORT_METHOD_LABEL);
    }
    SetViewAsControl(DEFAULT_VIEW_LIST);

    SetSortOrder(SortOrderAscending);
  }
  LoadViewState(items.GetPath(), WINDOW_MUSIC_NAV);
}

void CGUIViewStateWindowMusicNav::SaveViewState()
{
  SaveViewToDb(m_items.GetPath(), WINDOW_MUSIC_NAV);
}

void CGUIViewStateWindowMusicNav::AddOnlineShares()
{
  if (!g_advancedSettings.m_bVirtualShares)
    return;

  VECSOURCES *musicSources = CMediaSourceSettings::Get().GetSources("music");

  for (int i = 0; i < (int)musicSources->size(); ++i)
  {
    CMediaSource share = musicSources->at(i);
  }
}

VECSOURCES& CGUIViewStateWindowMusicNav::GetSources()
{
  //  Setup shares we want to have
  m_sources.clear();
  //  Musicdb shares
  CFileItemList items;
  CDirectory::GetDirectory("musicdb://", items, "");
  for (int i=0; i<items.Size(); ++i)
  {
    CFileItemPtr item=items[i];
    CMediaSource share;
    share.strName=item->GetLabel();
    share.strPath = item->GetPath();
    share.m_strThumbnailImage = item->GetIconImage();
    share.m_iDriveType = CMediaSource::SOURCE_TYPE_LOCAL;
    m_sources.push_back(share);
  }

  //  Playlists share
  CMediaSource share;
  share.strName=g_localizeStrings.Get(136); // Playlists
  share.strPath = "special://musicplaylists/";
  share.m_strThumbnailImage = CUtil::GetDefaultFolderThumb("DefaultMusicPlaylists.png");
  share.m_iDriveType = CMediaSource::SOURCE_TYPE_LOCAL;
  m_sources.push_back(share);

  AddOnlineShares();

  // Search share
  share.strName=g_localizeStrings.Get(137); // Search
  share.strPath = "musicsearch://";
  share.m_strThumbnailImage = CUtil::GetDefaultFolderThumb("DefaultMusicSearch.png");
  share.m_iDriveType = CMediaSource::SOURCE_TYPE_LOCAL;
  m_sources.push_back(share);

  // music video share
  CVideoDatabase database;
  database.Open();
  if (database.HasContent(VIDEODB_CONTENT_MUSICVIDEOS))
  {
    share.strName = g_localizeStrings.Get(20389);
    share.strPath = "videodb://musicvideos/";
    share.m_strThumbnailImage = CUtil::GetDefaultFolderThumb("DefaultMusicVideos.png");
    share.m_iDriveType = CMediaSource::SOURCE_TYPE_LOCAL;
    m_sources.push_back(share);
  }

  return CGUIViewStateWindowMusic::GetSources();
}

CGUIViewStateWindowMusicSongs::CGUIViewStateWindowMusicSongs(const CFileItemList& items) : CGUIViewStateWindowMusic(items)
{
  if (items.IsVirtualDirectoryRoot())
  {
    AddSortMethod(SORT_METHOD_LABEL, 551, LABEL_MASKS()); // Preformated
    AddSortMethod(SORT_METHOD_DRIVE_TYPE, 564, LABEL_MASKS()); // Preformated
    SetSortMethod(SORT_METHOD_LABEL);

    SetViewAsControl(DEFAULT_VIEW_LIST);

    SetSortOrder(SortOrderAscending);
  }
  else if (items.GetPath() == "special://musicplaylists/")
  { // playlists list sorts by label only, ignoring folders
    AddSortMethod(SORT_METHOD_LABEL_IGNORE_FOLDERS, 551, LABEL_MASKS("%F", "%D", "%L", ""));  // Filename, Duration | Foldername, empty
    SetSortMethod(SORT_METHOD_LABEL_IGNORE_FOLDERS);
  }
  else
  {
    CStdString strTrackLeft=CSettings::Get().GetString("musicfiles.trackformat");
    CStdString strTrackRight=CSettings::Get().GetString("musicfiles.trackformatright");

    if (CSettings::Get().GetBool("filelists.ignorethewhensorting"))
      AddSortMethod(SORT_METHOD_LABEL_IGNORE_THE, 551, LABEL_MASKS(strTrackLeft, strTrackRight, "%L", ""));  // Userdefined, Userdefined | FolderName, empty
    else
      AddSortMethod(SORT_METHOD_LABEL, 551, LABEL_MASKS(strTrackLeft, strTrackRight, "%L", ""));  // Userdefined, Userdefined | FolderName, empty
    AddSortMethod(SORT_METHOD_SIZE, 553, LABEL_MASKS(strTrackLeft, "%I", "%L", "%I"));  // Userdefined, Size | FolderName, Size
    AddSortMethod(SORT_METHOD_BITRATE, 623, LABEL_MASKS(strTrackLeft, "%X", "%L", "%X"));  // Userdefined, Bitrate | FolderName, Bitrate  
    AddSortMethod(SORT_METHOD_DATE, 552, LABEL_MASKS(strTrackLeft, "%J", "%L", "%J"));  // Userdefined, Date | FolderName, Date
    AddSortMethod(SORT_METHOD_FILE, 561, LABEL_MASKS(strTrackLeft, strTrackRight, "%L", ""));  // Userdefined, Userdefined | FolderName, empty
    AddSortMethod(SORT_METHOD_LISTENERS,20455,LABEL_MASKS(strTrackLeft, "%W", "%L", "%W"));

    const CViewState *viewState = CViewStateSettings::Get().Get("musicfiles");
    SetSortMethod(viewState->m_sortMethod);
    SetViewAsControl(viewState->m_viewMode);
    SetSortOrder(viewState->m_sortOrder);
  }
  LoadViewState(items.GetPath(), WINDOW_MUSIC_FILES);
}

void CGUIViewStateWindowMusicSongs::SaveViewState()
{
  SaveViewToDb(m_items.GetPath(), WINDOW_MUSIC_FILES, CViewStateSettings::Get().Get("musicfiles"));
}

VECSOURCES& CGUIViewStateWindowMusicSongs::GetSources()
{
  VECSOURCES *musicSources = CMediaSourceSettings::Get().GetSources("music");
  AddOrReplace(*musicSources, CGUIViewStateWindowMusic::GetSources());
  return *musicSources;
}

CGUIViewStateWindowMusicPlaylist::CGUIViewStateWindowMusicPlaylist(const CFileItemList& items) : CGUIViewStateWindowMusic(items)
{
  CStdString strTrackLeft=CSettings::Get().GetString("musicfiles.nowplayingtrackformat");
  if (strTrackLeft.IsEmpty())
    strTrackLeft = CSettings::Get().GetString("musicfiles.trackformat");
  CStdString strTrackRight=CSettings::Get().GetString("musicfiles.nowplayingtrackformatright");
  if (strTrackRight.IsEmpty())
    strTrackRight = CSettings::Get().GetString("musicfiles.trackformatright");

  AddSortMethod(SORT_METHOD_NONE, 551, LABEL_MASKS(strTrackLeft, strTrackRight, "%L", ""));  // Userdefined, Userdefined | FolderName, empty
  SetSortMethod(SORT_METHOD_NONE);

  SetViewAsControl(DEFAULT_VIEW_LIST);

  SetSortOrder(SortOrderNone);

  LoadViewState(items.GetPath(), WINDOW_MUSIC_PLAYLIST);
}

void CGUIViewStateWindowMusicPlaylist::SaveViewState()
{
  SaveViewToDb(m_items.GetPath(), WINDOW_MUSIC_PLAYLIST);
}

int CGUIViewStateWindowMusicPlaylist::GetPlaylist()
{
  return PLAYLIST_MUSIC;
}

bool CGUIViewStateWindowMusicPlaylist::AutoPlayNextItem()
{
  return false;
}

bool CGUIViewStateWindowMusicPlaylist::HideParentDirItems()
{
  return true;
}

VECSOURCES& CGUIViewStateWindowMusicPlaylist::GetSources()
{
  m_sources.clear();
  //  Playlist share
  CMediaSource share;
  share.strPath = "playlistmusic://";
  share.m_iDriveType = CMediaSource::SOURCE_TYPE_LOCAL;
  m_sources.push_back(share);

  // CGUIViewState::GetSources would add music plugins
  return m_sources;
}
