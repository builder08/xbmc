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

#include "stdafx.h"
#include "FileSystem/StackDirectory.h"
#include "ThumbLoader.h"
#include "Util.h"
#include "URL.h"
#include "Picture.h"
#include "FileSystem/File.h"
#include "FileItem.h"
#include "Settings.h"
#include "TextureManagerGL.h"
#include "VideoInfoTag.h"

#include "cores/dvdplayer/DVDFileInfo.h"

using namespace XFILE;
using namespace DIRECTORY;

CThumbLoader::CThumbLoader(int nThreads) :
  CBackgroundInfoLoader(nThreads)
{
}

CThumbLoader::~CThumbLoader()
{
}

bool CThumbLoader::LoadRemoteThumb(CFileItem *pItem)
{
  // look for remote thumbs
  CStdString thumb(pItem->GetThumbnailImage());
  if (!g_TextureManager.CanLoad(thumb))
  {
    CStdString cachedThumb(pItem->GetCachedVideoThumb());
    if (CFile::Exists(cachedThumb))
      pItem->SetThumbnailImage(cachedThumb);
    else
    {
      CPicture pic;
      if(pic.DoCreateThumbnail(thumb, cachedThumb))
        pItem->SetThumbnailImage(cachedThumb);
      else
        pItem->SetThumbnailImage("");
    }
  }
  return pItem->HasThumbnail();
}

CVideoThumbLoader::CVideoThumbLoader() : 
  CThumbLoader(), m_pStreamDetailsObs(NULL)
{
}

CVideoThumbLoader::~CVideoThumbLoader()
{
  StopThread();
}

void CVideoThumbLoader::OnLoaderStart()
{
}

void CVideoThumbLoader::OnLoaderFinish()
{
}

bool CVideoThumbLoader::ExtractThumb(const CStdString &strPath, const CStdString &strTarget, CStreamDetails *pStreamDetails)
{
  if (!g_guiSettings.GetBool("myvideos.autothumb"))
    return false;

  if (CUtil::IsTV(strPath)
  ||  CUtil::IsUPnP(strPath)
  ||  CUtil::IsDAAP(strPath))
    return false;

  if (CUtil::IsRemote(strPath) && !CUtil::IsOnLAN(strPath))
    return false;

  CLog::Log(LOGDEBUG,"%s - trying to extract thumb from video file %s", __FUNCTION__, strPath.c_str());
  return CDVDFileInfo::ExtractThumb(strPath, strTarget, pStreamDetails);
}

/**
 * Look for a thumbnail for pItem.  If one does not exist, look for an autogenerated
 * thumbnail.  If that does not exist, attempt to autogenerate one.  Finally, check 
 * for the existance of fanart and set properties accordinly.
 * @return: true if pItem has been modified 
 */
bool CVideoThumbLoader::LoadItem(CFileItem* pItem)
{
  if (pItem->m_bIsShareOrDrive) return false;

  bool retVal = false;
  if (pItem->IsVideoDb() && pItem->HasVideoInfoTag() && !pItem->HasThumbnail())
  {
    if (pItem->m_bIsFolder && pItem->GetVideoInfoTag()->m_iSeason > -1)
      return false;
    CFileItem item(*pItem->GetVideoInfoTag());
    bool bResult = LoadItem(&item);
    if (bResult)
    {
      pItem->SetProperty("HasAutoThumb",item.GetProperty("HasAutoThumb"));
      pItem->SetProperty("AutoThumbImage",item.GetProperty("AutoThumbImage"));
      pItem->SetProperty("fanart_image",item.GetProperty("fanart_image"));
      pItem->SetThumbnailImage(item.GetThumbnailImage());
      pItem->GetVideoInfoTag()->m_streamDetails = item.GetVideoInfoTag()->m_streamDetails;
    }
    return bResult;
  }
  CStdString cachedThumb(pItem->GetCachedVideoThumb());

  if (!pItem->HasThumbnail())
  {
    pItem->SetUserVideoThumb();
    if (!CFile::Exists(cachedThumb))
    {
      CStdString strPath, strFileName;
      CUtil::Split(cachedThumb, strPath, strFileName);

      // create unique thumb for auto generated thumbs
      cachedThumb = strPath + "auto-" + strFileName;
      if (pItem->IsVideo() && !pItem->IsInternetStream() && !pItem->IsPlayList() && !CFile::Exists(cachedThumb))
      {
        CStreamDetails details;
        if (pItem->IsStack())
        {
          CStackDirectory stack;
          CVideoThumbLoader::ExtractThumb(stack.GetFirstStackedFile(pItem->m_strPath), cachedThumb, &details);
        }
        else
        {
          CVideoThumbLoader::ExtractThumb(pItem->m_strPath, cachedThumb, &details);
        }

        if (details.HasItems() && m_pStreamDetailsObs)
          m_pStreamDetailsObs->OnStreamDetails(details, pItem->m_strPath, -1);
      }

      if (CFile::Exists(cachedThumb))
      {
        pItem->SetProperty("HasAutoThumb", "1");
        pItem->SetProperty("AutoThumbImage", cachedThumb);
        pItem->SetThumbnailImage(cachedThumb);
        retVal = true;
      }
    }
  }
  else
    LoadRemoteThumb(pItem);

  if (!pItem->HasProperty("fanart_image"))
  {
    pItem->CacheFanart();
    if (CFile::Exists(pItem->GetCachedFanart()))
    {
      pItem->SetProperty("fanart_image",pItem->GetCachedFanart());
      retVal = true;
    }
  }

  if (!pItem->m_bIsFolder && !pItem->IsInternetStream() && 
       pItem->HasVideoInfoTag() && 
       g_guiSettings.GetBool("myvideos.extractflags")   &&
       !pItem->GetVideoInfoTag()->HasStreamDetails())
  {
    if (CDVDFileInfo::GetFileStreamDetails(pItem) && m_pStreamDetailsObs)
    {
      CVideoInfoTag *info = pItem->GetVideoInfoTag();
      m_pStreamDetailsObs->OnStreamDetails(info->m_streamDetails, "", info->m_iFileId);
      pItem->SetInvalid();
      retVal = true;
    }
  }

//  if (pItem->IsVideo() && !pItem->IsInternetStream())
//    CDVDPlayer::GetFileMetaData(pItem->m_strPath, pItem);

  return retVal;
}

CProgramThumbLoader::CProgramThumbLoader()
{
}

CProgramThumbLoader::~CProgramThumbLoader()
{
}

bool CProgramThumbLoader::LoadItem(CFileItem *pItem)
{
  if (pItem->m_bIsShareOrDrive) return true;
  if (!pItem->HasThumbnail())
    pItem->SetUserProgramThumb();
  else
    LoadRemoteThumb(pItem);
  return true;
}

CMusicThumbLoader::CMusicThumbLoader()
{
}

CMusicThumbLoader::~CMusicThumbLoader()
{
}

bool CMusicThumbLoader::LoadItem(CFileItem* pItem)
{
  if (pItem->m_bIsShareOrDrive) return true;
  if (!pItem->HasThumbnail())
    pItem->SetUserMusicThumb();
  else
    LoadRemoteThumb(pItem);
  return true;
}

