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

#include "GUIViewState.h"
#include "GUIViewStateAddonBrowser.h"
#include "GUIViewStateMusic.h"
#include "GUIViewStateVideo.h"
#include "GUIViewStatePictures.h"
#include "GUIViewStatePrograms.h"
#include "GUIViewStateScripts.h"
#include "PlayListPlayer.h"
#include "Util.h"
#include "URL.h"
#include "GUIPassword.h"
#include "GUIBaseContainer.h" // for VIEW_TYPE_*
#include "ViewDatabase.h"
#include "AutoSwitch.h"
#include "GUIWindowManager.h"
#include "addons/AddonManager.h"
#include "ViewState.h"
#include "GUISettings.h"
#include "Settings.h"
#include "FileItem.h"
#include "Key.h"

using namespace std;

CStdString CGUIViewState::m_strPlaylistDirectory;
VECSOURCES CGUIViewState::m_sources;

CGUIViewState* CGUIViewState::GetViewState(int windowId, const CFileItemList& items)
{
  // don't expect derived classes to clear the sources
  m_sources.clear();

  if (windowId == 0)
    return GetViewState(g_windowManager.GetActiveWindow(),items);

  const CURL& url=items.GetAsUrl();

  if (items.HasSortDetails())
    return new CGUIViewStateFromItems(items);

  if (url.GetProtocol()=="musicdb")
    return new CGUIViewStateMusicDatabase(items);

  if (url.GetProtocol()=="musicsearch")
    return new CGUIViewStateMusicSearch(items);

  if (items.IsSmartPlayList())
  {
    if (items.GetContent() == "songs")
      return new CGUIViewStateMusicSmartPlaylist(items);
    else if (items.GetContent() == "musicvideos") // TODO: Update this
      return new CGUIViewStateMusicSmartPlaylist(items);
    else if (items.GetContent() == "tvshows")
      return new CGUIViewStateVideoTVShows(items);
    else if (items.GetContent() == "episodes")
      return new CGUIViewStateVideoEpisodes(items);
    else if (items.GetContent() == "movies")
      return new CGUIViewStateVideoMovies(items);
  }

  if (items.IsPlayList())
    return new CGUIViewStateMusicPlaylist(items);

  if (url.GetProtocol() == "shout")
    return new CGUIViewStateMusicShoutcast(items);

  if (url.GetProtocol() == "lastfm")
    return new CGUIViewStateMusicLastFM(items);

  if (items.m_strPath == "special://musicplaylists/")
    return new CGUIViewStateWindowMusicSongs(items);

  if (windowId==WINDOW_MUSIC_NAV)
    return new CGUIViewStateWindowMusicNav(items);

  if (windowId==WINDOW_MUSIC_FILES)
    return new CGUIViewStateWindowMusicSongs(items);

  if (windowId==WINDOW_MUSIC_PLAYLIST)
    return new CGUIViewStateWindowMusicPlaylist(items);

  if (windowId==WINDOW_MUSIC_PLAYLIST_EDITOR)
    return new CGUIViewStateWindowMusicSongs(items);

  if (windowId==WINDOW_VIDEO_FILES)
    return new CGUIViewStateWindowVideoFiles(items);

  if (windowId==WINDOW_VIDEO_NAV)
    return new CGUIViewStateWindowVideoNav(items);

  if (windowId==WINDOW_VIDEO_PLAYLIST)
    return new CGUIViewStateWindowVideoPlaylist(items);

  if (windowId==WINDOW_SCRIPTS)
    return new CGUIViewStateWindowScripts(items);

  if (windowId==WINDOW_PICTURES)
    return new CGUIViewStateWindowPictures(items);

  if (windowId==WINDOW_PROGRAMS)
    return new CGUIViewStateWindowPrograms(items);
  
  if (windowId==WINDOW_ADDON_BROWSER)
    return new CGUIViewStateAddonBrowser(items);

  //  Use as fallback/default
  return new CGUIViewStateGeneral(items);
}

CGUIViewState::CGUIViewState(const CFileItemList& items, const CONTENT_TYPE& content/*=CONTENT_NONE*/) : m_items(items)
{
  m_currentViewAsControl=0;
  m_currentSortMethod=0;
  m_sortOrder=SORT_ORDER_ASC;
  m_content = content;
}

CGUIViewState::~CGUIViewState()
{
}

SORT_ORDER CGUIViewState::GetDisplaySortOrder() const
{
  // we actually treat some sort orders in reverse, so that we can have
  // the one sort order variable to save but it can be ascending usually,
  // and descending for the views which should be usually descending.
  // default sort order for date, size, program count + rating is reversed
  SORT_METHOD sortMethod = GetSortMethod();
  if (sortMethod == SORT_METHOD_DATE || sortMethod == SORT_METHOD_SIZE ||
      sortMethod == SORT_METHOD_VIDEO_RATING || sortMethod == SORT_METHOD_PROGRAM_COUNT ||
      sortMethod == SORT_METHOD_SONG_RATING)
  {
    if (m_sortOrder == SORT_ORDER_ASC) return SORT_ORDER_DESC;
    if (m_sortOrder == SORT_ORDER_DESC) return SORT_ORDER_ASC;
  }
  return m_sortOrder;
}

