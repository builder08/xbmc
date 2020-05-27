/*
 *  Copyright (C) 2005-2018 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#include "VideoInfoTag.h"

#include "ServiceBroker.h"
#include "TextureDatabase.h"
#include "guilib/LocalizeStrings.h"
#include "settings/AdvancedSettings.h"
#include "settings/SettingsComponent.h"
#include "utils/Archive.h"
#include "utils/JSONVariantParser.h"
#include "utils/JSONVariantWriter.h"
#include "utils/StringUtils.h"
#include "utils/Variant.h"
#include "utils/XMLUtils.h"
#include "utils/log.h"

#include <algorithm>
#include <sstream>
#include <string>
#include <vector>

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

static const auto UniqueIDMapJsonSerializer =
    [](const CDefaultedMap<std::string, std::string>& values) -> std::string {
  rapidjson::StringBuffer stringBuffer;
  rapidjson::Writer<rapidjson::StringBuffer> writer(stringBuffer);
  if (!writer.StartObject())
    return false;

  const auto defaultValue = values.GetDefault();
  if (!writer.Key("default") || !writer.String(defaultValue.c_str(), defaultValue.size()))
    return false;

  if (!writer.Key("uniqueids") || !writer.StartObject())
    return false;
  const auto& uniqueIDs = values.Get();
  for (const auto& value : uniqueIDs)
  {
    if (!writer.Key(value.first.c_str()) ||
        !writer.String(value.second.c_str(), value.second.size()))
      return false;
  }
  writer.EndObject(uniqueIDs.size());

  writer.EndObject(2);

  if (!writer.IsComplete())
    return false;

  return stringBuffer.GetString();
};

static const auto UniqueIDMapJsonDeserializer =
    [](const std::string& data) -> CDefaultedMap<std::string, std::string> {
  if (data.empty())
    return {};

  rapidjson::Document doc;
  doc.Parse<rapidjson::kParseIterativeFlag>(data.c_str(), data.size());
  if (doc.HasParseError() || !doc.IsObject() || !doc.HasMember("default") ||
      !doc.HasMember("uniqueids"))
    return {};

  const auto& defaultValue = doc["default"];
  if (!defaultValue.IsString())
    return {};

  const auto& jsonValues = doc["uniqueids"];
  if (!jsonValues.IsObject())
    return {};

  std::map<std::string, std::string> uniqueIDs;
  for (auto value = jsonValues.MemberBegin(); value != jsonValues.MemberEnd(); ++value)
  {
    if (!value->value.IsString())
      continue;

    uniqueIDs.emplace(value->name.GetString(), value->value.GetString());
  }

  CDefaultedMap<std::string, std::string> values;
  values.Set(std::move(uniqueIDs), defaultValue.GetString());

  return values;
};

static const auto RatingMapJsonSerializer =
    [](const CDefaultedMap<std::string, CRating>& values) -> std::string {
  rapidjson::StringBuffer stringBuffer;
  rapidjson::Writer<rapidjson::StringBuffer> writer(stringBuffer);
  if (!writer.StartObject())
    return false;

  const auto defaultValue = values.GetDefault();
  if (!writer.Key("default") || !writer.String(defaultValue.c_str(), defaultValue.size()))
    return false;

  if (!writer.Key("ratings") || !writer.StartObject())
    return false;
  const auto& ratings = values.Get();
  for (const auto& rating : ratings)
  {
    if (!writer.Key(rating.first.c_str()) || !writer.StartObject())
      return false;

    if (!writer.Key("rating") || !writer.Double(rating.second.rating))
      return false;

    if (!writer.Key("votes") || !writer.Int(rating.second.votes))
      return false;

    writer.EndObject(2);
  }
  writer.EndObject(ratings.size());

  writer.EndObject(2);

  if (!writer.IsComplete())
    return false;

  return stringBuffer.GetString();
};

static const auto RatingMapJsonDeserializer =
    [](const std::string& data) -> CDefaultedMap<std::string, CRating> {
  if (data.empty())
    return {};

  rapidjson::Document doc;
  doc.Parse<rapidjson::kParseIterativeFlag>(data.c_str(), data.size());
  if (doc.HasParseError() || !doc.IsObject() || !doc.HasMember("default") ||
      !doc.HasMember("ratings"))
    return {};

  const auto& defaultValue = doc["default"];
  if (!defaultValue.IsString())
    return {};

  const auto& jsonValues = doc["ratings"];
  if (!jsonValues.IsObject())
    return {};

  RatingMap ratings;
  for (auto value = jsonValues.MemberBegin(); value != jsonValues.MemberEnd(); ++value)
  {
    if (!value->value.IsObject() || !value->value.HasMember("rating") ||
        !value->value.HasMember("votes"))
      continue;

    const auto& ratingJsonValue = value->value["rating"];
    if (!ratingJsonValue.IsDouble())
      continue;

    const auto& votesJsonValue = value->value["votes"];
    if (!votesJsonValue.IsInt())
      continue;

    CRating rating;
    rating.rating = ratingJsonValue.GetDouble();
    rating.votes = votesJsonValue.GetInt();

    ratings.emplace(value->name.GetString(), rating);
  }

  CDefaultedMap<std::string, CRating> values;
  values.Set(std::move(ratings), defaultValue.GetString());

  return values;
};

static const auto StringVectorJsonSerializer =
    [](const std::vector<std::string>& values) -> std::string {
  rapidjson::StringBuffer stringBuffer;
  rapidjson::Writer<rapidjson::StringBuffer> writer(stringBuffer);
  if (!writer.StartArray())
    return false;

  for (const auto& value : values)
  {
    if (!writer.String(value.c_str(), value.size()))
      return false;
  }
  writer.EndArray(values.size());

  if (!writer.IsComplete())
    return false;

  return stringBuffer.GetString();
};

static const auto StringVectorJsonDeserializer =
    [](const std::string& data) -> std::vector<std::string> {
  if (data.empty())
    return {};

  rapidjson::Document doc;
  doc.Parse<rapidjson::kParseIterativeFlag>(data.c_str(), data.size());
  if (doc.HasParseError() || !doc.IsArray())
    return {};

  std::vector<std::string> values;
  for (const auto& value : doc.GetArray())
  {
    if (!value.IsString())
      continue;

    values.emplace_back(value.GetString());
  }

  return values;
};

static const auto CastJsonSerializer = [](const std::vector<SActorInfo>& values) -> std::string
{
  rapidjson::StringBuffer stringBuffer;
  rapidjson::Writer<rapidjson::StringBuffer> writer(stringBuffer);
  if (!writer.StartArray())
    return false;

  for (const auto& actor : values)
  {
    if (!writer.StartObject())
      return false;

    if (!writer.Key("name") || !writer.String(actor.strName.c_str()) ||
        !writer.Key("role") || !writer.String(actor.strRole.c_str()) ||
        !writer.Key("order") || !writer.Int(actor.order) ||
        !writer.Key("thumb") || !writer.String(actor.thumb.c_str()) ||
        !writer.Key("thumbUrl") || !writer.String(actor.thumbUrl.GetData().c_str()))
      return false;

    writer.EndObject(5);
  }
  writer.EndArray(values.size());

  if (!writer.IsComplete())
    return false;

  return stringBuffer.GetString();

  return {};
};

static const auto CastJsonDeserializer = [](const std::string& data) -> std::vector<SActorInfo>
{
  if (data.empty())
    return {};

  rapidjson::Document doc;
  doc.Parse<rapidjson::kParseIterativeFlag>(data.c_str(), data.size());
  if (doc.HasParseError() || !doc.IsArray())
    return {};

  std::vector<SActorInfo> actors;
  for (const auto& value : doc.GetArray())
  {
    if (!value.IsObject() || !value.HasMember("name") || !value.HasMember("role") ||
        !value.HasMember("order") || !value.HasMember("thumb") || !value.HasMember("thumbUrl"))
      continue;

    const auto& name = value["name"];
    if (!name.IsString())
      continue;
    const auto& role = value["role"];
    if (!role.IsString())
      continue;
    const auto& order = value["order"];
    if (!order.IsInt())
      continue;
    const auto& thumb = value["thumb"];
    if (!thumb.IsString())
      continue;
    const auto& thumbUrl = value["thumbUrl"];
    if (!thumbUrl.IsString())
      continue;

    SActorInfo actor;
    actor.strName = name.GetString();
    actor.strRole = role.GetString();
    actor.order = order.GetInt();
    actor.thumb = thumb.GetString();
    actor.thumbUrl.ParseFromData(thumbUrl.GetString());
    actors.emplace_back(actor);
  }

  return actors;
};

static const auto StreamDetailsJsonSerializer = [](const CStreamDetails& values) -> std::string
{
  rapidjson::StringBuffer stringBuffer;
  rapidjson::Writer<rapidjson::StringBuffer> writer(stringBuffer);
  if (!writer.StartArray())
    return false;

  size_t streamCount = 0;

  // serialize video streams
  for (size_t i = 0; i < values.GetVideoStreamCount(); ++i)
  {
    if (!writer.StartObject())
      return false;

    const auto stream = static_cast<const CStreamDetailVideo*>(values.GetNthStream(CStreamDetail::VIDEO, i));
    if (!writer.Key("type") || !writer.String("video") ||
        !writer.Key("width") || !writer.Int(stream->m_iWidth) ||
        !writer.Key("height") || !writer.Int(stream->m_iHeight) ||
        !writer.Key("aspect") || !writer.Double(stream->m_fAspect) ||
        !writer.Key("duration") || !writer.Int(stream->m_iDuration) ||
        !writer.Key("codec") || !writer.String(stream->m_strCodec.c_str()) ||
        !writer.Key("stereomode") || !writer.String(stream->m_strStereoMode.c_str()) ||
        !writer.Key("language") || !writer.String(stream->m_strLanguage.c_str()))
      return false;

    writer.EndObject(8);

    streamCount += 1;
  }

  // serialize audio streams
  for (size_t i = 0; i < values.GetAudioStreamCount(); ++i)
  {
    if (!writer.StartObject())
      return false;

    const auto stream = static_cast<const CStreamDetailAudio*>(values.GetNthStream(CStreamDetail::AUDIO, i));
    if (!writer.Key("type") || !writer.String("audio") ||
        !writer.Key("channels") || !writer.Int(stream->m_iChannels) ||
        !writer.Key("codec") || !writer.String(stream->m_strCodec.c_str()) ||
        !writer.Key("language") || !writer.String(stream->m_strLanguage.c_str()))
      return false;

    writer.EndObject(4);

    streamCount += 1;
  }

  // serialize subtitle streams
  for (size_t i = 0; i < values.GetSubtitleStreamCount(); ++i)
  {
    if (!writer.StartObject())
      return false;

    const auto stream = static_cast<const CStreamDetailSubtitle*>(values.GetNthStream(CStreamDetail::SUBTITLE, i));
    if (!writer.Key("type") || !writer.String("subtitle") ||
        !writer.Key("language") || !writer.String(stream->m_strLanguage.c_str()))
      return false;

    writer.EndObject(2);

    streamCount += 1;
  }

  writer.EndArray(streamCount);

  if (!writer.IsComplete())
    return false;

  return stringBuffer.GetString();
};

static const auto StreamDetailsJsonDeserializer = [](const std::string& data) -> CStreamDetails
{
  if (data.empty())
    return {};

  rapidjson::Document doc;
  doc.Parse<rapidjson::kParseIterativeFlag>(data.c_str(), data.size());
  if (doc.HasParseError() || !doc.IsArray())
    return {};

  CStreamDetails streamDetails;
  for (const auto& value : doc.GetArray())
  {
    if (!value.IsObject() || !value.HasMember("type"))
      continue;

    const auto& typeJson = value["type"];
    if (!typeJson.IsString())
      continue;

    const std::string type = typeJson.GetString();
    if (type == "video")
    {
      if (!value.HasMember("width") || !value.HasMember("height") || !value.HasMember("aspect") ||
          !value.HasMember("duration") || !value.HasMember("codec") ||
          !value.HasMember("stereomode") || !value.HasMember("language"))
        continue;

      const auto& width = value["width"];
      if (!width.IsInt())
        continue;
      const auto& height = value["height"];
      if (!height.IsInt())
        continue;
      const auto& aspect = value["aspect"];
      if (!aspect.IsDouble())
        continue;
      const auto& duration = value["duration"];
      if (!duration.IsInt())
        continue;
      const auto& codec = value["codec"];
      if (!codec.IsString())
        continue;
      const auto& stereomode = value["stereomode"];
      if (!stereomode.IsString())
        continue;
      const auto& language = value["language"];
      if (!language.IsString())
        continue;

      auto stream = new CStreamDetailVideo();
      stream->m_iWidth = width.GetInt();
      stream->m_iHeight = height.GetInt();
      stream->m_fAspect = static_cast<float>(aspect.GetDouble());
      stream->m_iDuration = duration.GetInt();
      stream->m_strCodec = codec.GetString();
      stream->m_strStereoMode = stereomode.GetString();
      stream->m_strLanguage = language.GetString();
      streamDetails.AddStream(stream);
    }
    else if (type == "audio")
    {
      if (!value.HasMember("channels") || !value.HasMember("codec") ||
          !value.HasMember("language"))
        continue;

      const auto& channels = value["channels"];
      if (!channels.IsInt())
        continue;
      const auto& codec = value["codec"];
      if (!codec.IsString())
        continue;
      const auto& language = value["language"];
      if (!language.IsString())
        continue;

      auto stream = new CStreamDetailAudio();
      stream->m_iChannels = channels.GetInt();
      stream->m_strCodec = codec.GetString();
      stream->m_strLanguage = language.GetString();
      streamDetails.AddStream(stream);
    }
    else if (type == "subtitle")
    {
      if (!value.HasMember("language"))
        continue;

      const auto& language = value["language"];
      if (!language.IsString())
        continue;

      auto stream = new CStreamDetailSubtitle();
      stream->m_strLanguage = language.GetString();
      streamDetails.AddStream(stream);
    }
    else
      continue;
  }

  return streamDetails;
};

CVideoInfoTag::CVideoInfoTag()
  : m_cast(CastJsonSerializer, CastJsonDeserializer),
    m_tags(StringVectorJsonSerializer, StringVectorJsonDeserializer),
    m_ratings(RatingMapJsonSerializer, RatingMapJsonDeserializer, "default"),
    m_streamDetails(StreamDetailsJsonSerializer, StreamDetailsJsonDeserializer),
    m_uniqueIDs(UniqueIDMapJsonSerializer, UniqueIDMapJsonDeserializer, "unknown")
{
  Reset();
};

void CVideoInfoTag::Reset()
{
  m_director.clear();
  m_writingCredits.clear();
  m_genre.clear();
  m_country.clear();
  m_strTagLine.clear();
  m_strPlotOutline.clear();
  m_strPlot.clear();
  m_strPictureURL.Clear();
  m_strTitle.clear();
  m_strShowTitle.clear();
  m_strOriginalTitle.clear();
  m_strSortTitle.clear();
  m_cast->clear();
  m_set.title.clear();
  m_set.id = -1;
  m_set.overview.clear();
  m_tags->clear();
  m_strFile.clear();
  m_strPath.clear();
  m_strMPAARating.clear();
  m_strFileNameAndPath.clear();
  m_premiered.Reset();
  m_bHasPremiered = false;
  m_strStatus.clear();
  m_strProductionCode.clear();
  m_firstAired.Reset();
  m_studio.clear();
  m_strAlbum.clear();
  m_artist.clear();
  m_strTrailer.clear();
  m_iTop250 = 0;
  m_iSeason = -1;
  m_iEpisode = -1;
  m_iIdUniqueID = -1;
  m_uniqueIDs->Clear();
  m_iSpecialSortSeason = -1;
  m_iSpecialSortEpisode = -1;
  m_iIdRating = -1;
  m_ratings->Clear();
  m_iUserRating = 0;
  m_iDbId = -1;
  m_iFileId = -1;
  m_iBookmarkId = -1;
  m_iTrack = -1;
  m_fanart.m_xml.clear();
  m_duration = 0;
  m_lastPlayed.Reset();
  m_showLink.clear();
  m_namedSeasons.clear();
  m_streamDetails->Reset();
  m_playCount = PLAYCOUNT_NOT_SET;
  m_EpBookmark.Reset();
  m_EpBookmark.type = CBookmark::EPISODE;
  m_basePath.clear();
  m_parentPathID = -1;
  m_resumePoint.Reset();
  m_resumePoint.type = CBookmark::RESUME;
  m_iIdShow = -1;
  m_iIdSeason = -1;
  m_dateAdded.Reset();
  m_type.clear();
  m_relevance = -1;
  m_parsedDetails = 0;
  m_coverArt.clear();
}

bool CVideoInfoTag::Save(TiXmlNode *node, const std::string &tag, bool savePathInfo, const TiXmlElement *additionalNode)
{
  if (!node) return false;

  // we start with a <tag> tag
  TiXmlElement movieElement(tag.c_str());
  TiXmlNode *movie = node->InsertEndChild(movieElement);

  if (!movie) return false;

  XMLUtils::SetString(movie, "title", m_strTitle);
  if (!m_strOriginalTitle.empty())
    XMLUtils::SetString(movie, "originaltitle", m_strOriginalTitle);
  if (!m_strShowTitle.empty())
    XMLUtils::SetString(movie, "showtitle", m_strShowTitle);
  if (!m_strSortTitle.empty())
    XMLUtils::SetString(movie, "sorttitle", m_strSortTitle);
  if (!m_ratings->Empty())
  {
    TiXmlElement ratings("ratings");
    const auto& defaultRating = m_ratings->GetDefault();
    for (const auto& it : m_ratings->Get())
    {
      TiXmlElement rating("rating");
      rating.SetAttribute("name", it.first.c_str());
      XMLUtils::SetFloat(&rating, "value", it.second.rating);
      XMLUtils::SetInt(&rating, "votes", it.second.votes);
      rating.SetAttribute("max", 10);
      if (it.first == defaultRating)
        rating.SetAttribute("default", "true");
      ratings.InsertEndChild(rating);
    }
    movie->InsertEndChild(ratings);
  }
  XMLUtils::SetInt(movie, "userrating", m_iUserRating);

  if (m_EpBookmark.timeInSeconds > 0)
  {
    TiXmlElement epbookmark("episodebookmark");
    XMLUtils::SetDouble(&epbookmark, "position", m_EpBookmark.timeInSeconds);
    if (!m_EpBookmark.playerState.empty())
    {
      TiXmlElement playerstate("playerstate");
      CXBMCTinyXML doc;
      doc.Parse(m_EpBookmark.playerState);
      playerstate.InsertEndChild(*doc.RootElement());
      epbookmark.InsertEndChild(playerstate);
    }
    movie->InsertEndChild(epbookmark);
  }

  XMLUtils::SetInt(movie, "top250", m_iTop250);
  if (tag == "episodedetails" || tag == "tvshow")
  {
    XMLUtils::SetInt(movie, "season", m_iSeason);
    XMLUtils::SetInt(movie, "episode", m_iEpisode);
    XMLUtils::SetInt(movie, "displayseason",m_iSpecialSortSeason);
    XMLUtils::SetInt(movie, "displayepisode",m_iSpecialSortEpisode);
  }
  if (tag == "musicvideo")
  {
    XMLUtils::SetInt(movie, "track", m_iTrack);
    XMLUtils::SetString(movie, "album", m_strAlbum);
  }
  XMLUtils::SetString(movie, "outline", m_strPlotOutline);
  XMLUtils::SetString(movie, "plot", m_strPlot);
  XMLUtils::SetString(movie, "tagline", m_strTagLine);
  XMLUtils::SetInt(movie, "runtime", GetDuration() / 60);
  if (m_strPictureURL.HasData())
  {
    CXBMCTinyXML doc;
    doc.Parse(m_strPictureURL.GetData());
    const TiXmlNode* thumb = doc.FirstChild("thumb");
    while (thumb)
    {
      movie->InsertEndChild(*thumb);
      thumb = thumb->NextSibling("thumb");
    }
  }
  if (m_fanart.m_xml.size())
  {
    CXBMCTinyXML doc;
    doc.Parse(m_fanart.m_xml);
    movie->InsertEndChild(*doc.RootElement());
  }
  XMLUtils::SetString(movie, "mpaa", m_strMPAARating);
  XMLUtils::SetInt(movie, "playcount", GetPlayCount());
  XMLUtils::SetDate(movie, "lastplayed", m_lastPlayed);
  if (savePathInfo)
  {
    XMLUtils::SetString(movie, "file", m_strFile);
    XMLUtils::SetString(movie, "path", m_strPath);
    XMLUtils::SetString(movie, "filenameandpath", m_strFileNameAndPath);
    XMLUtils::SetString(movie, "basepath", m_basePath);
  }
  if (!m_strEpisodeGuide.empty())
  {
    CXBMCTinyXML doc;
    doc.Parse(m_strEpisodeGuide);
    if (doc.RootElement())
      movie->InsertEndChild(*doc.RootElement());
    else
      XMLUtils::SetString(movie, "episodeguide", m_strEpisodeGuide);
  }

  XMLUtils::SetString(movie, "id", GetUniqueID());
  const auto& defaultUniqueID = m_uniqueIDs->GetDefault();
  for (const auto& uniqueid : m_uniqueIDs->Get())
  {
    TiXmlElement uniqueID("uniqueid");
    uniqueID.SetAttribute("type", uniqueid.first);
    if (uniqueid.first == defaultUniqueID)
      uniqueID.SetAttribute("default", "true");
    TiXmlText value(uniqueid.second);
    uniqueID.InsertEndChild(value);

    movie->InsertEndChild(uniqueID);
  }
  XMLUtils::SetStringArray(movie, "genre", m_genre);
  XMLUtils::SetStringArray(movie, "country", m_country);
  if (!m_set.title.empty())
  {
    TiXmlElement set("set");
    XMLUtils::SetString(&set, "name", m_set.title);
    if (!m_set.overview.empty())
      XMLUtils::SetString(&set, "overview", m_set.overview);
    movie->InsertEndChild(set);
  }
  XMLUtils::SetStringArray(movie, "tag", m_tags);
  XMLUtils::SetStringArray(movie, "credits", m_writingCredits);
  XMLUtils::SetStringArray(movie, "director", m_director);
  if (HasPremiered())
    XMLUtils::SetDate(movie, "premiered", m_premiered);
  if (HasYear())
    XMLUtils::SetInt(movie, "year", GetYear());
  XMLUtils::SetString(movie, "status", m_strStatus);
  XMLUtils::SetString(movie, "code", m_strProductionCode);
  XMLUtils::SetDate(movie, "aired", m_firstAired);
  XMLUtils::SetStringArray(movie, "studio", m_studio);
  XMLUtils::SetString(movie, "trailer", m_strTrailer);

  if (m_streamDetails->HasItems())
  {
    // it goes fileinfo/streamdetails/[video|audio|subtitle]
    TiXmlElement fileinfo("fileinfo");
    TiXmlElement streamdetails("streamdetails");
    for (int iStream=1; iStream<=m_streamDetails->GetVideoStreamCount(); iStream++)
    {
      TiXmlElement stream("video");
      XMLUtils::SetString(&stream, "codec", m_streamDetails->GetVideoCodec(iStream));
      XMLUtils::SetFloat(&stream, "aspect", m_streamDetails->GetVideoAspect(iStream));
      XMLUtils::SetInt(&stream, "width", m_streamDetails->GetVideoWidth(iStream));
      XMLUtils::SetInt(&stream, "height", m_streamDetails->GetVideoHeight(iStream));
      XMLUtils::SetInt(&stream, "durationinseconds", m_streamDetails->GetVideoDuration(iStream));
      XMLUtils::SetString(&stream, "stereomode", m_streamDetails->GetStereoMode(iStream));
      streamdetails.InsertEndChild(stream);
    }
    for (int iStream=1; iStream<=m_streamDetails->GetAudioStreamCount(); iStream++)
    {
      TiXmlElement stream("audio");
      XMLUtils::SetString(&stream, "codec", m_streamDetails->GetAudioCodec(iStream));
      XMLUtils::SetString(&stream, "language", m_streamDetails->GetAudioLanguage(iStream));
      XMLUtils::SetInt(&stream, "channels", m_streamDetails->GetAudioChannels(iStream));
      streamdetails.InsertEndChild(stream);
    }
    for (int iStream=1; iStream<=m_streamDetails->GetSubtitleStreamCount(); iStream++)
    {
      TiXmlElement stream("subtitle");
      XMLUtils::SetString(&stream, "language", m_streamDetails->GetSubtitleLanguage(iStream));
      streamdetails.InsertEndChild(stream);
    }
    fileinfo.InsertEndChild(streamdetails);
    movie->InsertEndChild(fileinfo);
  }  /* if has stream details */

  // cast
  for (iCast it = m_cast->begin(); it != m_cast->end(); ++it)
  {
    // add a <actor> tag
    TiXmlElement cast("actor");
    TiXmlNode *node = movie->InsertEndChild(cast);
    XMLUtils::SetString(node, "name", it->strName);
    XMLUtils::SetString(node, "role", it->strRole);
    XMLUtils::SetInt(node, "order", it->order);
    XMLUtils::SetString(node, "thumb", it->thumbUrl.GetFirstUrlByType().m_url);
  }
  XMLUtils::SetStringArray(movie, "artist", m_artist);
  XMLUtils::SetStringArray(movie, "showlink", m_showLink);

  for (const auto& namedSeason : m_namedSeasons)
  {
    TiXmlElement season("namedseason");
    season.SetAttribute("number", namedSeason.first);
    TiXmlText value(namedSeason.second);
    season.InsertEndChild(value);
    movie->InsertEndChild(season);
  }

  TiXmlElement resume("resume");
  XMLUtils::SetDouble(&resume, "position", m_resumePoint.timeInSeconds);
  XMLUtils::SetDouble(&resume, "total", m_resumePoint.totalTimeInSeconds);
  if (!m_resumePoint.playerState.empty())
  {
    TiXmlElement playerstate("playerstate");
    CXBMCTinyXML doc;
    doc.Parse(m_resumePoint.playerState);
    playerstate.InsertEndChild(*doc.RootElement());
    resume.InsertEndChild(playerstate);
  }
  movie->InsertEndChild(resume);

  XMLUtils::SetDateTime(movie, "dateadded", m_dateAdded);

  if (additionalNode)
    movie->InsertEndChild(*additionalNode);

  return true;
}

