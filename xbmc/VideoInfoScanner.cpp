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

#include "FileItem.h"
#include "VideoInfoScanner.h"
#include "addons/AddonManager.h"
#include "FileSystem/DirectoryCache.h"
#include "Util.h"
#include "NfoFile.h"
#include "utils/RegExp.h"
#include "utils/md5.h"
#include "Picture.h"
#include "FileSystem/StackDirectory.h"
#include "utils/IMDB.h"
#include "utils/GUIInfoManager.h"
#include "FileSystem/File.h"
#include "GUIDialogProgress.h"
#include "GUIDialogYesNo.h"
#include "GUIDialogOK.h"
#include "AdvancedSettings.h"
#include "GUISettings.h"
#include "Settings.h"
#include "StringUtils.h"
#include "LocalizeStrings.h"
#include "utils/TimeUtils.h"
#include "utils/log.h"

using namespace std;
using namespace XFILE;
using namespace ADDON;

namespace VIDEO
{

  CVideoInfoScanner::CVideoInfoScanner()
  {
    m_bRunning = false;
    m_pObserver = NULL;
    m_bCanInterrupt = false;
    m_currentItem = 0;
    m_itemCount = 0;
    m_bClean = false;
  }

  CVideoInfoScanner::~CVideoInfoScanner()
  {
  }

  void CVideoInfoScanner::Process()
  {
    try
    {
      unsigned int tick = CTimeUtils::GetTimeMS();

      m_database.Open();

      if (m_pObserver)
        m_pObserver->OnStateChanged(PREPARING);

      m_bCanInterrupt = true;

      CLog::Log(LOGNOTICE, "VideoInfoScanner: Starting scan ..");

      // Reset progress vars
      m_currentItem = 0;
      m_itemCount = -1;

      SetPriority(GetMinPriority());

      // Database operations should not be canceled
      // using Interupt() while scanning as it could
      // result in unexpected behaviour.
      m_bCanInterrupt = false;

      bool bCancelled = false;
      while (!bCancelled && m_pathsToScan.size())
      {
        /*
         * A copy of the directory path is used because the path supplied is
         * immediately removed from the m_pathsToScan set in DoScan(). If the
         * reference points to the entry in the set a null reference error
         * occurs.
         */
        CStdString directory = *m_pathsToScan.begin();
        if (!DoScan(directory))
          bCancelled = true;
      }

      if (!bCancelled)
      {
        if (m_bClean)
          m_database.CleanDatabase(m_pObserver,&m_pathsToClean);
        else
        {
          if (m_pObserver)
            m_pObserver->OnStateChanged(COMPRESSING_DATABASE);
          m_database.Compress(false);
        }
      }

      m_database.Close();

      tick = CTimeUtils::GetTimeMS() - tick;
      CStdString strTime;
      StringUtils::SecondsToTimeString(tick / 1000, strTime);
      CLog::Log(LOGNOTICE, "VideoInfoScanner: Finished scan. Scanning for video info took %s", strTime.c_str());

      m_bRunning = false;
      if (m_pObserver)
        m_pObserver->OnFinished();
    }
    catch (...)
    {
      CLog::Log(LOGERROR, "VideoInfoScanner: Exception while scanning.");
    }
  }

  void CVideoInfoScanner::Start(const CStdString& strDirectory, bool bUpdateAll)
  {
    m_strStartDir = strDirectory;
    m_bUpdateAll = bUpdateAll;
    m_pathsToScan.clear();
    m_pathsToClean.clear();

    if (strDirectory.IsEmpty())
    { // scan all paths in the database.  We do this by scanning all paths in the db, and crossing them off the list as
      // we go.
      m_database.Open();
      m_database.GetPaths(m_pathsToScan);
      m_bClean = g_advancedSettings.m_bVideoLibraryCleanOnUpdate;
      m_database.Close();
    }
    else
    {
      m_pathsToScan.insert(strDirectory);
      m_bClean = false;
    }

    StopThread();
    Create();
    m_bRunning = true;
  }

  bool CVideoInfoScanner::IsScanning()
  {
    return m_bRunning;
  }

  void CVideoInfoScanner::Stop()
  {
    if (m_bCanInterrupt)
      m_database.Interupt();

    StopThread();
  }

  void CVideoInfoScanner::SetObserver(IVideoInfoScannerObserver* pObserver)
  {
    m_pObserver = pObserver;
  }

  bool CVideoInfoScanner::DoScan(const CStdString& strDirectory)
  {
    if (m_bUpdateAll)
    {
      if (m_pObserver)
        m_pObserver->OnStateChanged(REMOVING_OLD);

      m_database.RemoveContentForPath(strDirectory);
    }

    if (m_pObserver)
    {
      m_pObserver->OnDirectoryChanged(strDirectory);
      m_pObserver->OnSetTitle(g_localizeStrings.Get(20415));
    }

    /*
     * Remove this path from the list we're processing. This must be done prior to
     * the check for file or folder exclusion to prevent an infinite while loop
     * in Process().
     */
    set<CStdString>::iterator it = m_pathsToScan.find(strDirectory);
    if (it != m_pathsToScan.end())
      m_pathsToScan.erase(it);

    // load subfolder
    CFileItemList items;
    bool foundDirectly = false;
    bool bSkip = false;

    SScanSettings settings;
    ScraperPtr info = m_database.GetScraperForPath(strDirectory, settings, foundDirectly);
    CONTENT_TYPE content = info ? info->Content() : CONTENT_NONE;

    // exclude folders that match our exclude regexps
    CStdStringArray regexps = content == CONTENT_TVSHOWS ? g_advancedSettings.m_tvshowExcludeFromScanRegExps
                                                         : g_advancedSettings.m_moviesExcludeFromScanRegExps;

    if (CUtil::ExcludeFileOrFolder(strDirectory, regexps))
      return true;

    if (content == CONTENT_NONE)
      bSkip = true;

    CStdString hash, dbHash;
    if ((content == CONTENT_MOVIES ||content == CONTENT_MUSICVIDEOS) && !settings.noupdate)
    {
      if (m_pObserver)
        m_pObserver->OnStateChanged(content == CONTENT_MOVIES ? FETCHING_MOVIE_INFO : FETCHING_MUSICVIDEO_INFO);

      CStdString fastHash = GetFastHash(strDirectory);
      if (m_database.GetPathHash(strDirectory, dbHash) && fastHash == dbHash)
      { // fast hashes match - no need to process anything
        CLog::Log(LOGDEBUG, "VideoInfoScanner: Skipping dir '%s' due to no change", strDirectory.c_str());
        hash = fastHash;
        bSkip = true;
      }
      if (!bSkip)
      { // need to fetch the folder
        CDirectory::GetDirectory(strDirectory, items, g_settings.m_videoExtensions);
        if (content == CONTENT_MOVIES)
          items.Stack();
        // compute hash
        GetPathHash(items, hash);
        if (hash != dbHash)
        {
          if (dbHash.IsEmpty())
            CLog::Log(LOGDEBUG, "VideoInfoScanner: Scanning dir '%s' as not in the database", strDirectory.c_str());
          else
            CLog::Log(LOGDEBUG, "VideoInfoScanner: Rescanning dir '%s' due to change (%s != %s)", strDirectory.c_str(), dbHash.c_str(), hash.c_str());
        }
        else
        { // they're the same
          CLog::Log(LOGDEBUG, "VideoInfoScanner: Skipping dir '%s' due to no change", strDirectory.c_str());
          bSkip = true;
          if (m_pObserver)
            m_pObserver->OnDirectoryScanned(strDirectory);
        }
        // update the hash to a fast hash if needed
        if (CanFastHash(items) && !fastHash.IsEmpty())
          hash = fastHash;
      }
    }
    else if (content == CONTENT_TVSHOWS && !settings.noupdate)
    {
      if (m_pObserver)
        m_pObserver->OnStateChanged(FETCHING_TVSHOW_INFO);

      if (foundDirectly && !settings.parent_name_root)
      {
        CDirectory::GetDirectory(strDirectory, items, g_settings.m_videoExtensions);
        items.m_strPath = strDirectory;
        GetPathHash(items, hash);
        bSkip = true;
        if (!m_database.GetPathHash(strDirectory, dbHash) || dbHash != hash)
        {
          m_database.SetPathHash(strDirectory, hash);
          bSkip = false;
        }
        else
          items.Clear();
      }
      else
      {
        CFileItemPtr item(new CFileItem(CUtil::GetFileName(strDirectory)));
        item->m_strPath = strDirectory;
        item->m_bIsFolder = true;
        items.Add(item);
        CUtil::GetParentPath(item->m_strPath, items.m_strPath);
      }
    }

    if (!bSkip)
    {
      if (RetrieveVideoInfo(items, settings.parent_name_root,content))
      {
        if (!m_bStop && (content == CONTENT_MOVIES || content == CONTENT_MUSICVIDEOS))
        {
          m_database.SetPathHash(strDirectory, hash);
          m_pathsToClean.push_back(m_database.GetPathId(strDirectory));
          CLog::Log(LOGDEBUG, "VideoInfoScanner: Finished adding information from dir %s", strDirectory.c_str());
        }
      }
      else
      {
        m_pathsToClean.push_back(m_database.GetPathId(strDirectory));
        CLog::Log(LOGDEBUG, "VideoInfoScanner: No (new) information was found in dir %s", strDirectory.c_str());
      }
    }
    else if (hash != dbHash && (content == CONTENT_MOVIES || content == CONTENT_MUSICVIDEOS))
    { // update the hash either way - we may have changed the hash to a fast version
      m_database.SetPathHash(strDirectory, hash);
    }

    if (m_pObserver)
      m_pObserver->OnDirectoryScanned(strDirectory);

    for (int i = 0; i < items.Size(); ++i)
    {
      CFileItemPtr pItem = items[i];

      if (m_bStop)
        break;

      // if we have a directory item (non-playlist) we then recurse into that folder
      // do not recurse for tv shows - we have already looked recursively for episodes
      if (pItem->m_bIsFolder && !pItem->IsParentFolder() && !pItem->IsPlayList() && settings.recurse > 0 && content != CONTENT_TVSHOWS)
      {
        if (!DoScan(pItem->m_strPath))
        {
          m_bStop = true;
        }
      }
    }
    return !m_bStop;
  }

