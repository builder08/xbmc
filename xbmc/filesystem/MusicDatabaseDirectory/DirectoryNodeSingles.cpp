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

#ifndef MUSICDATABASEDIRECTORY_DIRECTORYNODESINGLES_H_INCLUDED
#define MUSICDATABASEDIRECTORY_DIRECTORYNODESINGLES_H_INCLUDED
#include "DirectoryNodeSingles.h"
#endif

#ifndef MUSICDATABASEDIRECTORY_QUERYPARAMS_H_INCLUDED
#define MUSICDATABASEDIRECTORY_QUERYPARAMS_H_INCLUDED
#include "QueryParams.h"
#endif

#ifndef MUSICDATABASEDIRECTORY_MUSIC_MUSICDATABASE_H_INCLUDED
#define MUSICDATABASEDIRECTORY_MUSIC_MUSICDATABASE_H_INCLUDED
#include "music/MusicDatabase.h"
#endif


using namespace XFILE::MUSICDATABASEDIRECTORY;

CDirectoryNodeSingles::CDirectoryNodeSingles(const CStdString& strName, CDirectoryNode* pParent)
  : CDirectoryNode(NODE_TYPE_SINGLES, strName, pParent)
{

}

bool CDirectoryNodeSingles::GetContent(CFileItemList& items) const
{
  CMusicDatabase musicdatabase;
  if (!musicdatabase.Open())
    return false;

  bool bSuccess=musicdatabase.GetSongsByWhere(BuildPath(), CDatabase::Filter(), items);

  musicdatabase.Close();

  return bSuccess;
}