bool CVideoInfoTag::Load(const TiXmlElement *element, bool append, bool prioritise)
{
  if (!element)
    return false;
  if (!append)
    Reset();
  ParseNative(element, prioritise);
  return true;
}

void CVideoInfoTag::Archive(CArchive& ar)
{
  if (ar.IsStoring())
  {
    ar << m_director;
    ar << m_writingCredits;
    ar << m_genre;
    ar << m_country;
    ar << m_strTagLine;
    ar << m_strPlotOutline;
    ar << m_strPlot;
    ar << m_strPictureURL.GetData();
    ar << m_fanart.m_xml;
    ar << m_strTitle;
    ar << m_strSortTitle;
    ar << m_studio;
    ar << m_strTrailer;
    ar << (int)m_cast->size();
    for (const auto& actor : m_cast.value())
    {
      ar << actor.strName;
      ar << actor.strRole;
      ar << actor.order;
      ar << actor.thumb;
      ar << actor.thumbUrl.GetData();
    }

    ar << m_set.title;
    ar << m_set.id;
    ar << m_set.overview;
    ar << m_tags;
    ar << m_duration;
    ar << m_strFile;
    ar << m_strPath;
    ar << m_strMPAARating;
    ar << m_strFileNameAndPath;
    ar << m_strOriginalTitle;
    ar << m_strEpisodeGuide;
    ar << m_premiered;
    ar << m_bHasPremiered;
    ar << m_strStatus;
    ar << m_strProductionCode;
    ar << m_firstAired;
    ar << m_strShowTitle;
    ar << m_strAlbum;
    ar << m_artist;
    ar << GetPlayCount();
    ar << m_lastPlayed;
    ar << m_iTop250;
    ar << m_iSeason;
    ar << m_iEpisode;
    ar << (int)m_uniqueIDs->Size();
    const auto& defaultUniqueID = m_uniqueIDs->GetDefault();
    for (const auto& i : m_uniqueIDs->Get())
    {
      ar << i.first;
      ar << (i.first == defaultUniqueID);
      ar << i.second;
    }
    ar << (int)m_ratings->Size();
    const auto& defaultRating = m_ratings->GetDefault();
    for (const auto& i : m_ratings->Get())
    {
      ar << i.first;
      ar << (i.first == defaultRating);
      ar << i.second.rating;
      ar << i.second.votes;
    }
    ar << m_iUserRating;
    ar << m_iDbId;
    ar << m_iFileId;
    ar << m_iSpecialSortSeason;
    ar << m_iSpecialSortEpisode;
    ar << m_iBookmarkId;
    ar << m_iTrack;
    ar << dynamic_cast<IArchivable&>(GetStreamDetails());
    ar << m_showLink;
    ar << static_cast<int>(m_namedSeasons.size());
    for (const auto& namedSeason : m_namedSeasons)
    {
      ar << namedSeason.first;
      ar << namedSeason.second;
    }
    ar << m_EpBookmark.playerState;
    ar << m_EpBookmark.timeInSeconds;
    ar << m_basePath;
    ar << m_parentPathID;
    ar << m_resumePoint.timeInSeconds;
    ar << m_resumePoint.totalTimeInSeconds;
    ar << m_resumePoint.playerState;
    ar << m_iIdShow;
    ar << m_dateAdded.GetAsDBDateTime();
    ar << m_type;
    ar << m_iIdSeason;
    ar << m_coverArt.size();
    for (auto& it : m_coverArt)
      ar << it;
  }
  else
  {
    ar >> m_director;
    ar >> m_writingCredits;
    ar >> m_genre;
    ar >> m_country;
    ar >> m_strTagLine;
    ar >> m_strPlotOutline;
    ar >> m_strPlot;
    std::string data;
    ar >> data;
    m_strPictureURL.SetData(data);
    ar >> m_fanart.m_xml;
    ar >> m_strTitle;
    ar >> m_strSortTitle;
    ar >> m_studio;
    ar >> m_strTrailer;
    int iCastSize;
    ar >> iCastSize;
    m_cast->reserve(iCastSize);
    for (int i=0;i<iCastSize;++i)
    {
      SActorInfo info;
      ar >> info.strName;
      ar >> info.strRole;
      ar >> info.order;
      ar >> info.thumb;
      std::string strXml;
      ar >> strXml;
      info.thumbUrl.ParseFromData(strXml);
      m_cast->push_back(info);
    }

    ar >> m_set.title;
    ar >> m_set.id;
    ar >> m_set.overview;
    ar >> m_tags;
    ar >> m_duration;
    ar >> m_strFile;
    ar >> m_strPath;
    ar >> m_strMPAARating;
    ar >> m_strFileNameAndPath;
    ar >> m_strOriginalTitle;
    ar >> m_strEpisodeGuide;
    ar >> m_premiered;
    ar >> m_bHasPremiered;
    ar >> m_strStatus;
    ar >> m_strProductionCode;
    ar >> m_firstAired;
    ar >> m_strShowTitle;
    ar >> m_strAlbum;
    ar >> m_artist;
    ar >> m_playCount;
    ar >> m_lastPlayed;
    ar >> m_iTop250;
    ar >> m_iSeason;
    ar >> m_iEpisode;
    int iUniqueIDSize;
    ar >> iUniqueIDSize;
    for (int i = 0; i < iUniqueIDSize; ++i)
    {
      std::string value;
      std::string name;
      bool defaultUniqueID;
      ar >> name;
      ar >> defaultUniqueID;
      ar >> value;
      SetUniqueID(value, name, defaultUniqueID);
    }
    int iRatingSize;
    ar >> iRatingSize;
    for (int i = 0; i < iRatingSize; ++i)
    {
      CRating rating;
      std::string name;
      bool defaultRating;
      ar >> name;
      ar >> defaultRating;
      ar >> rating.rating;
      ar >> rating.votes;
      SetRating(rating, name, defaultRating);
    }
    ar >> m_iUserRating;
    ar >> m_iDbId;
    ar >> m_iFileId;
    ar >> m_iSpecialSortSeason;
    ar >> m_iSpecialSortEpisode;
    ar >> m_iBookmarkId;
    ar >> m_iTrack;
    ar >> dynamic_cast<IArchivable&>(GetStreamDetails());
    ar >> m_showLink;

    int namedSeasonSize;
    ar >> namedSeasonSize;
    for (int i = 0; i < namedSeasonSize; ++i)
    {
      int seasonNumber;
      ar >> seasonNumber;
      std::string seasonName;
      ar >> seasonName;
      m_namedSeasons.insert(std::make_pair(seasonNumber, seasonName));
    }
    ar >> m_EpBookmark.playerState;
    ar >> m_EpBookmark.timeInSeconds;
    ar >> m_basePath;
    ar >> m_parentPathID;
    ar >> m_resumePoint.timeInSeconds;
    ar >> m_resumePoint.totalTimeInSeconds;
    ar >> m_resumePoint.playerState;
    ar >> m_iIdShow;

    std::string dateAdded;
    ar >> dateAdded;
    m_dateAdded.SetFromDBDateTime(dateAdded);
    ar >> m_type;
    ar >> m_iIdSeason;
    size_t size;
    ar >> size;
    m_coverArt.resize(size);
    for (size_t i = 0; i < size; ++i)
      ar >> m_coverArt[i];
  }
}

