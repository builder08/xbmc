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

#include "Song.h"
#include "music/tags/MusicInfoTag.h"
#include "utils/Variant.h"
#include "FileItem.h"
#include "settings/AdvancedSettings.h"

using namespace std;
using namespace MUSIC_INFO;

CSong::CSong(CFileItem& item)
{
  CMusicInfoTag& tag = *item.GetMusicInfoTag();
  SYSTEMTIME stTime;
  tag.GetReleaseDate(stTime);
  strTitle = tag.GetTitle();
  genre = tag.GetGenre();
  artist = tag.GetArtist();
  std::vector<std::string> musicBrainArtistHints = tag.GetMusicBrainzArtistHints();
  strArtistDesc = tag.GetArtistDesc();
  if (!tag.GetMusicBrainzArtistID().empty())
  { // have musicbrainz artist info, so use it
    for (size_t i = 0; i < tag.GetMusicBrainzArtistID().size(); i++)
    {
      std::string artistId = tag.GetMusicBrainzArtistID()[i];
      std::string artistName;
      /*
       We try and get the corresponding artist name from the hints list.
       If the hints list is missing or the wrong length, it will try the artist list.
       We match on the same index, and if that fails just use the first name we have.
       */
      if (i < musicBrainArtistHints.size())
        artistName = musicBrainArtistHints[i];
      else if (!artist.empty())
        artistName = (i < artist.size()) ? artist[i] : artist[0];
      if (artistName.empty())
        artistName = artistId;
      std::string strJoinPhrase = (i == tag.GetMusicBrainzArtistID().size()-1) ? "" : g_advancedSettings.m_musicItemSeparator;
	    MusicArtistRoles role = (i == 0) ? Role_MainArtist : Role_FeaturedArtist;
      CArtistCredit artistCredit(artistName, artistId, strJoinPhrase, role);
      artistCredits.push_back(artistCredit);
    }
  }
  else
  { // no musicbrainz info, so fill in directly
    for (vector<string>::const_iterator it = tag.GetArtist().begin(); it != tag.GetArtist().end(); ++it)
    {
      std::string strJoinPhrase = (it == --tag.GetArtist().end() ? "" : g_advancedSettings.m_musicItemSeparator);
	    MusicArtistRoles role = (it == tag.GetArtist().begin()) ? Role_MainArtist : Role_FeaturedArtist;
      CArtistCredit artistCredit(*it, "", strJoinPhrase, role);
      artistCredits.push_back(artistCredit);
    }
  }
  strAlbum = tag.GetAlbum();
  albumArtist = tag.GetAlbumArtist();
  strMusicBrainzTrackID = tag.GetMusicBrainzTrackID();
  strComment = tag.GetComment();
  strCueSheet = tag.GetCueSheet();
  strMood = tag.GetMood();
  strComposer = tag.GetComposer();
  if (!strComposer.empty())
  {
    CArtistCredit ComposerCredit(strComposer, "", Role_Composer);
    // Set role if composer already in song artist credits, or add composer to song artist credits
    VECARTISTCREDITS::iterator credit = find(artistCredits.begin(), artistCredits.end(), ComposerCredit);
    if (credit != artistCredits.end() && credit->GetRole() < Role_Composer)
      credit->SetRole(Role_Composer);
    else
    {
      if (artistCredits.begin() != artistCredits.end())
        artistCredits.back().SetJoinPhrase(g_advancedSettings.m_musicItemSeparator);
      artistCredits.push_back(ComposerCredit);
    }
  }
  /*
  if (!strComposer.empty())
  {
    //Add composer, no separators as not built into song artist string
    CArtistCredit ComposerCredit(strComposer, "", Role_Composer);
    artistCredits.push_back(ComposerCredit);
  }
  */
  strEnsemble = tag.GetEnsemble();
  // Add ensemble to song artist credits
  if (!strEnsemble.empty())
  {
    CArtistCredit EnsembleCredit(strEnsemble, "", Role_Ensemble);
    // Add ensemble to song artist credits, or set role if exists as artist generally (not Composer or Conductor)
    VECARTISTCREDITS::iterator credit = find(artistCredits.begin(), artistCredits.end(), EnsembleCredit);
    if (credit != artistCredits.end() && credit->GetRole() < Role_Composer) 
      credit->SetRole(Role_Ensemble);
    else
    {  
      if (artistCredits.begin() != artistCredits.end())
        artistCredits.back().SetJoinPhrase(g_advancedSettings.m_musicItemSeparator);
      artistCredits.push_back(EnsembleCredit);
    }  
  }

  strConductor = tag.GetConductor();
  if (!strConductor.empty())
  {
    CArtistCredit ConductorCredit(strConductor, "", Role_Conductor);
    // Add Conductor to song artist credits, or set role if exists as artist generally (not Composer or Ensemble)
    VECARTISTCREDITS::iterator credit = find(artistCredits.begin(), artistCredits.end(), ConductorCredit);
    if (credit != artistCredits.end() && credit->GetRole() < Role_Composer)
      credit->SetRole(Role_Conductor);
    else
    {
      if (artistCredits.begin() != artistCredits.end())
        artistCredits.back().SetJoinPhrase(g_advancedSettings.m_musicItemSeparator);
      artistCredits.push_back(ConductorCredit);
    }
  }
  rating = tag.GetRating();
  iYear = stTime.wYear;
  iTrack = tag.GetTrackAndDiscNumber();
  iDuration = tag.GetDuration();
  bCompilation = tag.GetCompilation();
  embeddedArt = tag.GetCoverArtInfo();
  strFileName = tag.GetURL().empty() ? item.GetPath() : tag.GetURL();
  dateAdded = tag.GetDateAdded();
  strThumb = item.GetUserMusicThumb(true);
  iStartOffset = item.m_lStartOffset;
  iEndOffset = item.m_lEndOffset;
  idSong = -1;
  iTimesPlayed = 0;
  iKaraokeNumber = 0;
  iKaraokeDelay = 0;         //! Karaoke song lyrics-music delay in 1/10 seconds.
  idAlbum = -1;
}