SORT_ORDER CGUIViewState::SetNextSortOrder()
{
  if (m_sortOrder==SORT_ORDER_ASC)
    SetSortOrder(SORT_ORDER_DESC);
  else
    SetSortOrder(SORT_ORDER_ASC);

  SaveViewState();

  return m_sortOrder;
}

int CGUIViewState::GetViewAsControl() const
{
  return m_currentViewAsControl;
}

void CGUIViewState::SetViewAsControl(int viewAsControl)
{
  if (viewAsControl == DEFAULT_VIEW_AUTO)
    m_currentViewAsControl = CAutoSwitch::GetView(m_items);
  else
    m_currentViewAsControl = viewAsControl;
}

void CGUIViewState::SaveViewAsControl(int viewAsControl)
{
  SetViewAsControl(viewAsControl);
  SaveViewState();
}

SORT_METHOD CGUIViewState::GetSortMethod() const
{
  if (m_currentSortMethod>=0 && m_currentSortMethod<(int)m_sortMethods.size())
    return m_sortMethods[m_currentSortMethod].m_sortMethod;

  return SORT_METHOD_NONE;
}

int CGUIViewState::GetSortMethodLabel() const
{
  if (m_currentSortMethod>=0 && m_currentSortMethod<(int)m_sortMethods.size())
    return m_sortMethods[m_currentSortMethod].m_buttonLabel;

  return 103; // Sort By: Name
}

void CGUIViewState::GetSortMethods(vector< pair<int,int> > &sortMethods) const
{
  for (unsigned int i = 0; i < m_sortMethods.size(); i++)
    sortMethods.push_back(make_pair(m_sortMethods[i].m_sortMethod, m_sortMethods[i].m_buttonLabel));
}

void CGUIViewState::GetSortMethodLabelMasks(LABEL_MASKS& masks) const
{
  if (m_currentSortMethod>=0 && m_currentSortMethod<(int)m_sortMethods.size())
  {
    masks=m_sortMethods[m_currentSortMethod].m_labelMasks;
    return;
  }

  masks.m_strLabelFile.Empty();
  masks.m_strLabel2File.Empty();
  masks.m_strLabelFolder.Empty();
  masks.m_strLabel2Folder.Empty();
  return;
}

void CGUIViewState::AddSortMethod(SORT_METHOD sortMethod, int buttonLabel, LABEL_MASKS labelmasks)
{
  SORT_METHOD_DETAILS sort;
  sort.m_sortMethod=sortMethod;
  sort.m_buttonLabel=buttonLabel;
  sort.m_labelMasks=labelmasks;

  m_sortMethods.push_back(sort);
}

void CGUIViewState::SetCurrentSortMethod(int method)
{
  bool ignoreThe = g_guiSettings.GetBool("filelists.ignorethewhensorting");

  if (method < SORT_METHOD_NONE || method >= SORT_METHOD_MAX)
    return; // invalid

  // compensate for "Ignore The" options to make it easier on the skin
  if (ignoreThe && (method == SORT_METHOD_LABEL || method == SORT_METHOD_TITLE || method == SORT_METHOD_ARTIST || method == SORT_METHOD_ALBUM || method == SORT_METHOD_STUDIO || method == SORT_METHOD_VIDEO_SORT_TITLE))
    method++;
  else if (!ignoreThe && (method == SORT_METHOD_LABEL_IGNORE_THE || method == SORT_METHOD_TITLE_IGNORE_THE || method == SORT_METHOD_ARTIST_IGNORE_THE || method==SORT_METHOD_ALBUM_IGNORE_THE || method == SORT_METHOD_STUDIO_IGNORE_THE || method == SORT_METHOD_VIDEO_SORT_TITLE_IGNORE_THE))
    method--;

  SetSortMethod((SORT_METHOD)method);
  SaveViewState();
}

void CGUIViewState::SetSortMethod(SORT_METHOD sortMethod)
{
  for (int i=0; i<(int)m_sortMethods.size(); ++i)
  {
    if (m_sortMethods[i].m_sortMethod==sortMethod)
    {
      m_currentSortMethod=i;
      break;
    }
  }
}

SORT_METHOD CGUIViewState::SetNextSortMethod(int direction /* = 1 */)
{
  m_currentSortMethod += direction;

  if (m_currentSortMethod >= (int)m_sortMethods.size())
    m_currentSortMethod=0;
  if (m_currentSortMethod < 0)
    m_currentSortMethod = m_sortMethods.size() ? (int)m_sortMethods.size() - 1 : 0;

  SaveViewState();

  return GetSortMethod();
}

bool CGUIViewState::HideExtensions()
{
  return !g_guiSettings.GetBool("filelists.showextensions");
}