void CVideoInfoTag::Serialize(CVariant& value) const
{
  value["director"] = m_director;
  value["writer"] = m_writingCredits;
  value["genre"] = m_genre;
  value["country"] = m_country;
  value["tagline"] = m_strTagLine;
  value["plotoutline"] = m_strPlotOutline;
  value["plot"] = m_strPlot;
  value["title"] = m_strTitle;
  value["votes"] = StringUtils::Format("%i", GetRating().votes);
  value["studio"] = m_studio;
  value["trailer"] = m_strTrailer;
  value["cast"] = CVariant(CVariant::VariantTypeArray);
  for (const auto& actor : m_cast.value())
  {
    CVariant actorVariant;
    actorVariant["name"] = actor.strName;
    actorVariant["role"] = actor.strRole;
    actorVariant["order"] = actor.order;
    if (!actor.thumb.empty())
      actorVariant["thumbnail"] = CTextureUtils::GetWrappedImageURL(actor.thumb);
    value["cast"].push_back(actorVariant);
  }
  value["set"] = m_set.title;
  value["setid"] = m_set.id;
  value["setoverview"] = m_set.overview;
  value["tag"] = *m_tags;
  value["runtime"] = GetDuration();
  value["file"] = m_strFile;
  value["path"] = m_strPath;
  value["imdbnumber"] = GetUniqueID();
  value["mpaa"] = m_strMPAARating;
  value["filenameandpath"] = m_strFileNameAndPath;
  value["originaltitle"] = m_strOriginalTitle;
  value["sorttitle"] = m_strSortTitle;
  value["episodeguide"] = m_strEpisodeGuide;
  value["premiered"] = m_premiered.IsValid() ? m_premiered.GetAsDBDate() : StringUtils::Empty;
  value["status"] = m_strStatus;
  value["productioncode"] = m_strProductionCode;
  value["firstaired"] = m_firstAired.IsValid() ? m_firstAired.GetAsDBDate() : StringUtils::Empty;
  value["showtitle"] = m_strShowTitle;
  value["album"] = m_strAlbum;
  value["artist"] = m_artist;
  value["playcount"] = GetPlayCount();
  value["lastplayed"] = m_lastPlayed.IsValid() ? m_lastPlayed.GetAsDBDateTime() : StringUtils::Empty;
  value["top250"] = m_iTop250;
  value["year"] = m_premiered.GetYear();
  value["season"] = m_iSeason;
  value["episode"] = m_iEpisode;
  for (const auto& i : m_uniqueIDs->Get())
    value["uniqueid"][i.first] = i.second;

  value["rating"] = GetRating().rating;
  CVariant ratings = CVariant(CVariant::VariantTypeObject);
  const auto& defaultRating = m_ratings->GetDefault();
  for (const auto& i : m_ratings->Get())
  {
    CVariant rating;
    rating["rating"] = i.second.rating;
    rating["votes"] = i.second.votes;
    rating["default"] = i.first == defaultRating;

    ratings[i.first] = rating;
  }
  value["ratings"] = ratings;
  value["userrating"] = m_iUserRating;
  value["dbid"] = m_iDbId;
  value["fileid"] = m_iFileId;
  value["track"] = m_iTrack;
  value["showlink"] = m_showLink;
  m_streamDetails->Serialize(value["streamdetails"]);
  CVariant resume = CVariant(CVariant::VariantTypeObject);
  resume["position"] = m_resumePoint.timeInSeconds;
  resume["total"] = m_resumePoint.totalTimeInSeconds;
  value["resume"] = resume;
  value["tvshowid"] = m_iIdShow;
  value["dateadded"] = m_dateAdded.IsValid() ? m_dateAdded.GetAsDBDateTime() : StringUtils::Empty;
  value["type"] = m_type;
  value["seasonid"] = m_iIdSeason;
  value["specialsortseason"] = m_iSpecialSortSeason;
  value["specialsortepisode"] = m_iSpecialSortEpisode;
}