  bool CVideoInfoScanner::RetrieveVideoInfo(CFileItemList& items, bool bDirNames, CONTENT_TYPE content, bool useLocal, CScraperUrl* pURL, bool fetchEpisodes, CGUIDialogProgress* pDlgProgress)
  {
    if (pDlgProgress)
    {
      if (items.Size() > 1 || (items[0]->m_bIsFolder && fetchEpisodes))
      {
        pDlgProgress->ShowProgressBar(true);
        pDlgProgress->SetPercentage(0);
      }
      else
        pDlgProgress->ShowProgressBar(false);

      pDlgProgress->Progress();
    }

    m_database.Open();
    // needed to ensure the movie count etc is cached
    for (int i=LIBRARY_HAS_VIDEO;i<LIBRARY_HAS_MUSICVIDEOS+1;++i)
      g_infoManager.GetBool(i);

    bool FoundSomeInfo = false;
    for (int i = 0; i < (int)items.Size(); ++i)
    {
      m_nfoReader.Close();
      CFileItemPtr pItem = items[i];

      // we do this since we may have a override per dir
      ScraperPtr info2 = m_database.GetScraperForPath(pItem->m_bIsFolder ? pItem->m_strPath : items.m_strPath);
      if (!info2) // skip
        continue;

      // Discard all exclude files defined by regExExclude
      if (CUtil::ExcludeFileOrFolder(pItem->m_strPath, (content == CONTENT_TVSHOWS) ? g_advancedSettings.m_tvshowExcludeFromScanRegExps
                                                                                    : g_advancedSettings.m_moviesExcludeFromScanRegExps))
        continue;

      if (info2->Content() == CONTENT_MOVIES || info2->Content() == CONTENT_MUSICVIDEOS)
      {
        if (m_pObserver)
        {
          m_pObserver->OnSetCurrentProgress(i, items.Size());
          if (!pItem->m_bIsFolder && m_itemCount)
            m_pObserver->OnSetProgress(m_currentItem++, m_itemCount);
        }

      }
      INFO_RET ret = INFO_CANCELLED;
      if (info2->Content() == CONTENT_TVSHOWS)
        ret = RetreiveInfoForTvShow(pItem, bDirNames, info2, useLocal, pURL, fetchEpisodes, pDlgProgress);
      else if (info2->Content() == CONTENT_MOVIES)
        ret = RetreiveInfoForMovie(pItem, bDirNames, info2, useLocal, pURL, pDlgProgress);
      else if (info2->Content() == CONTENT_MUSICVIDEOS)
        ret = RetreiveInfoForMusicVideo(pItem, bDirNames, info2, useLocal, pURL, pDlgProgress);
      else
      {
        CLog::Log(LOGERROR, "VideoInfoScanner: Unknown content type %d (%s)", info2->Content(), pItem->m_strPath.c_str());
        FoundSomeInfo = false;
        break;
      }
      if (ret == INFO_CANCELLED || ret == INFO_ERROR)
      {
        FoundSomeInfo = false;
        break;
      }
      if (ret == INFO_ADDED || ret == INFO_HAVE_ALREADY)
        FoundSomeInfo = true;

      pURL = NULL;
    }

    if(pDlgProgress)
      pDlgProgress->ShowProgressBar(false);

    g_infoManager.ResetPersistentCache();
    m_database.Close();
    return FoundSomeInfo;
  }