bool CGUIViewState::HideParentDirItems()
{
  return !g_guiSettings.GetBool("filelists.showparentdiritems");
}

bool CGUIViewState::DisableAddSourceButtons()
{
  if (g_settings.GetCurrentProfile().canWriteSources() || g_passwordManager.bMasterUser)
    return !g_guiSettings.GetBool("filelists.showaddsourcebuttons");

  return true;
}

int CGUIViewState::GetPlaylist()
{
  return PLAYLIST_NONE;
}

const CStdString& CGUIViewState::GetPlaylistDirectory()
{
  return m_strPlaylistDirectory;
}

void CGUIViewState::SetPlaylistDirectory(const CStdString& strDirectory)
{
  m_strPlaylistDirectory=strDirectory;
  CUtil::RemoveSlashAtEnd(m_strPlaylistDirectory);
}

bool CGUIViewState::IsCurrentPlaylistDirectory(const CStdString& strDirectory)
{
  if (g_playlistPlayer.GetCurrentPlaylist()!=GetPlaylist())
    return false;

  CStdString strDir=strDirectory;
  CUtil::RemoveSlashAtEnd(strDir);

  return (m_strPlaylistDirectory==strDir);
}


bool CGUIViewState::AutoPlayNextItem()
{
  return false;
}

CStdString CGUIViewState::GetLockType()
{
  return "";
}

CStdString CGUIViewState::GetExtensions()
{
  return "";
}

VECSOURCES& CGUIViewState::GetSources()
{
  // more consolidation could happen here for all content types
  // - playlists, autoconfig network shares, whatnot

  if (m_content == CONTENT_NONE)
    return m_sources;

  ADDON::VECADDONS addons;
  ADDON::CAddonMgr::Get().GetAddons(ADDON::ADDON_PLUGIN, addons, m_content);

  for (unsigned i=0; i<addons.size(); i++)
  {
    // format for sources's path is
    // eg. pictures://UUID
    CMediaSource plugin;
    CURL path;
    path.SetProtocol("plugin");
    path.SetHostName(addons[i]->ID());
    plugin.strPath = path.Get();
    plugin.strName = addons[i]->Name();
    plugin.m_strThumbnailImage = addons[i]->Icon(); //FIXME cache by UUID
    plugin.m_iDriveType = CMediaSource::SOURCE_TYPE_REMOTE;
    m_sources.push_back(plugin);
  }
  return m_sources;
}

CGUIViewStateGeneral::CGUIViewStateGeneral(const CFileItemList& items) : CGUIViewState(items)
{
  AddSortMethod(SORT_METHOD_LABEL, 551, LABEL_MASKS("%F", "%I", "%L", ""));  // Filename, size | Foldername, empty
  SetSortMethod(SORT_METHOD_LABEL);

  SetViewAsControl(DEFAULT_VIEW_LIST);

  SetSortOrder(SORT_ORDER_ASC);
}

void CGUIViewState::SetSortOrder(SORT_ORDER sortOrder)
{
  if (GetSortMethod() == SORT_METHOD_NONE)
    m_sortOrder = SORT_ORDER_NONE;
  else
    m_sortOrder = sortOrder;
}

void CGUIViewState::LoadViewState(const CStdString &path, int windowID)
{ // get our view state from the db
  CViewDatabase db;
  if (db.Open())
  {
    CViewState state;
    if (db.GetViewState(path, windowID, state))
    {
      SetViewAsControl(state.m_viewMode);
      SetSortMethod(state.m_sortMethod);
      SetSortOrder(state.m_sortOrder);
    }
    db.Close();
  }
}

void CGUIViewState::SaveViewToDb(const CStdString &path, int windowID, CViewState *viewState)
{
  CViewDatabase db;
  if (db.Open())
  {
    CViewState state(m_currentViewAsControl, GetSortMethod(), m_sortOrder);
    if (viewState)
      *viewState = state;
    db.SetViewState(path, windowID, state);
    db.Close();
    if (viewState)
      g_settings.Save();
  }
}

CGUIViewStateFromItems::CGUIViewStateFromItems(const CFileItemList &items) : CGUIViewState(items)
{
  const vector<SORT_METHOD_DETAILS> &details = items.GetSortDetails();
  for (unsigned int i = 0; i < details.size(); i++)
  {
    const SORT_METHOD_DETAILS sort = details[i];
    AddSortMethod(sort.m_sortMethod, sort.m_buttonLabel, sort.m_labelMasks);
  }
  // TODO: Should default sort/view mode be specified?
  m_currentSortMethod = 0;

  SetViewAsControl(DEFAULT_VIEW_LIST);

  SetSortOrder(SORT_ORDER_ASC);
  LoadViewState(items.m_strPath, g_windowManager.GetActiveWindow());
}

void CGUIViewStateFromItems::SaveViewState()
{
  SaveViewToDb(m_items.m_strPath, g_windowManager.GetActiveWindow());
}