void CVideoInfoTag::ToSortable(SortItem& sortable, Field field) const
{
  switch (field)
  {
  case FieldDirector:                 sortable[FieldDirector] = m_director; break;
  case FieldWriter:                   sortable[FieldWriter] = m_writingCredits; break;
  case FieldGenre:                    sortable[FieldGenre] = m_genre; break;
  case FieldCountry:                  sortable[FieldCountry] = m_country; break;
  case FieldTagline:                  sortable[FieldTagline] = m_strTagLine; break;
  case FieldPlotOutline:              sortable[FieldPlotOutline] = m_strPlotOutline; break;
  case FieldPlot:                     sortable[FieldPlot] = m_strPlot; break;
  case FieldTitle:
  {
    // make sure not to overwrite an existing title with an empty one
    std::string title = m_strTitle;
    if (!title.empty() || sortable.find(FieldTitle) == sortable.end())
      sortable[FieldTitle] = title;
    break;
  }
  case FieldVotes:                    sortable[FieldVotes] = GetRating().votes; break;
  case FieldStudio:                   sortable[FieldStudio] = m_studio; break;
  case FieldTrailer:                  sortable[FieldTrailer] = m_strTrailer; break;
  case FieldSet:                      sortable[FieldSet] = m_set.title; break;
  case FieldTime:                     sortable[FieldTime] = GetDuration(); break;
  case FieldFilename:                 sortable[FieldFilename] = m_strFile; break;
  case FieldMPAA:                     sortable[FieldMPAA] = m_strMPAARating; break;
  case FieldPath:
  {
    // make sure not to overwrite an existing path with an empty one
    std::string path = GetPath();
    if (!path.empty() || sortable.find(FieldPath) == sortable.end())
      sortable[FieldPath] = path;
    break;
  }
  case FieldSortTitle:
  {
    // seasons with a custom name/title need special handling as they should be sorted by season number
    if (m_type == MediaTypeSeason && !m_strSortTitle.empty())
      sortable[FieldSortTitle] = StringUtils::Format(g_localizeStrings.Get(20358).c_str(), m_iSeason);
    else
      sortable[FieldSortTitle] = m_strSortTitle;
    break;
  }
  case FieldTvShowStatus:             sortable[FieldTvShowStatus] = m_strStatus; break;
  case FieldProductionCode:           sortable[FieldProductionCode] = m_strProductionCode; break;
  case FieldAirDate:                  sortable[FieldAirDate] = m_firstAired.IsValid() ? m_firstAired.GetAsDBDate() : (m_premiered.IsValid() ? m_premiered.GetAsDBDate() : StringUtils::Empty); break;
  case FieldTvShowTitle:              sortable[FieldTvShowTitle] = m_strShowTitle; break;
  case FieldAlbum:                    sortable[FieldAlbum] = m_strAlbum; break;
  case FieldArtist:                   sortable[FieldArtist] = m_artist; break;
  case FieldPlaycount:                sortable[FieldPlaycount] = GetPlayCount(); break;
  case FieldLastPlayed:               sortable[FieldLastPlayed] = m_lastPlayed.IsValid() ? m_lastPlayed.GetAsDBDateTime() : StringUtils::Empty; break;
  case FieldTop250:                   sortable[FieldTop250] = m_iTop250; break;
  case FieldYear:                     sortable[FieldYear] = m_premiered.GetYear(); break;
  case FieldSeason:                   sortable[FieldSeason] = m_iSeason; break;
  case FieldEpisodeNumber:            sortable[FieldEpisodeNumber] = m_iEpisode; break;
  case FieldNumberOfEpisodes:         sortable[FieldNumberOfEpisodes] = m_iEpisode; break;
  case FieldNumberOfWatchedEpisodes:  sortable[FieldNumberOfWatchedEpisodes] = m_iEpisode; break;
  case FieldEpisodeNumberSpecialSort: sortable[FieldEpisodeNumberSpecialSort] = m_iSpecialSortEpisode; break;
  case FieldSeasonSpecialSort:        sortable[FieldSeasonSpecialSort] = m_iSpecialSortSeason; break;
  case FieldRating:                   sortable[FieldRating] = GetRating().rating; break;
  case FieldUserRating:               sortable[FieldUserRating] = m_iUserRating; break;
  case FieldId:                       sortable[FieldId] = m_iDbId; break;
  case FieldTrackNumber:              sortable[FieldTrackNumber] = m_iTrack; break;
  case FieldTag:                      sortable[FieldTag] = *m_tags; break;

  case FieldVideoResolution:          sortable[FieldVideoResolution] = m_streamDetails->GetVideoHeight(); break;
  case FieldVideoAspectRatio:         sortable[FieldVideoAspectRatio] = m_streamDetails->GetVideoAspect(); break;
  case FieldVideoCodec:               sortable[FieldVideoCodec] = m_streamDetails->GetVideoCodec(); break;
  case FieldStereoMode:               sortable[FieldStereoMode] = m_streamDetails->GetStereoMode(); break;

  case FieldAudioChannels:            sortable[FieldAudioChannels] = m_streamDetails->GetAudioChannels(); break;
  case FieldAudioCodec:               sortable[FieldAudioCodec] = m_streamDetails->GetAudioCodec(); break;
  case FieldAudioLanguage:            sortable[FieldAudioLanguage] = m_streamDetails->GetAudioLanguage(); break;

  case FieldSubtitleLanguage:         sortable[FieldSubtitleLanguage] = m_streamDetails->GetSubtitleLanguage(); break;

  case FieldInProgress:               sortable[FieldInProgress] = m_resumePoint.IsPartWay(); break;
  case FieldDateAdded:                sortable[FieldDateAdded] = m_dateAdded.IsValid() ? m_dateAdded.GetAsDBDateTime() : StringUtils::Empty; break;
  case FieldMediaType:                sortable[FieldMediaType] = m_type; break;
  case FieldRelevance:                sortable[FieldRelevance] = m_relevance; break;
  default: break;
  }
}