  INFO_RET CVideoInfoScanner::RetreiveInfoForTvShow(CFileItemPtr pItem, bool bDirNames, ScraperPtr &info2, bool useLocal, CScraperUrl* pURL, bool fetchEpisodes, CGUIDialogProgress* pDlgProgress)
  {
    long idTvShow = -1;
    if (pItem->m_bIsFolder)
      idTvShow = m_database.GetTvShowId(pItem->m_strPath);
    else
    {
      CStdString strPath;
      CUtil::GetDirectory(pItem->m_strPath,strPath);
      idTvShow = m_database.GetTvShowId(strPath);
    }
    if (idTvShow > -1 && (fetchEpisodes || !pItem->m_bIsFolder))
    {
      INFO_RET ret = RetrieveInfoForEpisodes(pItem, idTvShow, info2, pDlgProgress);
      if (ret == INFO_ADDED)
        m_database.SetPathHash(pItem->m_strPath, pItem->GetProperty("hash"));
      return ret;
    }

    if (ProgressCancelled(pDlgProgress, pItem->m_bIsFolder ? 20353 : 20361, pItem->GetLabel()))
      return INFO_CANCELLED;

    CNfoFile::NFOResult result=CNfoFile::NO_NFO;
    CScraperUrl scrUrl;
    // handle .nfo files
    if (useLocal)
      result = CheckForNFOFile(pItem.get(), bDirNames, info2, scrUrl);
    if (result != CNfoFile::NO_NFO && result != CNfoFile::ERROR_NFO)
    { // check for preconfigured scraper; if found, overwrite with interpreted scraper (from Nfofile)
      // but keep current scan settings
      SScanSettings settings;
      if (m_database.GetScraperForPath(pItem->m_strPath, settings))
        m_database.SetScraperForPath(pItem->m_strPath, info2, settings);
    }
    if (result == CNfoFile::FULL_NFO)
    {
      pItem->GetVideoInfoTag()->Reset();
      m_nfoReader.GetDetails(*pItem->GetVideoInfoTag());
      if (m_pObserver)
        m_pObserver->OnSetTitle(pItem->GetVideoInfoTag()->m_strTitle);

      long lResult = AddVideo(pItem.get(), info2->Content());
      if (lResult < 0)
        return INFO_ERROR;
      GetArtwork(pItem.get(), info2->Content(), bDirNames, fetchEpisodes, pDlgProgress);
      if (!fetchEpisodes && g_guiSettings.GetBool("videolibrary.seasonthumbs"))
        FetchSeasonThumbs(lResult);
      if (fetchEpisodes)
      {
        INFO_RET ret = RetrieveInfoForEpisodes(pItem, lResult, info2, pDlgProgress);
        if (ret == INFO_ADDED)
          m_database.SetPathHash(pItem->m_strPath, pItem->GetProperty("hash"));
        return ret;
      }
      return INFO_ADDED;
    }
    if (result == CNfoFile::URL_NFO || result == CNfoFile::COMBINED_NFO)
      pURL = &scrUrl;

    CScraperUrl url;
    int retVal = 0;
    if (pURL)
      url = *pURL;
    else if ((retVal = FindVideo(pItem->GetMovieName(bDirNames), info2, url, pDlgProgress)) <= 0)
      return retVal < 0 ? INFO_CANCELLED : INFO_NOT_FOUND;

    if (m_pObserver && !url.strTitle.IsEmpty())
      m_pObserver->OnSetTitle(url.strTitle);

    long lResult=-1;
    if (GetDetails(pItem.get(), url, info2, result == CNfoFile::COMBINED_NFO ? &m_nfoReader : NULL, pDlgProgress))
    {
      if ((lResult = AddVideo(pItem.get(), info2->Content())) < 0)
        return INFO_ERROR;
      GetArtwork(pItem.get(), info2->Content(), false, useLocal);
    }
    if (fetchEpisodes)
    {
      INFO_RET ret = RetrieveInfoForEpisodes(pItem, lResult, info2, pDlgProgress);
      if (ret == INFO_ADDED)
        m_database.SetPathHash(pItem->m_strPath, pItem->GetProperty("hash"));
    }
    else
      if (g_guiSettings.GetBool("videolibrary.seasonthumbs"))
        FetchSeasonThumbs(lResult);
    return INFO_ADDED;
  }

  INFO_RET CVideoInfoScanner::RetreiveInfoForMovie(CFileItemPtr pItem, bool bDirNames, ScraperPtr &info2, bool useLocal, CScraperUrl* pURL, CGUIDialogProgress* pDlgProgress)
  {
    if (pItem->m_bIsFolder || !pItem->IsVideo() || pItem->IsNFO() || pItem->IsPlayList())
      return INFO_NOT_NEEDED;

    if (ProgressCancelled(pDlgProgress, 198, pItem->GetLabel()))
      return INFO_CANCELLED;

    if (m_database.HasMovieInfo(pItem->m_strPath))
      return INFO_HAVE_ALREADY;

    CNfoFile::NFOResult result=CNfoFile::NO_NFO;
    CScraperUrl scrUrl;
    // handle .nfo files
    if (useLocal)
      result = CheckForNFOFile(pItem.get(), bDirNames, info2, scrUrl);
    if (result == CNfoFile::FULL_NFO)
    {
      pItem->GetVideoInfoTag()->Reset();
      m_nfoReader.GetDetails(*pItem->GetVideoInfoTag());
      if (m_pObserver)
        m_pObserver->OnSetTitle(pItem->GetVideoInfoTag()->m_strTitle);

      if (AddVideo(pItem.get(), info2->Content()) < 0)
        return INFO_ERROR;
      GetArtwork(pItem.get(), info2->Content(), bDirNames, true, pDlgProgress);
      return INFO_ADDED;
    }
    if (result == CNfoFile::URL_NFO || result == CNfoFile::COMBINED_NFO)
      pURL = &scrUrl;

    CScraperUrl url;
    int retVal = 0;
    if (pURL)
      url = *pURL;
    else if ((retVal = FindVideo(pItem->GetMovieName(bDirNames), info2, url, pDlgProgress)) <= 0)
      return retVal < 0 ? INFO_CANCELLED : INFO_NOT_FOUND;

    if (m_pObserver && !url.strTitle.IsEmpty())
      m_pObserver->OnSetTitle(url.strTitle);

    if (GetDetails(pItem.get(), url, info2, result == CNfoFile::COMBINED_NFO ? &m_nfoReader : NULL, pDlgProgress))
    {
      if (AddVideo(pItem.get(), info2->Content()) < 0)
        return INFO_ERROR;
      GetArtwork(pItem.get(), info2->Content(), bDirNames, useLocal);
      return INFO_ADDED;
    }
    // TODO: This is not strictly correct as we could fail to download information here or error, or be cancelled
    return INFO_NOT_FOUND;
  }

  INFO_RET CVideoInfoScanner::RetreiveInfoForMusicVideo(CFileItemPtr pItem, bool bDirNames, ScraperPtr &info2, bool useLocal, CScraperUrl* pURL, CGUIDialogProgress* pDlgProgress)
  {
    if (pItem->m_bIsFolder || !pItem->IsVideo() || pItem->IsNFO() || pItem->IsPlayList())
      return INFO_NOT_NEEDED;

    if (ProgressCancelled(pDlgProgress, 20394, pItem->GetLabel()))
      return INFO_CANCELLED;

    if (m_database.HasMusicVideoInfo(pItem->m_strPath))
      return INFO_HAVE_ALREADY;

    CNfoFile::NFOResult result=CNfoFile::NO_NFO;
    CScraperUrl scrUrl;
    // handle .nfo files
    if (useLocal)
      result = CheckForNFOFile(pItem.get(), bDirNames, info2, scrUrl);
    if (result == CNfoFile::FULL_NFO)
    {
      pItem->GetVideoInfoTag()->Reset();
      m_nfoReader.GetDetails(*pItem->GetVideoInfoTag());
      if (m_pObserver)
        m_pObserver->OnSetTitle(pItem->GetVideoInfoTag()->m_strTitle);

      if (AddVideo(pItem.get(), info2->Content()) < 0)
        return INFO_ERROR;
      GetArtwork(pItem.get(), info2->Content(), bDirNames, true, pDlgProgress);
      return INFO_ADDED;
    }
    if (result == CNfoFile::URL_NFO || result == CNfoFile::COMBINED_NFO)
      pURL = &scrUrl;

    CScraperUrl url;
    int retVal = 0;
    if (pURL)
      url = *pURL;
    else if ((retVal = FindVideo(pItem->GetMovieName(bDirNames), info2, url, pDlgProgress)) <= 0)
      return retVal < 0 ? INFO_CANCELLED : INFO_NOT_FOUND;

    if (m_pObserver && !url.strTitle.IsEmpty())
      m_pObserver->OnSetTitle(url.strTitle);

    if (GetDetails(pItem.get(), url, info2, result == CNfoFile::COMBINED_NFO ? &m_nfoReader : NULL, pDlgProgress))
    {
      if (AddVideo(pItem.get(), info2->Content()) < 0)
        return INFO_ERROR;
      GetArtwork(pItem.get(), info2->Content(), bDirNames, useLocal);
      return INFO_ADDED;
    }
    // TODO: This is not strictly correct as we could fail to download information here or error, or be cancelled
    return INFO_NOT_FOUND;
  }

