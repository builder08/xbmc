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

#ifndef FILESYSTEM_ASAPFILEDIRECTORY_H_INCLUDED
#define FILESYSTEM_ASAPFILEDIRECTORY_H_INCLUDED
#include "ASAPFileDirectory.h"
#endif

#ifndef FILESYSTEM_FILE_H_INCLUDED
#define FILESYSTEM_FILE_H_INCLUDED
#include "File.h"
#endif

#ifndef FILESYSTEM_MUSIC_TAGS_MUSICINFOTAGLOADERASAP_H_INCLUDED
#define FILESYSTEM_MUSIC_TAGS_MUSICINFOTAGLOADERASAP_H_INCLUDED
#include "music/tags/MusicInfoTagLoaderASAP.h"
#endif


using namespace XFILE;

CASAPFileDirectory::CASAPFileDirectory()
{
  m_strExt = "asapstream";
}

CASAPFileDirectory::~CASAPFileDirectory()
{
}

int CASAPFileDirectory::GetTrackCount(const CStdString &strPath)
{
  if (!m_dll.Load())
    return 0;

  MUSIC_INFO::CMusicInfoTagLoaderASAP loader;
  loader.Load(strPath,m_tag);
  m_tag.SetDuration(0); // ignore duration or all songs get duration of track 1

  return m_dll.asapGetSongs(strPath.c_str());
}