const std::vector<std::string>& CVideoInfoTag::GetTags() const
{
  return m_tags.value();
}

const CRating CVideoInfoTag::GetRating(std::string type) const
{
  return m_ratings->GetValue(type);
}

const std::map<std::string, CRating>& CVideoInfoTag::GetRatings() const
{
  return m_ratings->Get();
}

const std::string& CVideoInfoTag::GetDefaultRating() const
{
  return m_ratings->GetDefault();
}

bool CVideoInfoTag::HasYear() const
{
  return m_firstAired.IsValid() || m_premiered.IsValid();
}

int CVideoInfoTag::GetYear() const
{
  if (m_firstAired.IsValid())
    return GetFirstAired().GetYear();
  if (m_premiered.IsValid())
    return GetPremiered().GetYear();
  return 0;
}

bool CVideoInfoTag::HasPremiered() const
{
  return m_bHasPremiered;
}

const CDateTime& CVideoInfoTag::GetPremiered() const
{
  return m_premiered;
}

const CDateTime& CVideoInfoTag::GetFirstAired() const
{
  return m_firstAired;
}

const std::string CVideoInfoTag::GetUniqueID(std::string type) const
{
  return m_uniqueIDs->GetValue(type);
}

const std::map<std::string, std::string>& CVideoInfoTag::GetUniqueIDs() const
{
  return m_uniqueIDs->Get();
}

const std::string& CVideoInfoTag::GetDefaultUniqueID() const
{
  return m_uniqueIDs->GetDefault();
}

bool CVideoInfoTag::HasUniqueID() const
{
  return !m_uniqueIDs->Empty();
}

const std::vector<SActorInfo>& CVideoInfoTag::GetCast() const
{
  return m_cast.value();
}