  INFO_RET CVideoInfoScanner::RetrieveInfoForEpisodes(CFileItemPtr item, long showID, const ADDON::ScraperPtr &scraper, CGUIDialogProgress *progress)
  {
    IMDB_EPISODELIST episodes;
    EPISODES files;
    // fetch episode guide
    CVideoInfoTag details;
    m_database.GetTvShowInfo(item->m_strPath, details, showID);
    if (!details.m_strEpisodeGuide.IsEmpty()) // assume local-only series if no episode guide url
    {
      CScraperUrl url;
      url.ParseEpisodeGuide(details.m_strEpisodeGuide);

      EnumerateSeriesFolder(item.get(), files);
      if (files.size() == 0) // no update or no files
        return INFO_NOT_NEEDED;

      if (progress)
      {
        if (item->m_bIsFolder)
          progress->SetHeading(20353);
        else
          progress->SetHeading(20361);
        progress->SetLine(0, item->GetLabel());
        progress->SetLine(1, details.m_strTitle);
        progress->SetLine(2, 20354);
        progress->Progress();
      }

      CIMDB imdb(scraper);
      if (!imdb.GetEpisodeList(url, episodes))
        return INFO_NOT_FOUND;
    }

    if (m_bStop || (progress && progress->IsCanceled()))
      return INFO_CANCELLED;

    if (m_pObserver)
      m_pObserver->OnDirectoryChanged(item->m_strPath);

    return OnProcessSeriesFolder(episodes, files, scraper, showID, details.m_strTitle, progress);
  }

  void CVideoInfoScanner::EnumerateSeriesFolder(CFileItem* item, EPISODES& episodeList)
  {
    CFileItemList items;

    if (item->m_bIsFolder)
    {
      CUtil::GetRecursiveListing(item->m_strPath, items, g_settings.m_videoExtensions, true);
      CStdString hash, dbHash;
      int numFilesInFolder = GetPathHash(items, hash);

      if (m_database.GetPathHash(item->m_strPath, dbHash) && dbHash == hash)
      {
        m_currentItem += numFilesInFolder;

        // notify our observer of our progress
        if (m_pObserver)
        {
          if (m_itemCount>0)
          {
            m_pObserver->OnSetProgress(m_currentItem, m_itemCount);
            m_pObserver->OnSetCurrentProgress(numFilesInFolder, numFilesInFolder);
          }
          m_pObserver->OnDirectoryScanned(item->m_strPath);
        }
        return;
      }
      m_pathsToClean.push_back(m_database.GetPathId(item->m_strPath));
      m_database.GetPathsForTvShow(m_database.GetTvShowId(item->m_strPath), m_pathsToClean);
      item->SetProperty("hash", hash);
    }
    else
    {
      CFileItemPtr newItem(new CFileItem(*item));
      items.Add(newItem);
    }

    /*
    stack down any dvd folders
    need to sort using the full path since this is a collapsed recursive listing of all subdirs
    video_ts.ifo files should sort at the top of a dvd folder in ascending order

    /foo/bar/video_ts.ifo
    /foo/bar/vts_x_y.ifo
    /foo/bar/vts_x_y.vob
    */

    // since we're doing this now anyway, should other items be stacked?
    items.Sort(SORT_METHOD_FULLPATH, SORT_ORDER_ASC);
    int x = 0;
    while (x < items.Size())
    {
      if (items[x]->m_bIsFolder)
        continue;


      CStdString strPathX, strFileX;
      CUtil::Split(items[x]->m_strPath, strPathX, strFileX);
      //CLog::Log(LOGDEBUG,"%i:%s:%s", x, strPathX.c_str(), strFileX.c_str());

      int y = x + 1;
      if (strFileX.Equals("VIDEO_TS.IFO"))
      {
        while (y < items.Size())
        {
          CStdString strPathY, strFileY;
          CUtil::Split(items[y]->m_strPath, strPathY, strFileY);
          //CLog::Log(LOGDEBUG," %i:%s:%s", y, strPathY.c_str(), strFileY.c_str());

          if (strPathY.Equals(strPathX))
            /*
            remove everything sorted below the video_ts.ifo file in the same path.
            understandbly this wont stack correctly if there are other files in the the dvd folder.
            this should be unlikely and thus is being ignored for now but we can monitor the
            where the path changes and potentially remove the items above the video_ts.ifo file.
            */
            items.Remove(y);
          else
            break;
        }
      }
      x = y;
    }

    // enumerate
    SETTINGS_TVSHOWLIST expression = g_advancedSettings.m_tvshowStackRegExps;
    CStdStringArray regexps = g_advancedSettings.m_tvshowExcludeFromScanRegExps;

    for (int i=0;i<items.Size();++i)
    {
      if (items[i]->m_bIsFolder)
        continue;
      CStdString strPath;
      CUtil::GetDirectory(items[i]->m_strPath, strPath);
      CUtil::RemoveSlashAtEnd(strPath); // want no slash for the test that follows

      if (CUtil::GetFileName(strPath).Equals("sample"))
        continue;

      // Discard all exclude files defined by regExExcludes
      if (CUtil::ExcludeFileOrFolder(items[i]->m_strPath, regexps))
        continue;

      bool bMatched=false;
      for (unsigned int j=0;j<expression.size();++j)
      {
        if (expression[j].byDate)
        {
          bMatched = ProcessItemByDate(items[i], episodeList, expression[j].regexp);
        }
        else
        {
          bMatched = ProcessItemNormal(items[i], episodeList, expression[j].regexp);
        }
        if (bMatched)
          break;
      }

      if (!bMatched)
        CLog::Log(LOGDEBUG, "VideoInfoScanner: Could not enumerate file %s", items[i]->m_strPath.c_str());

    }
  }

  bool CVideoInfoScanner::ProcessItemNormal(CFileItemPtr item, EPISODES &episodeList, CStdString regexp)
  {
    CRegExp reg;
    if (!reg.RegComp(regexp))
      return false;

    CStdString strLabel=item->m_strPath;
    strLabel.MakeLower();
//    CLog::Log(LOGDEBUG,"running expression %s on label %s",regexp.c_str(),strLabel.c_str());
    int regexppos, regexp2pos;

    if ((regexppos = reg.RegFind(strLabel.c_str())) < 0)
      return false;


    SEpisode episode;
    episode.strPath = item->m_strPath;
    episode.cDate.SetValid(false);
    if (!GetEpisodeAndSeasonFromRegExp(reg, episode))
      return false;

    CLog::Log(LOGDEBUG, "VideoInfoScanner: Found episode match %s (s%ie%i) [%s]", strLabel.c_str(), episode.iSeason, episode.iEpisode, regexp.c_str());
    episodeList.push_back(episode);

    // check the remainder of the string for any further episodes.
    CRegExp reg2;
    if (!reg2.RegComp(g_advancedSettings.m_tvshowMultiPartStackRegExp))
      return true;

    char *remainder = reg.GetReplaceString("\\3");
    int offset = 0;

    // we want "long circuit" OR below so that both offsets are evaluated
    while (((regexp2pos = reg2.RegFind(remainder + offset)) > -1) | ((regexppos = reg.RegFind(remainder + offset)) > -1))
    {
      if (((regexppos <= regexp2pos) && regexppos != -1) ||
         (regexppos >= 0 && regexp2pos == -1))
      {
        GetEpisodeAndSeasonFromRegExp(reg, episode);
        CLog::Log(LOGDEBUG, "VideoInfoScanner: Adding new season %u, multipart episode %u", episode.iSeason, episode.iEpisode);
        episodeList.push_back(episode);
        free(remainder);
        remainder = reg.GetReplaceString("\\3");
        offset = 0;
      }
      else if (((regexp2pos < regexppos) && regexp2pos != -1) ||
               (regexp2pos >= 0 && regexppos == -1))
      {
        char *ep = reg2.GetReplaceString("\\1");
        episode.iEpisode = atoi(ep);
        free(ep);
        CLog::Log(LOGDEBUG, "VideoInfoScanner: Adding multipart episode %u", episode.iEpisode);
        episodeList.push_back(episode);
        offset += regexp2pos + reg2.GetFindLen();
      }
    }
    free(remainder);
    return true;
  }

