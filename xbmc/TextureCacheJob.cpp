/*
 *      Copyright (C) 2012-2013 Team XBMC
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

#ifndef ROOT_TEXTURECACHEJOB_H_INCLUDED
#define ROOT_TEXTURECACHEJOB_H_INCLUDED
#include "TextureCacheJob.h"
#endif

#ifndef ROOT_TEXTURECACHE_H_INCLUDED
#define ROOT_TEXTURECACHE_H_INCLUDED
#include "TextureCache.h"
#endif

#ifndef ROOT_GUILIB_TEXTURE_H_INCLUDED
#define ROOT_GUILIB_TEXTURE_H_INCLUDED
#include "guilib/Texture.h"
#endif

#ifndef ROOT_GUILIB_DDSIMAGE_H_INCLUDED
#define ROOT_GUILIB_DDSIMAGE_H_INCLUDED
#include "guilib/DDSImage.h"
#endif

#ifndef ROOT_SETTINGS_ADVANCEDSETTINGS_H_INCLUDED
#define ROOT_SETTINGS_ADVANCEDSETTINGS_H_INCLUDED
#include "settings/AdvancedSettings.h"
#endif

#ifndef ROOT_SETTINGS_SETTINGS_H_INCLUDED
#define ROOT_SETTINGS_SETTINGS_H_INCLUDED
#include "settings/Settings.h"
#endif

#ifndef ROOT_UTILS_LOG_H_INCLUDED
#define ROOT_UTILS_LOG_H_INCLUDED
#include "utils/log.h"
#endif

#ifndef ROOT_FILESYSTEM_FILE_H_INCLUDED
#define ROOT_FILESYSTEM_FILE_H_INCLUDED
#include "filesystem/File.h"
#endif

#ifndef ROOT_PICTURES_PICTURE_H_INCLUDED
#define ROOT_PICTURES_PICTURE_H_INCLUDED
#include "pictures/Picture.h"
#endif

#ifndef ROOT_UTILS_URIUTILS_H_INCLUDED
#define ROOT_UTILS_URIUTILS_H_INCLUDED
#include "utils/URIUtils.h"
#endif

#ifndef ROOT_UTILS_STRINGUTILS_H_INCLUDED
#define ROOT_UTILS_STRINGUTILS_H_INCLUDED
#include "utils/StringUtils.h"
#endif

#ifndef ROOT_URL_H_INCLUDED
#define ROOT_URL_H_INCLUDED
#include "URL.h"
#endif

#ifndef ROOT_FILEITEM_H_INCLUDED
#define ROOT_FILEITEM_H_INCLUDED
#include "FileItem.h"
#endif

#ifndef ROOT_MUSIC_MUSICTHUMBLOADER_H_INCLUDED
#define ROOT_MUSIC_MUSICTHUMBLOADER_H_INCLUDED
#include "music/MusicThumbLoader.h"
#endif

#ifndef ROOT_MUSIC_TAGS_MUSICINFOTAG_H_INCLUDED
#define ROOT_MUSIC_TAGS_MUSICINFOTAG_H_INCLUDED
#include "music/tags/MusicInfoTag.h"
#endif

#if defined(HAS_OMXPLAYER)
#include "cores/omxplayer/OMXImage.h"
#endif

CTextureCacheJob::CTextureCacheJob(const CStdString &url, const CStdString &oldHash)
{
  m_url = url;
  m_oldHash = oldHash;
  m_cachePath = CTextureCache::GetCacheFile(m_url);
}

CTextureCacheJob::~CTextureCacheJob()
{
}

bool CTextureCacheJob::operator==(const CJob* job) const
{
  if (strcmp(job->GetType(),GetType()) == 0)
  {
    const CTextureCacheJob* cacheJob = dynamic_cast<const CTextureCacheJob*>(job);
    if (cacheJob && cacheJob->m_cachePath == m_cachePath)
      return true;
  }
  return false;
}

bool CTextureCacheJob::DoWork()
{
  if (ShouldCancel(0, 0))
    return false;
  if (ShouldCancel(1, 0)) // HACK: second check is because we cancel the job in the first callback, but we don't detect it
    return false;         //       until the second

  // check whether we need cache the job anyway
  bool needsRecaching = false;
  CStdString path(CTextureCache::Get().CheckCachedImage(m_url, false, needsRecaching));
  if (!path.empty() && !needsRecaching)
    return false;
  return CacheTexture();
}

bool CTextureCacheJob::CacheTexture(CBaseTexture **out_texture)
{
  // unwrap the URL as required
  std::string additional_info;
  unsigned int width, height;
  CStdString image = DecodeImageURL(m_url, width, height, additional_info);

  m_details.updateable = additional_info != "music" && UpdateableURL(image);

  // generate the hash
  m_details.hash = GetImageHash(image);
  if (m_details.hash.empty())
    return false;
  else if (m_details.hash == m_oldHash)
    return true;

#if defined(HAS_OMXPLAYER)
  if (COMXImage::CreateThumb(image, width, height, additional_info, CTextureCache::GetCachedPath(m_cachePath + ".jpg")))
  {
    m_details.width = width;
    m_details.height = height;
    m_details.file = m_cachePath + ".jpg";
    if (out_texture)
      *out_texture = LoadImage(CTextureCache::GetCachedPath(m_details.file), width, height, additional_info);
    CLog::Log(LOGDEBUG, "Fast %s image '%s' to '%s': %p", m_oldHash.empty() ? "Caching" : "Recaching", image.c_str(), m_details.file.c_str(), out_texture);
    return true;
  }
#endif
  CBaseTexture *texture = LoadImage(image, width, height, additional_info, true);
  if (texture)
  {
    if (texture->HasAlpha())
      m_details.file = m_cachePath + ".png";
    else
      m_details.file = m_cachePath + ".jpg";

    CLog::Log(LOGDEBUG, "%s image '%s' to '%s':", m_oldHash.empty() ? "Caching" : "Recaching", image.c_str(), m_details.file.c_str());

    if (CPicture::CacheTexture(texture, width, height, CTextureCache::GetCachedPath(m_details.file)))
    {
      m_details.width = width;
      m_details.height = height;
      if (out_texture) // caller wants the texture
        *out_texture = texture;
      else
        delete texture;
      return true;
    }
  }
  delete texture;
  return false;
}

CStdString CTextureCacheJob::DecodeImageURL(const CStdString &url, unsigned int &width, unsigned int &height, std::string &additional_info)
{
  // unwrap the URL as required
  CStdString image(url);
  additional_info.clear();
  width = height = 0;
  if (StringUtils::StartsWith(url, "image://"))
  {
    // format is image://[type@]<url_encoded_path>?options
    CURL thumbURL(url);

    if (!CTextureCache::CanCacheImageURL(thumbURL))
      return "";
    if (thumbURL.GetUserName() == "music")
      additional_info = "music";

    image = thumbURL.GetHostName();

    CStdString optionString = thumbURL.GetOptions().empty() ? "" : thumbURL.GetOptions().substr(1);
    StringUtils::TrimRight(optionString, "/"); // In case XBMC adds a slash.

    std::vector<CStdString> options;
    StringUtils::SplitString(optionString, "&", options);
    for (std::vector<CStdString>::iterator i = options.begin(); i != options.end(); i++)
    {
      CStdString option, value;
      size_t pos = i->find('=');
      if (pos != std::string::npos)
      {
        option = i->substr(0, pos);
        value  = i->substr(pos + 1);
      }
      else
      {
        option = *i;
        value = "";
      }
      if (option == "size" && value == "thumb")
      {
        width = height = g_advancedSettings.GetThumbSize();
      }
      else if (option == "flipped")
      {
        additional_info = "flipped";
      }
    }
  }
  return image;
}

CBaseTexture *CTextureCacheJob::LoadImage(const CStdString &image, unsigned int width, unsigned int height, const std::string &additional_info, bool requirePixels)
{
  if (additional_info == "music")
  { // special case for embedded music images
    MUSIC_INFO::EmbeddedArt art;
    if (CMusicThumbLoader::GetEmbeddedThumb(image, art))
      return CBaseTexture::LoadFromFileInMemory(&art.data[0], art.size, art.mime, width, height);
  }

  // Validate file URL to see if it is an image
  CFileItem file(image, false);
  file.FillInMimeType();
  if (!(file.IsPicture() && !(file.IsZIP() || file.IsRAR() || file.IsCBR() || file.IsCBZ() ))
      && !StringUtils::StartsWithNoCase(file.GetMimeType(), "image/") && !file.GetMimeType().Equals("application/octet-stream")) // ignore non-pictures
    return NULL;

  CBaseTexture *texture = CBaseTexture::LoadFromFile(image, width, height, CSettings::Get().GetBool("pictures.useexifrotation"), requirePixels, file.GetMimeType());
  if (!texture)
    return NULL;

  // EXIF bits are interpreted as: <flipXY><flipY*flipX><flipX>
  // where to undo the operation we apply them in reverse order <flipX>*<flipY*flipX>*<flipXY>
  // When flipped we have an additional <flipX> on the left, which is equivalent to toggling the last bit
  if (additional_info == "flipped")
    texture->SetOrientation(texture->GetOrientation() ^ 1);

  return texture;
}

bool CTextureCacheJob::UpdateableURL(const CStdString &url) const
{
  // we don't constantly check online images
  if (StringUtils::StartsWith(url, "http://") ||
      StringUtils::StartsWith(url, "https://"))
    return false;
  return true;
}

CStdString CTextureCacheJob::GetImageHash(const CStdString &url)
{
  struct __stat64 st;
  if (XFILE::CFile::Stat(url, &st) == 0)
  {
    int64_t time = st.st_mtime;
    if (!time)
      time = st.st_ctime;
    if (time || st.st_size)
      return StringUtils::Format("d%"PRId64"s%"PRId64, time, st.st_size);;

  }
  CLog::Log(LOGDEBUG, "%s - unable to stat url %s", __FUNCTION__, url.c_str());
  return "";
}

CTextureDDSJob::CTextureDDSJob(const CStdString &original)
{
  m_original = original;
}

bool CTextureDDSJob::operator==(const CJob* job) const
{
  if (strcmp(job->GetType(),GetType()) == 0)
  {
    const CTextureDDSJob* ddsJob = dynamic_cast<const CTextureDDSJob*>(job);
    if (ddsJob && ddsJob->m_original == m_original)
      return true;
  }
  return false;
}

bool CTextureDDSJob::DoWork()
{
  if (URIUtils::HasExtension(m_original, ".dds"))
    return false;
  CBaseTexture *texture = CBaseTexture::LoadFromFile(m_original);
  if (texture)
  { // convert to DDS
    CDDSImage dds;
    CLog::Log(LOGDEBUG, "Creating DDS version of: %s", m_original.c_str());
    bool ret = dds.Create(URIUtils::ReplaceExtension(m_original, ".dds"), texture->GetWidth(), texture->GetHeight(), texture->GetPitch(), texture->GetPixels(), 40);
    delete texture;
    return ret;
  }
  return false;
}

CTextureUseCountJob::CTextureUseCountJob(const std::vector<CTextureDetails> &textures) : m_textures(textures)
{
}

bool CTextureUseCountJob::operator==(const CJob* job) const
{
  if (strcmp(job->GetType(),GetType()) == 0)
  {
    const CTextureUseCountJob* useJob = dynamic_cast<const CTextureUseCountJob*>(job);
    if (useJob && useJob->m_textures == m_textures)
      return true;
  }
  return false;
}

bool CTextureUseCountJob::DoWork()
{
  CTextureDatabase db;
  if (db.Open())
  {
    db.BeginTransaction();
    for (std::vector<CTextureDetails>::const_iterator i = m_textures.begin(); i != m_textures.end(); ++i)
      db.IncrementUseCount(*i);
    db.CommitTransaction();
  }
  return true;
}