const std::string CVideoInfoTag::GetCastAsString(bool bIncludeRole /*= false*/) const
{
  std::string strLabel;
  for (iCast it = m_cast->begin(); it != m_cast->end(); ++it)
  {
    std::string character;
    if (it->strRole.empty() || !bIncludeRole)
      character = StringUtils::Format("%s\n", it->strName.c_str());
    else
      character = StringUtils::Format("%s %s %s\n", it->strName.c_str(), g_localizeStrings.Get(20347).c_str(), it->strRole.c_str());
    strLabel += character;
  }
  return StringUtils::TrimRight(strLabel, "\n");
}

void CVideoInfoTag::ParseNative(const TiXmlElement* movie, bool prioritise)
{
  std::string value;
  float fValue;

  if (XMLUtils::GetString(movie, "title", value))
    SetTitle(value);

  if (XMLUtils::GetString(movie, "originaltitle", value))
    SetOriginalTitle(value);

  if (XMLUtils::GetString(movie, "showtitle", value))
    SetShowTitle(value);

  if (XMLUtils::GetString(movie, "sorttitle", value))
    SetSortTitle(value);

  const TiXmlElement* node = movie->FirstChildElement("ratings");
  if (node)
  {
    for (const TiXmlElement* child = node->FirstChildElement("rating"); child != nullptr; child = child->NextSiblingElement("rating"))
    {
      CRating r;
      std::string name;
      if (child->QueryStringAttribute("name", &name) != TIXML_SUCCESS)
        name = "default";
      XMLUtils::GetFloat(child, "value", r.rating);
      if (XMLUtils::GetString(child, "votes", value))
        r.votes = StringUtils::ReturnDigits(value);
      int max_value = 10;
      if ((child->QueryIntAttribute("max", &max_value) == TIXML_SUCCESS) && max_value >= 1)
        r.rating = r.rating / max_value * 10; // Normalise the Movie Rating to between 1 and 10
      bool isDefault = false;
      bool isDefaultRating = false;
      // guard against assert in tinyxml
      const char* rAtt = child->Attribute("default", static_cast<int*>(nullptr));
      if (rAtt && strlen(rAtt) != 0 &&
          (child->QueryBoolAttribute("default", &isDefault) == TIXML_SUCCESS) && isDefault)
        isDefaultRating = isDefault;
      SetRating(r, name, isDefaultRating);
    }
  }
  else if (XMLUtils::GetFloat(movie, "rating", fValue))
  {
    CRating r(fValue, 0);
    if (XMLUtils::GetString(movie, "votes", value))
      r.votes = StringUtils::ReturnDigits(value);
    int max_value = 10;
    const TiXmlElement* rElement = movie->FirstChildElement("rating");
    if (rElement && (rElement->QueryIntAttribute("max", &max_value) == TIXML_SUCCESS) && max_value >= 1)
      r.rating = r.rating / max_value * 10; // Normalise the Movie Rating to between 1 and 10
    SetRating(r, "default", true);
  }
  XMLUtils::GetInt(movie, "userrating", m_iUserRating);

  const TiXmlElement *epbookmark = movie->FirstChildElement("episodebookmark");
  if (epbookmark)
  {
    XMLUtils::GetDouble(epbookmark, "position", m_EpBookmark.timeInSeconds);
    const TiXmlElement *playerstate = epbookmark->FirstChildElement("playerstate");
    if (playerstate)
    {
      const TiXmlElement *value = playerstate->FirstChildElement();
      if (value)
        m_EpBookmark.playerState << *value;
    }
  }
  else
    XMLUtils::GetDouble(movie, "epbookmark", m_EpBookmark.timeInSeconds);

  int max_value = 10;
  const TiXmlElement* urElement = movie->FirstChildElement("userrating");
  if (urElement && (urElement->QueryIntAttribute("max", &max_value) == TIXML_SUCCESS) && max_value >= 1)
    m_iUserRating = m_iUserRating / max_value * 10; // Normalise the user Movie Rating to between 1 and 10
  XMLUtils::GetInt(movie, "top250", m_iTop250);
  XMLUtils::GetInt(movie, "season", m_iSeason);
  XMLUtils::GetInt(movie, "episode", m_iEpisode);
  XMLUtils::GetInt(movie, "track", m_iTrack);

  XMLUtils::GetInt(movie, "displayseason", m_iSpecialSortSeason);
  XMLUtils::GetInt(movie, "displayepisode", m_iSpecialSortEpisode);
  int after=0;
  XMLUtils::GetInt(movie, "displayafterseason",after);
  if (after > 0)
  {
    m_iSpecialSortSeason = after;
    m_iSpecialSortEpisode = 0x1000; // should be more than any realistic episode number
  }

  if (XMLUtils::GetString(movie, "outline", value))
    SetPlotOutline(value);

  if (XMLUtils::GetString(movie, "plot", value))
    SetPlot(value);

  if (XMLUtils::GetString(movie, "tagline", value))
    SetTagLine(value);


  if (XMLUtils::GetString(movie, "runtime", value) && !value.empty())
    m_duration = GetDurationFromMinuteString(StringUtils::Trim(value));

  if (XMLUtils::GetString(movie, "mpaa", value))
    SetMPAARating(value);

  XMLUtils::GetInt(movie, "playcount", m_playCount);
  XMLUtils::GetDate(movie, "lastplayed", m_lastPlayed);

  if (XMLUtils::GetString(movie, "file", value))
    SetFile(value);

  if (XMLUtils::GetString(movie, "path", value))
    SetPath(value);

  const TiXmlElement* uniqueid = movie->FirstChildElement("uniqueid");
  if (uniqueid == nullptr)
  {
    if (XMLUtils::GetString(movie, "id", value))
      SetUniqueID(value);
  }
  else
  {
    for (; uniqueid != nullptr; uniqueid = uniqueid->NextSiblingElement("uniqueid"))
    {
      if (uniqueid->FirstChild())
      {
        std::string type;
        if (uniqueid->QueryStringAttribute("type", &value) == TIXML_SUCCESS)
          type = value;
        bool isDefault;
        bool isDefaultUniqueId = false;
        if ((uniqueid->QueryBoolAttribute("default", &isDefault) == TIXML_SUCCESS) && isDefault)
          isDefaultUniqueId = isDefault;
        SetUniqueID(uniqueid->FirstChild()->ValueStr(), type, isDefaultUniqueId);
      }
    }
  }

  if (XMLUtils::GetString(movie, "filenameandpath", value))
    SetFileNameAndPath(value);

  if (XMLUtils::GetDate(movie, "premiered", m_premiered))
  {
    m_bHasPremiered = true;
  }
  else
  {
    int year;
    if (XMLUtils::GetInt(movie, "year", year))
      SetYear(year);
  }

  if (XMLUtils::GetString(movie, "status", value))
    SetStatus(value);

  if (XMLUtils::GetString(movie, "code", value))
    SetProductionCode(value);

  XMLUtils::GetDate(movie, "aired", m_firstAired);

  if (XMLUtils::GetString(movie, "album", value))
    SetAlbum(value);

  if (XMLUtils::GetString(movie, "trailer", value))
    SetTrailer(value);

  if (XMLUtils::GetString(movie, "basepath", value))
    SetBasePath(value);

  // make sure the picture URLs have been parsed
  m_strPictureURL.Parse();
  size_t iThumbCount = m_strPictureURL.GetUrls().size();
  std::string xmlAdd = m_strPictureURL.GetData();

  const TiXmlElement* thumb = movie->FirstChildElement("thumb");
  while (thumb)
  {
    m_strPictureURL.ParseAndAppendUrl(thumb);
    if (prioritise)
    {
      std::string temp;
      temp << *thumb;
      xmlAdd = temp+xmlAdd;
    }
    thumb = thumb->NextSiblingElement("thumb");
  }

  // prioritise thumbs from nfos
  if (prioritise && iThumbCount && iThumbCount != m_strPictureURL.GetUrls().size())
  {
    auto thumbUrls = m_strPictureURL.GetUrls();
    rotate(thumbUrls.begin(), thumbUrls.begin() + iThumbCount, thumbUrls.end());
    m_strPictureURL.SetUrls(thumbUrls);
    m_strPictureURL.SetData(xmlAdd);
  }

  const std::string itemSeparator = CServiceBroker::GetSettingsComponent()->GetAdvancedSettings()->m_videoItemSeparator;

  std::vector<std::string> genres(m_genre);
  if (XMLUtils::GetStringArray(movie, "genre", genres, prioritise, itemSeparator))
    SetGenre(genres);

  std::vector<std::string> country(m_country);
  if (XMLUtils::GetStringArray(movie, "country", country, prioritise, itemSeparator))
    SetCountry(country);

  std::vector<std::string> credits(m_writingCredits);
  if (XMLUtils::GetStringArray(movie, "credits", credits, prioritise, itemSeparator))
    SetWritingCredits(credits);

  std::vector<std::string> director(m_director);
  if (XMLUtils::GetStringArray(movie, "director", director, prioritise, itemSeparator))
    SetDirector(director);

  std::vector<std::string> showLink(m_showLink);
  if (XMLUtils::GetStringArray(movie, "showlink", showLink, prioritise, itemSeparator))
    SetShowLink(showLink);

  const TiXmlElement* namedSeason = movie->FirstChildElement("namedseason");
  while (namedSeason != nullptr)
  {
    if (namedSeason->FirstChild() != nullptr)
    {
      int seasonNumber;
      std::string seasonName = namedSeason->FirstChild()->ValueStr();
      if (!seasonName.empty() &&
          namedSeason->Attribute("number", &seasonNumber) != nullptr)
        m_namedSeasons.insert(std::make_pair(seasonNumber, seasonName));
    }

    namedSeason = namedSeason->NextSiblingElement("namedseason");
  }

  // cast
  node = movie->FirstChildElement("actor");
  if (node && node->FirstChild() && prioritise)
    m_cast->clear();
  while (node)
  {
    const TiXmlNode *actor = node->FirstChild("name");
    if (actor && actor->FirstChild())
    {
      SActorInfo info;
      info.strName = actor->FirstChild()->Value();

      if (XMLUtils::GetString(node, "role", value))
        info.strRole = StringUtils::Trim(value);

      XMLUtils::GetInt(node, "order", info.order);
      const TiXmlElement* thumb = node->FirstChildElement("thumb");
      while (thumb)
      {
        info.thumbUrl.ParseAndAppendUrl(thumb);
        thumb = thumb->NextSiblingElement("thumb");
      }
      const char* clear=node->Attribute("clear");
      if (clear && StringUtils::CompareNoCase(clear, "true"))
        m_cast->clear();
      m_cast->push_back(info);
    }
    node = node->NextSiblingElement("actor");
  }

  // Pre-Jarvis NFO file:
  // <set>A set</set>
  if (XMLUtils::GetString(movie, "set", value))
    SetSet(value);
  // Jarvis+:
  // <set><name>A set</name><overview>A set with a number of movies...</overview></set>
  node = movie->FirstChildElement("set");
  if (node)
  {
    // No name, no set
    if (XMLUtils::GetString(node, "name", value))
    {
      SetSet(value);
      if (XMLUtils::GetString(node, "overview", value))
        SetSetOverview(value);
    }
  }

  std::vector<std::string> tags(m_tags);
  if (XMLUtils::GetStringArray(movie, "tag", tags, prioritise, itemSeparator))
    SetTags(tags);

  std::vector<std::string> studio(m_studio);
  if (XMLUtils::GetStringArray(movie, "studio", studio, prioritise, itemSeparator))
    SetStudio(studio);

  // artists
  std::vector<std::string> artist(m_artist);
  node = movie->FirstChildElement("artist");
  if (node && node->FirstChild() && prioritise)
    artist.clear();
  while (node)
  {
    const TiXmlNode* pNode = node->FirstChild("name");
    const char* pValue=NULL;
    if (pNode && pNode->FirstChild())
      pValue = pNode->FirstChild()->Value();
    else if (node->FirstChild())
      pValue = node->FirstChild()->Value();
    if (pValue)
    {
      const char* clear=node->Attribute("clear");
      if (clear && StringUtils::CompareNoCase(clear, "true") == 0)
        artist.clear();
      std::vector<std::string> newArtists = StringUtils::Split(pValue, itemSeparator);
      artist.insert(artist.end(), newArtists.begin(), newArtists.end());
    }
    node = node->NextSiblingElement("artist");
  }
  SetArtist(artist);

  node = movie->FirstChildElement("fileinfo");
  if (node)
  {
    // Try to pull from fileinfo/streamdetails/[video|audio|subtitle]
    const TiXmlNode *nodeStreamDetails = node->FirstChild("streamdetails");
    if (nodeStreamDetails)
    {
      const TiXmlNode *nodeDetail = NULL;
      while ((nodeDetail = nodeStreamDetails->IterateChildren("audio", nodeDetail)))
      {
        CStreamDetailAudio *p = new CStreamDetailAudio();
        if (XMLUtils::GetString(nodeDetail, "codec", value))
          p->m_strCodec = StringUtils::Trim(value);

        if (XMLUtils::GetString(nodeDetail, "language", value))
          p->m_strLanguage = StringUtils::Trim(value);

        XMLUtils::GetInt(nodeDetail, "channels", p->m_iChannels);
        StringUtils::ToLower(p->m_strCodec);
        StringUtils::ToLower(p->m_strLanguage);
        m_streamDetails->AddStream(p);
      }
      nodeDetail = NULL;
      while ((nodeDetail = nodeStreamDetails->IterateChildren("video", nodeDetail)))
      {
        CStreamDetailVideo *p = new CStreamDetailVideo();
        if (XMLUtils::GetString(nodeDetail, "codec", value))
          p->m_strCodec = StringUtils::Trim(value);

        XMLUtils::GetFloat(nodeDetail, "aspect", p->m_fAspect);
        XMLUtils::GetInt(nodeDetail, "width", p->m_iWidth);
        XMLUtils::GetInt(nodeDetail, "height", p->m_iHeight);
        XMLUtils::GetInt(nodeDetail, "durationinseconds", p->m_iDuration);
        if (XMLUtils::GetString(nodeDetail, "stereomode", value))
          p->m_strStereoMode = StringUtils::Trim(value);
        if (XMLUtils::GetString(nodeDetail, "language", value))
          p->m_strLanguage = StringUtils::Trim(value);

        StringUtils::ToLower(p->m_strCodec);
        StringUtils::ToLower(p->m_strStereoMode);
        StringUtils::ToLower(p->m_strLanguage);
        m_streamDetails->AddStream(p);
      }
      nodeDetail = NULL;
      while ((nodeDetail = nodeStreamDetails->IterateChildren("subtitle", nodeDetail)))
      {
        CStreamDetailSubtitle *p = new CStreamDetailSubtitle();
        if (XMLUtils::GetString(nodeDetail, "language", value))
          p->m_strLanguage = StringUtils::Trim(value);
        StringUtils::ToLower(p->m_strLanguage);
        m_streamDetails->AddStream(p);
      }
    }
    m_streamDetails->DetermineBestStreams();
  }  /* if fileinfo */

  const TiXmlElement *epguide = movie->FirstChildElement("episodeguide");
  if (epguide)
  {
    // DEPRECIATE ME - support for old XML-encoded <episodeguide> blocks.
    if (epguide->FirstChild() &&
        StringUtils::CompareNoCase("<episodeguide", epguide->FirstChild()->Value(), 13) == 0)
      m_strEpisodeGuide = epguide->FirstChild()->Value();
    else
    {
      std::stringstream stream;
      stream << *epguide;
      m_strEpisodeGuide = stream.str();
    }
  }

  // fanart
  const TiXmlElement *fanart = movie->FirstChildElement("fanart");
  if (fanart)
  {
    // we prioritise mixed-mode nfo's with fanart set
    if (prioritise)
    {
      std::string temp;
      temp << *fanart;
      m_fanart.m_xml = temp+m_fanart.m_xml;
    }
    else
      m_fanart.m_xml << *fanart;
    m_fanart.Unpack();
  }

  // resumePoint
  const TiXmlNode *resume = movie->FirstChild("resume");
  if (resume)
  {
    XMLUtils::GetDouble(resume, "position", m_resumePoint.timeInSeconds);
    XMLUtils::GetDouble(resume, "total", m_resumePoint.totalTimeInSeconds);
    const TiXmlElement *playerstate = resume->FirstChildElement("playerstate");
    if (playerstate)
    {
      const TiXmlElement *value = playerstate->FirstChildElement();
      if (value)
        m_resumePoint.playerState << *value;
    }
  }

  XMLUtils::GetDateTime(movie, "dateadded", m_dateAdded);
}