  bool CVideoInfoScanner::GetEpisodeAndSeasonFromRegExp(CRegExp &reg, SEpisode &episodeInfo)
  {
    char* season = reg.GetReplaceString("\\1");
    char* episode = reg.GetReplaceString("\\2");

    if (season && episode)
    {
      if (strlen(season) == 0 && strlen(episode) > 0)
      { // no season specified -> assume season 1
        episodeInfo.iSeason = 1;
        if ((episodeInfo.iEpisode = CUtil::TranslateRomanNumeral(episode)) == -1)
          episodeInfo.iEpisode = atoi(episode);
      }
      else if (strlen(season) > 0 && strlen(episode) == 0)
      { // no episode specification -> assume season 1
        episodeInfo.iSeason = 1;
        if ((episodeInfo.iEpisode = CUtil::TranslateRomanNumeral(season)) == -1)
          episodeInfo.iEpisode = atoi(season);
      }
      else
      { // season and episode specified
        episodeInfo.iSeason = atoi(season);
        episodeInfo.iEpisode = atoi(episode);
      }
    }
    free(season);
    free(episode);
    return (season && episode);
  }

  bool CVideoInfoScanner::ProcessItemByDate(CFileItemPtr item, EPISODES &episodeList, CStdString regexp)
  {
    CRegExp reg;
    if (!reg.RegComp(regexp))
      return false;

    CStdString strLabel=item->m_strPath;
    strLabel.MakeLower();
//    CLog::Log(LOGDEBUG,"running expression %s on label %s",regexp.c_str(),strLabel.c_str());
    int regexppos;

    if ((regexppos = reg.RegFind(strLabel.c_str())) < 0)
      return false;

    bool bMatched = false;

    char* param1 = reg.GetReplaceString("\\1");
    char* param2 = reg.GetReplaceString("\\2");
    char* param3 = reg.GetReplaceString("\\3");
    if (param1 && param2 && param3)
    {
      // regular expression by date
      int len1 = strlen( param1 );
      int len2 = strlen( param2 );
      int len3 = strlen( param3 );
      char* day;
      char* month;
      char* year;
      if (len1==4 && len2==2 && len3==2)
      {
        // yyyy mm dd format
        bMatched = true;
        year = param1;
        month = param2;
        day = param3;
      }
      else if (len1==2 && len2==2 && len3==4)
      {
        // mm dd yyyy format
        bMatched = true;
        year = param3;
        month = param1;
        day = param2;
      }
      if (bMatched)
      {
        CLog::Log(LOGDEBUG, "VideoInfoScanner: Found date based match %s (Y%sm=%sd=%s) [%s]", strLabel.c_str(), year, month, day, regexp.c_str());
        SEpisode myEpisode;
        myEpisode.strPath = item->m_strPath;
        myEpisode.iSeason = -1;
        myEpisode.iEpisode = -1;
        myEpisode.cDate.SetDate(atoi(year), atoi(month), atoi(day));
        episodeList.push_back(myEpisode);
      }
    }
    free(param1);
    free(param2);
    free(param3);
    return bMatched;
  }

  long CVideoInfoScanner::AddVideo(CFileItem *pItem, const CONTENT_TYPE &content, int idShow)
  {
    // ensure our database is open (this can get called via other classes)
    if (!m_database.Open())
      return -1;

    CLog::Log(LOGDEBUG, "VideoInfoScanner: Adding new item to %s:%s", TranslateContent(content).c_str(), pItem->m_strPath.c_str());
    long lResult = -1;

    CVideoInfoTag &movieDetails = *pItem->GetVideoInfoTag();
    if (content == CONTENT_MOVIES)
    {
      // find local trailer first
      CStdString strTrailer = pItem->FindTrailer();
      if (!strTrailer.IsEmpty())
        movieDetails.m_strTrailer = strTrailer;

      lResult = m_database.SetDetailsForMovie(pItem->m_strPath, movieDetails);

      // setup links to shows if the linked shows are in the db
      if (!movieDetails.m_strShowLink.IsEmpty())
      {
        CStdStringArray list;
        StringUtils::SplitString(movieDetails.m_strShowLink, g_advancedSettings.m_videoItemSeparator,list);
        for (unsigned int i=0; i < list.size(); ++i)
        {
          CFileItemList items;
          m_database.GetTvShowsByName(list[i], items);
          if (items.Size())
            m_database.LinkMovieToTvshow(lResult, items[0]->GetVideoInfoTag()->m_iDbId, false);
          else
            CLog::Log(LOGDEBUG, "VideoInfoScanner: Failed to link movie %s to show %s", movieDetails.m_strTitle.c_str(), list[i].c_str());
        }
      }
    }
    else if (content == CONTENT_TVSHOWS)
    {
      if (pItem->m_bIsFolder)
      {
        lResult=m_database.SetDetailsForTvShow(pItem->m_strPath, movieDetails);
      }
      else
      {
        // we add episode then set details, as otherwise set details will delete the
        // episode then add, which breaks multi-episode files.
        int idEpisode = m_database.AddEpisode(idShow, pItem->m_strPath);
        lResult = m_database.SetDetailsForEpisode(pItem->m_strPath, movieDetails, idShow, idEpisode);
        if (movieDetails.m_fEpBookmark > 0)
        {
          movieDetails.m_strFileNameAndPath = pItem->m_strPath;
          CBookmark bookmark;
          bookmark.timeInSeconds = movieDetails.m_fEpBookmark;
          bookmark.seasonNumber = movieDetails.m_iSeason;
          bookmark.episodeNumber = movieDetails.m_iEpisode;
          m_database.AddBookMarkForEpisode(movieDetails, bookmark);
        }
      }
    }
    else if (content == CONTENT_MUSICVIDEOS)
    {
      lResult = m_database.SetDetailsForMusicVideo(pItem->m_strPath, movieDetails);
    }

    if (g_advancedSettings.m_bVideoLibraryImportWatchedState)
      m_database.SetPlayCount(*pItem, movieDetails.m_playCount, movieDetails.m_lastPlayed);

    m_database.Close();
    return lResult;
  }