CSong::CSong()
{
  Clear();
}

void CSong::MergeScrapedSong(const CSong& source, bool override)
{
  if ((override && !source.strTitle.empty()) || strTitle.empty())
    strTitle = source.strTitle;
  if ((override && source.iTrack != 0) || iTrack == 0)
    iTrack = source.iTrack;
  // artist = source.artist; // artist is read-only from the database
  if (override)
    artistCredits = source.artistCredits;
  else if (source.artistCredits.size() > artistCredits.size())
    artistCredits.insert(artistCredits.end(), source.artistCredits.begin()+artistCredits.size(), source.artistCredits.end());
}

void CSong::Serialize(CVariant& value) const
{
  value["filename"] = strFileName;
  value["title"] = strTitle;
  value["artist"] = artist;
  value["album"] = strAlbum;
  value["albumartist"] = albumArtist;
  value["genre"] = genre;
  value["duration"] = iDuration;
  value["track"] = iTrack;
  value["year"] = iYear;
  value["musicbrainztrackid"] = strMusicBrainzTrackID;
  value["comment"] = strComment;
  value["mood"] = strMood;
  value["composer"] = strComposer;
  value["ensemble"] = strEnsemble;
  value["conductor"] = strConductor;
  value["rating"] = rating;
  value["timesplayed"] = iTimesPlayed;
  value["lastplayed"] = lastPlayed.IsValid() ? lastPlayed.GetAsDBDateTime() : "";
  value["dateadded"] = dateAdded.IsValid() ? dateAdded.GetAsDBDateTime() : "";
  value["karaokenumber"] = (int64_t) iKaraokeNumber;
  value["albumid"] = idAlbum;
}

void CSong::Clear()
{
  strFileName.clear();
  strTitle.clear();
  artist.clear();
  strAlbum.clear();
  albumArtist.clear();
  genre.clear();
  strThumb.clear();
  strMusicBrainzTrackID.clear();
  strComment.clear();
  strMood.clear();
  strComposer.clear();
  strEnsemble.clear();
  strConductor.clear();
  rating = '0';
  iTrack = 0;
  iDuration = 0;
  iYear = 0;
  iStartOffset = 0;
  iEndOffset = 0;
  idSong = -1;
  iTimesPlayed = 0;
  lastPlayed.Reset();
  dateAdded.Reset();
  iKaraokeNumber = 0;
  strKaraokeLyrEncoding.clear();
  iKaraokeDelay = 0;
  idAlbum = -1;
  bCompilation = false;
  embeddedArt.clear();
}

bool CSong::HasArt() const
{
  if (!strThumb.empty()) return true;
  if (!embeddedArt.empty()) return true;
  return false;
}

bool CSong::ArtMatches(const CSong &right) const
{
  return (right.strThumb == strThumb &&
          embeddedArt.matches(right.embeddedArt));
}
