#pragma once


#include "../guilib/system.h"
#include "utils/Archive.h"
#include "utils/ScraperParser.h"
#include "PluginSettings.h"

#include <vector>

class CScraperSettings : public CBasicSettings
{
public:
  CScraperSettings();
  virtual ~CScraperSettings();
  bool LoadUserXML(const CStdString& strXML);
  bool LoadSettingsXML(const CStdString& strScraper, const CStdString& strFunction="GetSettings", const CScraperUrl* url=NULL);
  bool Load(const CStdString& strSettings, const CStdString& strSaved);
  CStdString GetSettings() const;
};

struct SScraperInfo
{
  CStdString strTitle;
  CStdString strPath;
  CStdString strThumb;
  CStdString strContent; // dupe, whatever
  CScraperSettings settings;
};

struct SActorInfo
{
  CStdString strName;
  CStdString strRole;
  CScraperUrl thumbUrl;
};

class CVideoInfoTag : public ISerializable
{
public:
  CVideoInfoTag() { Reset(); };
  void Reset();
  bool Load(const TiXmlElement *movie, bool chained = false);
  bool Save(TiXmlNode *node, const CStdString &tag, bool savePathInfo = true);
  virtual void Serialize(CArchive& ar);
  const CStdString GetCast(bool bIncludeRole = false) const;
  const CStdString GetArtist() const;

  CStdString m_strDirector;
  CStdString m_strWritingCredits;
  CStdString m_strGenre;
  CStdString m_strTagLine;
  CStdString m_strPlotOutline;
  CStdString m_strTrailer;
  CStdString m_strPlot;
  CScraperUrl m_strPictureURL;
  CStdString m_strTitle;
  CStdString m_strVotes;
  std::vector< CStdString> m_artist;
  std::vector< SActorInfo > m_cast;
  typedef std::vector< SActorInfo >::const_iterator iCast;

  CStdString m_strRuntime;
  CStdString m_strFile;
  CStdString m_strPath;
  CStdString m_strIMDBNumber;
  CStdString m_strMPAARating;
  CStdString m_strFileNameAndPath;
  CStdString m_strOriginalTitle;
  CStdString m_strEpisodeGuide;
  CStdString m_strPremiered;
  CStdString m_strStatus;
  CStdString m_strProductionCode;
  CStdString m_strFirstAired;
  CStdString m_strShowTitle;
  CStdString m_strStudio;
  CStdString m_strAlbum;
  bool m_bWatched;
  int m_iTop250;
  int m_iYear;
  int m_iSeason;
  int m_iEpisode;
  int m_iDbId;
  int m_iSpecialSortSeason;
  int m_iSpecialSortEpisode;
  float m_fRating;
  int m_iBookmarkId;
};