  void CVideoInfoScanner::GetArtwork(CFileItem *pItem, const CONTENT_TYPE &content, bool bApplyToDir, bool useLocal, CGUIDialogProgress* pDialog /* == NULL */)
  {
    CVideoInfoTag &movieDetails = *pItem->GetVideoInfoTag();
    // get & save fanart image
    if (!useLocal || !pItem->CacheLocalFanart())
    {
      if (movieDetails.m_fanart.GetNumFanarts())
        DownloadImage(movieDetails.m_fanart.GetImageURL(), pItem->GetCachedFanart(), false, pDialog);
    }

    // get & save thumb image
    CStdString cachedThumb = pItem->GetCachedVideoThumb();
    if (content == CONTENT_TVSHOWS && !pItem->m_bIsFolder && CFile::Exists(cachedThumb))
    { // have an episode (??? and also a normal "cached" thumb that we're going to override now???)
      movieDetails.m_strFileNameAndPath = pItem->m_strPath;
      CFileItem item(movieDetails);
      cachedThumb = item.GetCachedEpisodeThumb();
    }

    CStdString localThumb;
    if (useLocal)
    {
      localThumb = pItem->GetUserVideoThumb();
      if (bApplyToDir && localThumb.IsEmpty())
      {
        CStdString strParent;
        CUtil::GetParentPath(pItem->m_strPath, strParent);
        CFileItem item(*pItem);
        item.m_strPath = strParent;
        item.m_bIsFolder = true;
        localThumb = item.GetUserVideoThumb();
      }
    }

    // parent folder to apply the thumb to and to search for local actor thumbs
    CStdString parentDir = GetParentDir(*pItem);

    if (!localThumb.IsEmpty())
      CPicture::CacheThumb(localThumb, cachedThumb);
    else
    { // see if we have an online image to use
      CStdString onlineThumb = CScraperUrl::GetThumbURL(movieDetails.m_strPictureURL.GetFirstThumb());
      if (!onlineThumb.IsEmpty())
      {
        if (onlineThumb.Find("http://") < 0 &&
            onlineThumb.Find("/") < 0 &&
            onlineThumb.Find("\\") < 0)
        {
          CStdString strPath;
          CUtil::GetDirectory(pItem->m_strPath, strPath);
          onlineThumb = CUtil::AddFileToFolder(strPath, onlineThumb);
        }
        DownloadImage(onlineThumb, cachedThumb, true, pDialog, bApplyToDir ? parentDir : "");
      }
    }
    if (g_guiSettings.GetBool("videolibrary.actorthumbs"))
      FetchActorThumbs(movieDetails.m_cast, parentDir);
  }

  void CVideoInfoScanner::DownloadImage(const CStdString &url, const CStdString &destination, bool asThumb /*= true */, CGUIDialogProgress *progress /*= NULL */, const CStdString &directory /*= "" */)
  {
    if (progress)
    {
      progress->SetLine(2, 415);
      progress->Progress();
    }
    bool result = false;
    if (asThumb)
      result = CPicture::CreateThumbnail(url, destination);
    else
      result = CPicture::CacheFanart(url, destination);
    if (!result)
    {
      CFile::Delete(destination);
      return;
    }
    if (!directory.IsEmpty())
      ApplyThumbToFolder(directory, destination);
  }

  INFO_RET CVideoInfoScanner::OnProcessSeriesFolder(IMDB_EPISODELIST& episodes, EPISODES& files, const ADDON::ScraperPtr &scraper, int idShow, const CStdString& strShowTitle, CGUIDialogProgress* pDlgProgress /* = NULL */)
  {
    if (pDlgProgress)
    {
      pDlgProgress->SetLine(2, 20361);
      pDlgProgress->SetPercentage(0);
      pDlgProgress->ShowProgressBar(true);
      pDlgProgress->Progress();
    }

    int iMax = files.size();
    int iCurr = 1;
    for (EPISODES::iterator file = files.begin(); file != files.end(); ++file)
    {
      m_nfoReader.Close();
      if (pDlgProgress)
      {
        pDlgProgress->SetLine(2, 20361);
        pDlgProgress->SetPercentage((int)((float)(iCurr++)/iMax*100));
        pDlgProgress->Progress();
      }
      if (m_pObserver)
      {
        if (m_itemCount > 0)
          m_pObserver->OnSetProgress(m_currentItem++, m_itemCount);
        m_pObserver->OnSetCurrentProgress(iCurr++, iMax);
      }
      if ((pDlgProgress && pDlgProgress->IsCanceled()) || m_bStop)
        return INFO_CANCELLED;

      if (m_database.GetEpisodeId(file->strPath, file->iEpisode, file->iSeason) > -1)
      {
        if (m_pObserver)
          m_pObserver->OnSetTitle(g_localizeStrings.Get(20415));
        continue;
      }

      CFileItem item;
      item.m_strPath = file->strPath;

      // handle .nfo files
      CScraperUrl scrUrl;
      ScraperPtr info(scraper);
      item.GetVideoInfoTag()->m_iEpisode = file->iEpisode;
      CNfoFile::NFOResult result = CheckForNFOFile(&item, false, info,scrUrl);
      if (result == CNfoFile::FULL_NFO)
      {
        m_nfoReader.GetDetails(*item.GetVideoInfoTag());
        if (m_pObserver)
        {
          CStdString strTitle;
          strTitle.Format("%s - %ix%i - %s", strShowTitle.c_str(), item.GetVideoInfoTag()->m_iSeason, item.GetVideoInfoTag()->m_iEpisode, item.GetVideoInfoTag()->m_strTitle.c_str());
          m_pObserver->OnSetTitle(strTitle);
        }
        if (AddVideo(&item, CONTENT_TVSHOWS, idShow) < 0)
          return INFO_ERROR;
        GetArtwork(&item, CONTENT_TVSHOWS);
        continue;
      }

      if (episodes.empty())
      {
        CLog::Log(LOGERROR, "VideoInfoScanner: Asked to lookup episode %s"
                            " online, but we have no episode guide. Check your tvshow.nfo and make"
                            " sure the <episodeguide> tag is in place.", file->strPath.c_str());
        continue;
      }

      std::pair<int,int> key;
      key.first = file->iSeason;
      key.second = file->iEpisode;
      bool bFound = false;
      IMDB_EPISODELIST::iterator guide = episodes.begin();;

      for (; guide != episodes.end(); ++guide )
      {
        if (file->cDate.IsValid() && guide->cDate.IsValid() && file->cDate==guide->cDate)
        {
          bFound = true;
          break;
        }
        if ((file->iEpisode!=-1) && (file->iSeason!=-1) && (key==guide->key))
        {
          bFound = true;
          break;
        }
      }

      if (bFound)
      {
        CIMDB imdb(scraper);
        CFileItem item;
        item.m_strPath = file->strPath;
        if (!imdb.GetEpisodeDetails(guide->cScraperUrl, *item.GetVideoInfoTag(), pDlgProgress))
          return INFO_NOT_FOUND; // TODO: should we just skip to the next episode?
        item.GetVideoInfoTag()->m_iSeason = guide->key.first;
        item.GetVideoInfoTag()->m_iEpisode = guide->key.second;
        if (m_pObserver)
        {
          CStdString strTitle;
          strTitle.Format("%s - %ix%i - %s", strShowTitle.c_str(), item.GetVideoInfoTag()->m_iSeason, item.GetVideoInfoTag()->m_iEpisode, item.GetVideoInfoTag()->m_strTitle.c_str());
          m_pObserver->OnSetTitle(strTitle);
        }
        if (AddVideo(&item, CONTENT_TVSHOWS, idShow) < 0)
          return INFO_ERROR;
        GetArtwork(&item, CONTENT_TVSHOWS);
      }
    }
    if (g_guiSettings.GetBool("videolibrary.seasonthumbs"))
      FetchSeasonThumbs(idShow);
    return INFO_ADDED;
  }