bool CVideoInfoTag::HasStreamDetails() const
{
  return m_streamDetails->HasItems();
}

CStreamDetails& CVideoInfoTag::GetStreamDetails()
{
  return m_streamDetails.value();
}

const CStreamDetails& CVideoInfoTag::GetStreamDetails() const
{
  return m_streamDetails.value();
}

void CVideoInfoTag::SetStreamDetails(CStreamDetails streamDetails)
{
  m_streamDetails = std::move(streamDetails);
}

bool CVideoInfoTag::IsEmpty() const
{
  return (m_strTitle.empty() &&
          m_strFile.empty() &&
          m_strPath.empty());
}

void CVideoInfoTag::SetDuration(int duration)
{
  m_duration = duration;
}

unsigned int CVideoInfoTag::GetDuration() const
{
  /*
   Prefer the duration from the stream if it isn't too
   small (60%) compared to the duration from the tag.
   */
  unsigned int duration = m_streamDetails->GetVideoDuration();
  if (duration > m_duration * 0.6)
    return duration;

  return m_duration;
}

unsigned int CVideoInfoTag::GetStaticDuration() const
{
  return m_duration;
}

unsigned int CVideoInfoTag::GetDurationFromMinuteString(const std::string &runtime)
{
  unsigned int duration = (unsigned int)str2uint64(runtime);
  if (!duration)
  { // failed for some reason, or zero
    duration = strtoul(runtime.c_str(), NULL, 10);
    CLog::Log(LOGWARNING, "%s <runtime> should be in minutes. Interpreting '%s' as %u minutes", __FUNCTION__, runtime.c_str(), duration);
  }
  return duration*60;
}

