/*
 *      Copyright (C) 2005-2007 Team XboxMediaCenter
 *      http://www.xboxmediacenter.com
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
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#include "stdafx.h"
#include "musicinfotagloadercdda.h"
#include "FileSystem/cddb.h"
#include "DetectDVDType.h"

using namespace MUSIC_INFO;
using namespace CDDB;

CMusicInfoTagLoaderCDDA::CMusicInfoTagLoaderCDDA(void)
{
}

CMusicInfoTagLoaderCDDA::~CMusicInfoTagLoaderCDDA()
{
}

bool CMusicInfoTagLoaderCDDA::Load(const CStdString& strFileName, CMusicInfoTag& tag)
{
  try
  {
    tag.SetURL(strFileName);
    bool bResult = false;

    // Get information for the inserted disc
    CCdInfo* pCdInfo = CDetectDVDMedia::GetCdInfo();
    if (pCdInfo == NULL)
      return bResult;

    // Prepare cddb
    Xcddb cddb;
    cddb.setCacheDir(g_settings.GetCDDBFolder());

    int iTrack = atoi(strFileName.substr(13, strFileName.size() - 13 - 5).c_str());

    // duration is always available
    tag.SetDuration( ( pCdInfo->GetTrackInformation(iTrack).nMins * 60 )
                     + pCdInfo->GetTrackInformation(iTrack).nSecs );

    // Only load cached cddb info in this tag loader, the internet database query is made in CCDDADirectory
    if (pCdInfo->HasCDDBInfo() && cddb.isCDCached(pCdInfo))
    {
      // get cddb information
      if (cddb.queryCDinfo(pCdInfo))
      {
        // Fill the fileitems music tag with cddb information, if available
        CStdString strTitle = cddb.getTrackTitle(iTrack);
        if (strTitle.size() > 0)
        {
          // Tracknumber
          tag.SetTrackNumber(iTrack);

          // Title
          tag.SetTitle(strTitle);

          // Artist: Use track artist or disc artist
          CStdString strArtist = cddb.getTrackArtist(iTrack);
          if (strArtist.IsEmpty())
            cddb.getDiskArtist(strArtist);
          tag.SetArtist(strArtist);

          // Album
          CStdString strAlbum;
          cddb.getDiskTitle( strAlbum );
          tag.SetAlbum(strAlbum);

          // Year
          SYSTEMTIME dateTime;
          dateTime.wYear = atoi(cddb.getYear().c_str());
          tag.SetReleaseDate( dateTime );

          // Genre
          tag.SetGenre( cddb.getGenre() );

          tag.SetLoaded(true);
          bResult = true;
        }
      }
    }
    else
    {
      // No cddb info, maybe we have CD-Text
      trackinfo ti = pCdInfo->GetTrackInformation(iTrack);

      // Fill the fileitems music tag with CD-Text information, if available
      CStdString strTitle = ti.cdtext.field[CDTEXT_TITLE];
      if (strTitle.size() > 0)
      {
        // Tracknumber
        tag.SetTrackNumber(iTrack);

        // Title
        tag.SetTitle(strTitle);

        // Get info for track zero, as we may have and need CD-Text Album info
        cdtext_t discCDText = pCdInfo->GetDiscCDTextInformation();

        // Artist: Use track artist or disc artist
        CStdString strArtist = ti.cdtext.field[CDTEXT_PERFORMER];
        if (strArtist.IsEmpty())
          strArtist = discCDText.field[CDTEXT_PERFORMER];
        tag.SetArtist(strArtist);

        // Album
        CStdString strAlbum;
        strAlbum = discCDText.field[CDTEXT_TITLE];
        tag.SetAlbum(strAlbum);

        // Genre
        CStdString strGenre = ti.cdtext.field[CDTEXT_GENRE];
        tag.SetGenre( strGenre );

        tag.SetLoaded(true);
        bResult = true;
      }
    }
    return bResult;
  }
  catch (...)
  {
    CLog::Log(LOGERROR, "Tag loader CDDB: exception in file %s", strFileName.c_str());
  }

  tag.SetLoaded(false);
  return false;
}