  CStdString CVideoInfoScanner::GetnfoFile(CFileItem *item, bool bGrabAny) const
  {
    CStdString nfoFile;
    // Find a matching .nfo file
    if (!item->m_bIsFolder)
    {
      // file
      CStdString strExtension;
      CUtil::GetExtension(item->m_strPath, strExtension);

      if (CUtil::IsInRAR(item->m_strPath)) // we have a rarred item - we want to check outside the rars
      {
        CFileItem item2(*item);
        CURL url(item->m_strPath);
        CStdString strPath;
        CUtil::GetDirectory(url.GetHostName(), strPath);
        CUtil::AddFileToFolder(strPath, CUtil::GetFileName(item->m_strPath),item2.m_strPath);
        return GetnfoFile(&item2, bGrabAny);
      }

      CStdString strPath;
      CUtil::GetDirectory(item->m_strPath, strPath);
      nfoFile = CUtil::AddFileToFolder(strPath, "movie.nfo");
      if (CFile::Exists(nfoFile))
        return nfoFile;

      // already an .nfo file?
      if ( strcmpi(strExtension.c_str(), ".nfo") == 0 )
        nfoFile = item->m_strPath;
      // no, create .nfo file
      else
        nfoFile = CUtil::ReplaceExtension(item->m_strPath, ".nfo");

      // test file existence
      if (!nfoFile.IsEmpty() && !CFile::Exists(nfoFile))
        nfoFile.Empty();

      // try looking for .nfo file for a stacked item
      if (item->IsStack())
      {
        // first try .nfo file matching first file in stack
        CStackDirectory dir;
        CStdString firstFile = dir.GetFirstStackedFile(item->m_strPath);
        CFileItem item2;
        item2.m_strPath = firstFile;
        nfoFile = GetnfoFile(&item2, bGrabAny);
        // else try .nfo file matching stacked title
        if (nfoFile.IsEmpty())
        {
          CStdString stackedTitlePath = dir.GetStackedTitlePath(item->m_strPath);
          item2.m_strPath = stackedTitlePath;
          nfoFile = GetnfoFile(&item2, bGrabAny);
        }
      }

      if (nfoFile.IsEmpty()) // final attempt - strip off any cd1 folders
      {
        CStdString strPath;
        CUtil::GetDirectory(item->m_strPath, strPath);
        CUtil::RemoveSlashAtEnd(strPath); // need no slash for the check that follows
        CFileItem item2;
        if (strPath.Mid(strPath.size()-3).Equals("cd1"))
        {
          strPath = strPath.Mid(0,strPath.size()-3);
          CUtil::AddFileToFolder(strPath, CUtil::GetFileName(item->m_strPath),item2.m_strPath);
          return GetnfoFile(&item2, bGrabAny);
        }

        // finally try mymovies.xml
        nfoFile = CUtil::AddFileToFolder(strPath, "mymovies.xml");
        if (CFile::Exists(nfoFile))
          return nfoFile;
        else
          nfoFile.clear();
      }
    }
    if (item->m_bIsFolder || (bGrabAny && nfoFile.IsEmpty()))
    {
      // see if there is a unique nfo file in this folder, and if so, use that
      CFileItemList items;
      CDirectory dir;
      CStdString strPath = item->m_strPath;
      if (!item->m_bIsFolder)
        CUtil::GetDirectory(item->m_strPath, strPath);
      if (dir.GetDirectory(strPath, items, ".nfo") && items.Size())
      {
        int numNFO = -1;
        for (int i = 0; i < items.Size(); i++)
        {
          if (items[i]->IsNFO())
          {
            if (numNFO == -1)
              numNFO = i;
            else
            {
              numNFO = -1;
              break;
            }
          }
        }
        if (numNFO > -1)
          return items[numNFO]->m_strPath;
      }
    }

    return nfoFile;
  }

  bool CVideoInfoScanner::GetDetails(CFileItem *pItem, CScraperUrl &url, const ScraperPtr& scraper, CNfoFile *nfoFile, CGUIDialogProgress* pDialog /* = NULL */)
  {
    CVideoInfoTag movieDetails;
    movieDetails.m_strFileNameAndPath = pItem->m_strPath;

    CIMDB imdb(scraper);
    if ( imdb.GetDetails(url, movieDetails, pDialog) )
    {
      if (nfoFile)
        nfoFile->GetDetails(movieDetails);

      if (m_pObserver && url.strTitle.IsEmpty())
        m_pObserver->OnSetTitle(movieDetails.m_strTitle);

      if (pDialog)
      {
        pDialog->SetLine(1, movieDetails.m_strTitle);
        pDialog->Progress();
      }

      *pItem->GetVideoInfoTag() = movieDetails;
      return true;
    }
    return false; // no info found, or cancelled
  }

  void CVideoInfoScanner::ApplyThumbToFolder(const CStdString &folder, const CStdString &imdbThumb)
  {
    // copy icon to folder also;
    if (CFile::Exists(imdbThumb))
    {
      CFileItem folderItem(folder, true);
      CStdString strThumb(folderItem.GetCachedVideoThumb());
      CFile::Cache(imdbThumb.c_str(), strThumb.c_str(), NULL, NULL);
    }
  }

  int CVideoInfoScanner::GetPathHash(const CFileItemList &items, CStdString &hash)
  {
    // Create a hash based on the filenames, filesize and filedate.  Also count the number of files
    if (0 == items.Size()) return 0;
    XBMC::XBMC_MD5 md5state;
    int count = 0;
    for (int i = 0; i < items.Size(); ++i)
    {
      const CFileItemPtr pItem = items[i];
      md5state.append(pItem->m_strPath);
      md5state.append((unsigned char *)&pItem->m_dwSize, sizeof(pItem->m_dwSize));
      FILETIME time = pItem->m_dateTime;
      md5state.append((unsigned char *)&time, sizeof(FILETIME));
      if (pItem->IsVideo() && !pItem->IsPlayList() && !pItem->IsNFO())
        count++;
    }
    md5state.getDigest(hash);
    return count;
  }

  bool CVideoInfoScanner::CanFastHash(const CFileItemList &items) const
  {
    // TODO: Probably should account for excluded folders here (eg samples), though that then
    //       introduces possible problems if the user then changes the exclude regexps and
    //       expects excluded folders that are inside a fast-hashed folder to then be picked
    //       up. The chances that the user has a folder which contains only excluded folders
    //       where some of those folders should be scanned recursively is pretty small.
    return items.GetFolderCount() == 0;
  }

  CStdString CVideoInfoScanner::GetFastHash(const CStdString &directory) const
  {
    struct __stat64 buffer;
    if (XFILE::CFile::Stat(directory, &buffer) == 0)
    {
      int64_t time = buffer.st_mtime;
      if (!time)
        time = buffer.st_ctime;
      if (time)
      {
        CStdString hash;
        hash.Format("fast%"PRId64, time);
        return hash;
      }
    }
    return "";
  }