void CVideoInfoTag::SetBasePath(std::string basePath)
{
  m_basePath = Trim(std::move(basePath));
}

void CVideoInfoTag::SetDirector(std::vector<std::string> director)
{
  m_director = Trim(std::move(director));
}

void CVideoInfoTag::SetWritingCredits(std::vector<std::string> writingCredits)
{
  m_writingCredits = Trim(std::move(writingCredits));
}

void CVideoInfoTag::SetGenre(std::vector<std::string> genre)
{
  m_genre = Trim(std::move(genre));
}

void CVideoInfoTag::SetCountry(std::vector<std::string> country)
{
  m_country = Trim(std::move(country));
}

void CVideoInfoTag::SetTagLine(std::string tagLine)
{
  m_strTagLine = Trim(std::move(tagLine));
}

void CVideoInfoTag::SetPlotOutline(std::string plotOutline)
{
  m_strPlotOutline = Trim(std::move(plotOutline));
}

void CVideoInfoTag::SetTrailer(std::string trailer)
{
  m_strTrailer = Trim(std::move(trailer));
}

void CVideoInfoTag::SetPlot(std::string plot)
{
  m_strPlot = Trim(std::move(plot));
}

void CVideoInfoTag::SetTitle(std::string title)
{
  m_strTitle = Trim(std::move(title));
}

std::string const &CVideoInfoTag::GetTitle()
{
  return m_strTitle;
}

void CVideoInfoTag::SetSortTitle(std::string sortTitle)
{
  m_strSortTitle = Trim(std::move(sortTitle));
}

void CVideoInfoTag::SetPictureURL(CScraperUrl &pictureURL)
{
  m_strPictureURL = pictureURL;
}

void CVideoInfoTag::SetRating(float rating, int votes, const std::string& type /* = "" */, bool def /* = false */)
{
  SetRating(CRating(rating, votes), type, def);
}

void CVideoInfoTag::SetRating(CRating rating, const std::string& type /* = "" */, bool def /* = false */)
{
  if (rating.rating <= 0 || rating.rating > 10)
    return;

  m_ratings->SetValue(rating, type, def);
}

void CVideoInfoTag::SetRating(float rating, const std::string& type /* = "" */, bool def /* = false */)
{
  if (rating <= 0 || rating > 10)
    return;

  CRating rat;
  if (!m_ratings->TryGetValue(rat, type))
    return;

  rat.rating = rating;
  m_ratings->SetValue(rat, type, def);
}

void CVideoInfoTag::RemoveRating(const std::string& type)
{
  m_ratings->RemoveValue(type);
}

void CVideoInfoTag::SetRatings(RatingMap ratings)
{
  m_ratings->Set(std::move(ratings));
}

void CVideoInfoTag::SetVotes(int votes, const std::string& type /* = "" */)
{
  CRating rat;
  if (!m_ratings->TryGetValue(rat, type))
    return;

  rat.votes = votes;
  m_ratings->SetValue(rat, type);
}

void CVideoInfoTag::SetPremiered(CDateTime premiered)
{
  m_premiered = std::move(premiered);
  m_bHasPremiered = premiered.IsValid();
}

void CVideoInfoTag::SetPremieredFromDBDate(std::string premieredString)
{
  CDateTime premiered;
  premiered.SetFromDBDate(std::move(premieredString));
  SetPremiered(premiered);
}

void CVideoInfoTag::SetYear(int year)
{
  if (year <= 0)
    return;

  if (m_bHasPremiered)
    m_premiered.SetDate(year, m_premiered.GetMonth(), m_premiered.GetDay());
  else
    m_premiered = CDateTime(year, 1, 1, 0, 0, 0);
}

void CVideoInfoTag::SetArtist(std::vector<std::string> artist)
{
  m_artist = Trim(std::move(artist));
}

void CVideoInfoTag::SetUniqueIDs(std::map<std::string, std::string> uniqueIDs)
{
  const auto emptyUniqueID = uniqueIDs.find("");
  if (emptyUniqueID != uniqueIDs.end())
    uniqueIDs.erase(emptyUniqueID);

  m_uniqueIDs->Set(std::move(uniqueIDs));
}

void CVideoInfoTag::SetSet(std::string set)
{
  m_set.title = Trim(std::move(set));
}

void CVideoInfoTag::SetSetOverview(std::string setOverview)
{
  m_set.overview = Trim(std::move(setOverview));
}

void CVideoInfoTag::SetTags(std::vector<std::string> tags)
{
  m_tags = Trim(std::move(tags));
}

void CVideoInfoTag::SetFile(std::string file)
{
  m_strFile = Trim(std::move(file));
}

void CVideoInfoTag::SetPath(std::string path)
{
  m_strPath = Trim(std::move(path));
}

void CVideoInfoTag::SetMPAARating(std::string mpaaRating)
{
  m_strMPAARating = Trim(std::move(mpaaRating));
}

void CVideoInfoTag::SetFileNameAndPath(std::string fileNameAndPath)
{
  m_strFileNameAndPath = Trim(std::move(fileNameAndPath));
}

void CVideoInfoTag::SetOriginalTitle(std::string originalTitle)
{
  m_strOriginalTitle = Trim(std::move(originalTitle));
}

void CVideoInfoTag::SetEpisodeGuide(std::string episodeGuide)
{
  if (StringUtils::StartsWith(episodeGuide, "<episodeguide"))
    m_strEpisodeGuide = Trim(std::move(episodeGuide));
  else
    m_strEpisodeGuide = StringUtils::Format("<episodeguide>%s</episodeguide>", Trim(std::move(episodeGuide)).c_str());
}

void CVideoInfoTag::SetStatus(std::string status)
{
  m_strStatus = Trim(std::move(status));
}

void CVideoInfoTag::SetProductionCode(std::string productionCode)
{
  m_strProductionCode = Trim(std::move(productionCode));
}

void CVideoInfoTag::SetShowTitle(std::string showTitle)
{
  m_strShowTitle = Trim(std::move(showTitle));
}

void CVideoInfoTag::SetStudio(std::vector<std::string> studio)
{
  m_studio = Trim(std::move(studio));
}

void CVideoInfoTag::SetAlbum(std::string album)
{
  m_strAlbum = Trim(std::move(album));
}

void CVideoInfoTag::SetShowLink(std::vector<std::string> showLink)
{
  m_showLink = Trim(std::move(showLink));
}

void CVideoInfoTag::SetUniqueID(const std::string& uniqueid, const std::string& type /* = "" */, bool isDefaultID /* = false */)
{
  if (uniqueid.empty())
    return;

  m_uniqueIDs->SetValue(uniqueid, type, isDefaultID);
}

void CVideoInfoTag::RemoveUniqueID(const std::string& type)
{
  m_uniqueIDs->RemoveValue(type);
}

void CVideoInfoTag::SetNamedSeasons(std::map<int, std::string> namedSeasons)
{
  m_namedSeasons = std::move(namedSeasons);
}

void CVideoInfoTag::SetUserrating(int userrating)
{
  //This value needs to be between 0-10 - 0 will unset the userrating
  userrating = std::max(userrating, 0);
  userrating = std::min(userrating, 10);

  m_iUserRating = userrating;
}

std::string CVideoInfoTag::Trim(std::string &&value)
{
  return StringUtils::Trim(value);
}

std::vector<std::string> CVideoInfoTag::Trim(std::vector<std::string>&& items)
{
  std::for_each(items.begin(), items.end(), [](std::string &str){
    str = StringUtils::Trim(str);
  });
  return std::move(items);
}

int CVideoInfoTag::GetPlayCount() const
{
  return IsPlayCountSet() ? m_playCount : 0;
}

bool CVideoInfoTag::SetPlayCount(int count)
{
  m_playCount = count;
  return true;
}

bool CVideoInfoTag::IncrementPlayCount()
{
  if (!IsPlayCountSet())
    m_playCount = 0;

  m_playCount++;
  return true;
}

void CVideoInfoTag::ResetPlayCount()
{
  m_playCount = PLAYCOUNT_NOT_SET;
}

bool CVideoInfoTag::IsPlayCountSet() const
{
  return m_playCount != PLAYCOUNT_NOT_SET;
}

CBookmark CVideoInfoTag::GetResumePoint() const
{
  return m_resumePoint;
}

bool CVideoInfoTag::SetResumePoint(const CBookmark &resumePoint)
{
  m_resumePoint = resumePoint;
  return true;
}

bool CVideoInfoTag::SetResumePoint(double timeInSeconds, double totalTimeInSeconds, const std::string &playerState)
{
  CBookmark resumePoint;
  resumePoint.timeInSeconds = timeInSeconds;
  resumePoint.totalTimeInSeconds = totalTimeInSeconds;
  resumePoint.playerState = playerState;
  resumePoint.type = CBookmark::RESUME;

  m_resumePoint = resumePoint;
  return true;
}

void CVideoInfoTag::ClearCast()
{
  m_cast->clear();
}

void CVideoInfoTag::SetCast(std::vector<SActorInfo> cast)
{
  m_cast->swap(cast);
}

void CVideoInfoTag::AddActor(SActorInfo actor)
{
  m_cast->emplace_back(std::move(actor));
}
