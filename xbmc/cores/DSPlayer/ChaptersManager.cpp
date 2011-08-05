/*
 *      Copyright (C) 2005-2010 Team XBMC
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

#ifdef HAS_DS_PLAYER

#include "ChaptersManager.h"

CChaptersManager *CChaptersManager::m_pSingleton = NULL;

CChaptersManager::CChaptersManager(void):
 m_currentChapter(-1),
 m_init(false)
{
}

CChaptersManager::~CChaptersManager(void)
{
  for (std::map<long, SChapterInfos *>::iterator it = m_chapters.begin();
    it != m_chapters.end(); ++it)
    delete it->second;

  CLog::Log(LOGDEBUG, "%s Ressources released", __FUNCTION__);
}

CChaptersManager *CChaptersManager::Get()
{
  return (m_pSingleton) ? m_pSingleton : (m_pSingleton = new CChaptersManager());
}

void CChaptersManager::Destroy()
{
  delete m_pSingleton;
  m_pSingleton = NULL;
}

int CChaptersManager::GetChapterCount()
{
  return m_chapters.size();
}

int CChaptersManager::GetChapter()
{
  if ( m_chapters.empty())
    return -1;
  else
    return m_currentChapter;
}

void CChaptersManager::GetChapterName(CStdString& strChapterName)
{
  if (m_currentChapter == -1)
    return;

  strChapterName = m_chapters[m_currentChapter]->name;
}

void CChaptersManager::UpdateChapters()
{
  if (m_pIAMExtendedSeeking && !m_chapters.empty() && GetChapterCount() > 1)
    m_pIAMExtendedSeeking->get_CurrentMarker(&m_currentChapter);
}

bool CChaptersManager::LoadChapters()
{
  if (! CGraphFilters::Get()->Splitter.pBF)
    return false;

  CStdString splitterName = CGraphFilters::Get()->Splitter.osdname;

  CLog::Log(LOGDEBUG, "%s Looking for chapters in \"%s\"", __FUNCTION__, splitterName.c_str());

  m_pIAMExtendedSeeking = CGraphFilters::Get()->Splitter.pBF;
  if (m_pIAMExtendedSeeking)
  {
    long chaptersCount = -1;
    m_pIAMExtendedSeeking->get_MarkerCount(&chaptersCount);
    if (chaptersCount <= 0)
    {
      m_pIAMExtendedSeeking.Release();
      return false;
    }

    SChapterInfos *infos = NULL;
    BSTR chapterName;
    for (int i = 1; i < chaptersCount + 1; i++)
    {
      infos = new SChapterInfos();
      infos->name = ""; infos->starttime = 0; infos->endtime = 0;

      if (SUCCEEDED(m_pIAMExtendedSeeking->GetMarkerName(i, &chapterName)))
      {
        g_charsetConverter.wToUTF8(chapterName, infos->name);
        SysFreeString(chapterName);
      } else
        infos->name = "Unknown chapter";

      double starttime = 0;
      m_pIAMExtendedSeeking->GetMarkerTime(i, &starttime);

      infos->starttime = (uint64_t) (starttime * 1000.0); // To ms
      CLog::Log(LOGNOTICE, "%s Chapter \"%s\" found. Start time: %"PRId64, __FUNCTION__, infos->name.c_str(), infos->starttime);
      m_chapters.insert( std::pair<long, SChapterInfos *>(i, infos) );
    }

    m_currentChapter = 1;

    return true;
  } 
  else
  {
    CLog::Log(LOGERROR, "%s The splitter \"%s\" doesn't support chapters", __FUNCTION__, splitterName.c_str());
    return false;
  }
}

int CChaptersManager::SeekChapter(int iChapter)
{
  if (! m_init)
    return -1;

  if (GetChapterCount() > 0)
  {
    if (iChapter < 1)
      iChapter = 1;
    if (iChapter > GetChapterCount())
      return -1;

    // Seek to the chapter.
    CLog::Log(LOGDEBUG, "%s Seeking to chapter %d", __FUNCTION__, iChapter);
    g_dsGraph->SeekInMilliSec( m_chapters[iChapter]->starttime );
  }
  else
  {
    // Do a regular big jump.
    if (iChapter > GetChapter())
      g_dsGraph->Seek(true, true);
    else
      g_dsGraph->Seek(false, true);
  }
  return iChapter;
}

void CChaptersManager::InitManager()
{
  m_init = true;
}
#endif