  void CVideoInfoScanner::FetchSeasonThumbs(int idTvShow, const CStdString &folderToCheck, bool download, bool overwrite)
  {
    // ensure our database is open (this can get called via other classes)
    if (!m_database.Open())
      return;

    CVideoInfoTag movie;
    m_database.GetTvShowInfo("", movie, idTvShow);
    CStdString showDir(folderToCheck.IsEmpty() ? movie.m_strPath : folderToCheck);
    CFileItemList items;
    CStdString strPath;
    strPath.Format("videodb://2/2/%i/", idTvShow);
    m_database.GetSeasonsNav(strPath, items, -1, -1, -1, -1, idTvShow);
    CFileItemPtr pItem;
    pItem.reset(new CFileItem(g_localizeStrings.Get(20366)));  // "All Seasons"
    pItem->m_strPath.Format("%s/-1/", strPath.c_str());
    pItem->GetVideoInfoTag()->m_iSeason = -1;
    pItem->GetVideoInfoTag()->m_strPath = movie.m_strPath;
    if (overwrite || !XFILE::CFile::Exists(pItem->GetCachedSeasonThumb()))
      items.Add(pItem);

    // used for checking for a season[ ._-](number).tbn
    CFileItemList tbnItems;
    CDirectory::GetDirectory(showDir, tbnItems, ".tbn");
    for (int i=0;i<items.Size();++i)
    {
      if (overwrite || !items[i]->HasThumbnail())
      {
        CStdString strExpression;
        int iSeason = items[i]->GetVideoInfoTag()->m_iSeason;
        if (iSeason == -1)
          strExpression = "season-all.tbn";
        else if (iSeason == 0)
          strExpression = "season-specials.tbn";
        else
          strExpression.Format("season[ ._-]?(0?%i)\\.tbn", items[i]->GetVideoInfoTag()->m_iSeason);
        bool bDownload = download;
        CRegExp reg;
        if (reg.RegComp(strExpression.c_str()))
        {
          for (int j=0;j<tbnItems.Size();++j)
          {
            CStdString strCheck = CUtil::GetFileName(tbnItems[j]->m_strPath);
            strCheck.ToLower();
            if (reg.RegFind(strCheck.c_str()) > -1)
            {
              CPicture::CreateThumbnail(tbnItems[j]->m_strPath, items[i]->GetCachedSeasonThumb());
              bDownload=false;
              break;
            }
          }
        }
        if (bDownload)
          DownloadImage(CScraperUrl::GetThumbURL(movie.m_strPictureURL.GetSeasonThumb(items[i]->GetVideoInfoTag()->m_iSeason)), items[i]->GetCachedSeasonThumb());
      }
    }
    m_database.Close();
  }

  void CVideoInfoScanner::FetchActorThumbs(const vector<SActorInfo>& actors, const CStdString& strPath)
  {
    for (unsigned int i=0;i<actors.size();++i)
    {
      CFileItem item;
      item.SetLabel(actors[i].strName);
      CStdString strThumb = item.GetCachedActorThumb();
      if (!CFile::Exists(strThumb))
      {
        CStdString thumbFile = actors[i].strName;
        thumbFile.Replace(" ","_");
        thumbFile += ".tbn";
        CStdString strLocal = CUtil::AddFileToFolder(CUtil::AddFileToFolder(strPath, ".actors"), thumbFile);
        if (CFile::Exists(strLocal))
          CPicture::CreateThumbnail(strLocal, strThumb);
        else if (!actors[i].thumbUrl.GetFirstThumb().m_url.IsEmpty())
          DownloadImage(CScraperUrl::GetThumbURL(actors[i].thumbUrl.GetFirstThumb()), strThumb);
      }
    }
  }

  CNfoFile::NFOResult CVideoInfoScanner::CheckForNFOFile(CFileItem* pItem, bool bGrabAny, ScraperPtr& info, CScraperUrl& scrUrl)
  {
    CStdString strNfoFile;
    if (info->Content() == CONTENT_MOVIES || info->Content() == CONTENT_MUSICVIDEOS
        || (info->Content() == CONTENT_TVSHOWS && !pItem->m_bIsFolder))
      strNfoFile = GetnfoFile(pItem, bGrabAny);
    if (info->Content() == CONTENT_TVSHOWS && pItem->m_bIsFolder)
      CUtil::AddFileToFolder(pItem->m_strPath, "tvshow.nfo", strNfoFile);

    CNfoFile::NFOResult result=CNfoFile::NO_NFO;
    if (!strNfoFile.IsEmpty() && CFile::Exists(strNfoFile))
    {
      result = m_nfoReader.Create(strNfoFile,info,pItem->GetVideoInfoTag()->m_iEpisode);

      CStdString type;
      switch(result)
      {
        case CNfoFile::COMBINED_NFO:
          type = "Mixed";
          break;
        case CNfoFile::FULL_NFO:
          type = "Full";
          break;
        case CNfoFile::URL_NFO:
          type = "URL";
          break;
        default:
          type = "malformed";
      }
      CLog::Log(LOGDEBUG, "VideoInfoScanner: Found matching %s NFO file: %s", type.c_str(), strNfoFile.c_str());
      if (result == CNfoFile::FULL_NFO)
      {
        if (info->Content() == CONTENT_TVSHOWS)
          info = m_nfoReader.GetScraperInfo();
      }
      else if (result != CNfoFile::NO_NFO && result != CNfoFile::ERROR_NFO)
      {
        CScraperUrl url(m_nfoReader.m_strImDbUrl);
        scrUrl = url;

        scrUrl.strId  = m_nfoReader.m_strImDbNr;
        info = m_nfoReader.GetScraperInfo();

        CLog::Log(LOGDEBUG, "VideoInfoScanner: Fetching url '%s' using %s scraper (content: '%s')",
          scrUrl.m_url[0].m_url.c_str(), info->Name().c_str(), TranslateContent(info->Content()).c_str());

        if (result == CNfoFile::COMBINED_NFO)
          m_nfoReader.GetDetails(*pItem->GetVideoInfoTag());
      }
    }
    else
      CLog::Log(LOGDEBUG, "VideoInfoScanner: No NFO file found. Using title search for '%s'", pItem->m_strPath.c_str());

    return result;
  }

  bool CVideoInfoScanner::DownloadFailed(CGUIDialogProgress* pDialog)
  {
    if (pDialog)
    {
      CGUIDialogOK::ShowAndGetInput(20448,20449,20022,20022);
      return false;
    }
    return CGUIDialogYesNo::ShowAndGetInput(20448,20449,20450,20022);
  }

  bool CVideoInfoScanner::ProgressCancelled(CGUIDialogProgress* progress, int heading, const CStdString &line1)
  {
    if (progress)
    {
      progress->SetHeading(heading);
      progress->SetLine(0, line1);
      progress->SetLine(2, "");
      progress->Progress();
      return progress->IsCanceled();
    }
    return m_bStop;
  }

  int CVideoInfoScanner::FindVideo(const CStdString &videoName, const ScraperPtr &scraper, CScraperUrl &url, CGUIDialogProgress *progress)
  {
    IMDB_MOVIELIST movielist;
    CIMDB imdb(scraper);
    int returncode = imdb.FindMovie(videoName, movielist, progress);
    if (returncode == -1 || (returncode == 0 && !DownloadFailed(progress)))
    {
      m_bStop = true;
      return -1; // cancelled
    }
    if (returncode > 0 && movielist.size())
    {
      url = movielist[0];
      return 1;  // found a movie
    }
    return 0;    // didn't find anything
  }

  CStdString CVideoInfoScanner::GetParentDir(const CFileItem &item) const
  {
    CStdString strCheck = item.m_strPath;
    if (item.IsStack())
      strCheck = CStackDirectory::GetFirstStackedFile(item.m_strPath);

    CStdString strDirectory;
    CUtil::GetDirectory(strCheck, strDirectory);
    if (CUtil::IsInRAR(strCheck))
    {
      CStdString strPath=strDirectory;
      CUtil::GetParentPath(strPath, strDirectory);
    }
    if (item.IsStack())
    {
      strCheck = strDirectory;
      CUtil::RemoveSlashAtEnd(strCheck);
      if (CUtil::GetFileName(strCheck).size() == 3 && CUtil::GetFileName(strCheck).Left(2).Equals("cd"))
        CUtil::GetDirectory(strCheck, strDirectory);
    }
    return strDirectory;
  }

}
