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

#include "FileOperations.h"
#include "VideoLibrary.h"
#include "AudioLibrary.h"
#include "MediaSource.h"
#include "filesystem/Directory.h"
#include "filesystem/File.h"
#include "FileItem.h"
#include "settings/AdvancedSettings.h"
#include "settings/MediaSourceSettings.h"
#include "Util.h"
#include "URL.h"
#include "utils/URIUtils.h"
#include "utils/FileUtils.h"
#include "Application.h"
#include "addons/AddonManager.h"
#include "addons/Scraper.h"
#include "video/VideoDatabase.h"
#include "video/VideoInfoScanner.h"
#include "music/MusicDatabase.h"
#include "PasswordManager.h"

using namespace XFILE;
using namespace JSONRPC;

JSONRPC_STATUS CFileOperations::GetRootDirectory(const std::string &method, ITransportLayer *transport, IClient *client, const CVariant &parameterObject, CVariant &result)
{
  std::string media = parameterObject["media"].asString();
  StringUtils::ToLower(media);

  VECSOURCES *sources = CMediaSourceSettings::Get().GetSources(media);
  if (sources)
  {
    CFileItemList items;
    for (unsigned int i = 0; i < (unsigned int)sources->size(); i++)
    {
      // Do not show sources which are locked
      if (sources->at(i).m_iHasLock == 2)
        continue;

      items.Add(CFileItemPtr(new CFileItem(sources->at(i))));
    }

    for (unsigned int i = 0; i < (unsigned int)items.Size(); i++)
    {
      if (items[i]->IsSmb())
      {
        CURL url(items[i]->GetPath());
        items[i]->SetPath(url.GetWithoutUserDetails());
      }
    }

    CVariant param = parameterObject;
    param["properties"] = CVariant(CVariant::VariantTypeArray);
    param["properties"].append("file");

    HandleFileItemList(NULL, true, "sources", items, param, result);
  }

  return OK;
}

JSONRPC_STATUS CFileOperations::GetDirectory(const std::string &method, ITransportLayer *transport, IClient *client, const CVariant &parameterObject, CVariant &result)
{
  std::string media = parameterObject["media"].asString();
  StringUtils::ToLower(media);

  CFileItemList items;
  std::string strPath = parameterObject["directory"].asString();

  if (!CFileUtils::RemoteAccessAllowed(strPath))
    return InvalidParams;

  std::vector<std::string> regexps;
  std::string extensions;
  if (media == "video")
  {
    regexps = g_advancedSettings.m_videoExcludeFromListingRegExps;
    extensions = g_advancedSettings.m_videoExtensions;
  }
  else if (media == "music")
  {
    regexps = g_advancedSettings.m_audioExcludeFromListingRegExps;
    extensions = g_advancedSettings.GetMusicExtensions();
  }
  else if (media == "pictures")
  {
    regexps = g_advancedSettings.m_pictureExcludeFromListingRegExps;
    extensions = g_advancedSettings.m_pictureExtensions;
  }

  if (CDirectory::GetDirectory(strPath, items, extensions))
  {
    // we might need to get additional information for music items
    if (media == "music")
    {
      JSONRPC_STATUS status = CAudioLibrary::GetAdditionalDetails(parameterObject, items);
      if (status != OK)
        return status;
    }

    CFileItemList filteredFiles;
    for (unsigned int i = 0; i < (unsigned int)items.Size(); i++)
    {
      if (CUtil::ExcludeFileOrFolder(items[i]->GetPath(), regexps))
        continue;

      if (items[i]->IsSmb())
      {
        CURL url(items[i]->GetPath());
        items[i]->SetPath(url.GetWithoutUserDetails());
      }

      if ((media == "video" && items[i]->HasVideoInfoTag()) ||
          (media == "music" && items[i]->HasMusicInfoTag()) ||
          (media == "picture" && items[i]->HasPictureInfoTag()) ||
           media == "files" ||
           URIUtils::IsUPnP(items.GetPath()))
          filteredFiles.Add(items[i]);
      else
      {
        CFileItemPtr fileItem(new CFileItem());
        if (FillFileItem(items[i], fileItem, media, parameterObject))
            filteredFiles.Add(fileItem);
        else
            filteredFiles.Add(items[i]);
      }
    }

    // Check if the "properties" list exists
    // and make sure it contains the "file" and "filetype"
    // fields
    CVariant param = parameterObject;
    if (!param.isMember("properties"))
      param["properties"] = CVariant(CVariant::VariantTypeArray);

    bool hasFileField = false;
    for (CVariant::const_iterator_array itr = param["properties"].begin_array(); itr != param["properties"].end_array(); itr++)
    {
      if (itr->asString().compare("file") == 0)
      {
        hasFileField = true;
        break;
      }
    }

    if (!hasFileField)
      param["properties"].append("file");
    param["properties"].append("filetype");

    HandleFileItemList("id", true, "files", filteredFiles, param, result);

    return OK;
  }

  return InvalidParams;
}

JSONRPC_STATUS CFileOperations::GetFileDetails(const std::string &method, ITransportLayer *transport, IClient *client, const CVariant &parameterObject, CVariant &result)
{
  std::string file = parameterObject["file"].asString();
  if (!CFile::Exists(file))
    return InvalidParams;

  if (!CFileUtils::RemoteAccessAllowed(file))
    return InvalidParams;

  std::string path = URIUtils::GetDirectory(file);

  CFileItemList items;
  if (path.empty() || !CDirectory::GetDirectory(path, items) || !items.Contains(file))
    return InvalidParams;

  CFileItemPtr item = items.Get(file);
  if (!URIUtils::IsUPnP(file))
    FillFileItem(item, item, parameterObject["media"].asString(), parameterObject);

  // Check if the "properties" list exists
  // and make sure it contains the "file"
  // field
  CVariant param = parameterObject;
  if (!param.isMember("properties"))
    param["properties"] = CVariant(CVariant::VariantTypeArray);

  bool hasFileField = false;
  for (CVariant::const_iterator_array itr = param["properties"].begin_array(); itr != param["properties"].end_array(); itr++)
  {
    if (itr->asString().compare("file") == 0)
    {
      hasFileField = true;
      break;
    }
  }

  if (!hasFileField)
    param["properties"].append("file");
  param["properties"].append("filetype");

  HandleFileItem("id", true, "filedetails", item, parameterObject, param["properties"], result, false);
  return OK;
}

JSONRPC_STATUS CFileOperations::PrepareDownload(const std::string &method, ITransportLayer *transport, IClient *client, const CVariant &parameterObject, CVariant &result)
{
  std::string protocol;
  if (transport->PrepareDownload(parameterObject["path"].asString().c_str(), result["details"], protocol))
  {
    result["protocol"] = protocol;

    if ((transport->GetCapabilities() & FileDownloadDirect) == FileDownloadDirect)
      result["mode"] = "direct";
    else
      result["mode"] = "redirect";

    return OK;
  }
  
  return InvalidParams;
}

JSONRPC_STATUS CFileOperations::Download(const std::string &method, ITransportLayer *transport, IClient *client, const CVariant &parameterObject, CVariant &result)
{
  return transport->Download(parameterObject["path"].asString().c_str(), result) ? OK : InvalidParams;
}

CONTENT_TYPE contentTypeFromString(const std::string &content) {
  if (content == "movies")
    return CONTENT_MOVIES;
  else if (content == "tvshows")
    return CONTENT_TVSHOWS;
  else if (content == "musicvideos")
    return CONTENT_MUSICVIDEOS;
  else
    return CONTENT_NONE;
}

JSONRPC_STATUS CFileOperations::AddSource(const std::string &method, ITransportLayer *transport, IClient *client, const CVariant &parameterObject, CVariant &result)
{
  std::string name = parameterObject["name"].asString();
  std::string content = parameterObject["content"].asString("none");
  std::vector<std::string> contents = StringUtils::Split(content, ".");

  CVariant directory = parameterObject["directory"];
  std::vector<std::string> paths;

  if (directory.isArray())
  {
    for (CVariant::iterator_array itr = directory.begin_array(); itr != directory.end_array(); itr++)
      paths.push_back(itr->asString());
  }
  else
    paths.push_back(directory.asString());

  std::string media = contents[0] == "audio" ? "music" : contents[0];

  for (std::vector<std::string>::iterator itr = paths.begin(); itr != paths.end(); itr++)
  {
    if (!itr->empty())
    { // strip off the user and password for smb paths (anything that the password manager can auth)
      // and add the user/pass to the password manager - note, we haven't confirmed that it works
      // at this point, but if it doesn't, the user will get prompted anyway in SMBDirectory.
      CURL url(*itr);
      if (url.IsProtocol("smb"))
      {
        CPasswordManager::GetInstance().SaveAuthenticatedURL(url);
        url.SetPassword("");
        url.SetUserName("");
      }
      itr->assign(url.Get());
    }
  }

  CMediaSource share;
  VECSOURCES* pShares = CMediaSourceSettings::Get().GetSources(media);
  for (VECSOURCES::iterator itr = pShares->begin(); itr != pShares->end(); itr++)
  {
    if (StringUtils::EqualsNoCase(itr->strName, name))
      return InvalidParams;
  }

  share.FromNameAndPaths(media, name, paths);

  CMediaSourceSettings::Get().AddShare(media, share);

  if (media == "video" && contents.size() > 1)
  {
    ADDON::AddonPtr scraperAddon;
    if (ADDON::CAddonMgr::Get().GetDefault(ADDON::ScraperTypeFromContent(ADDON::TranslateContent(contents[1])), scraperAddon))
    {
      ADDON::ScraperPtr scraper = boost::dynamic_pointer_cast<ADDON::CScraper>(scraperAddon);

      CVideoDatabase db;
      db.Open();

      VIDEO::SScanSettings settings;

      settings.parent_name      = parameterObject["parent_name"].asBoolean();
      settings.parent_name_root = parameterObject["parent_name_root"].asBoolean();
      settings.recurse          = parameterObject["recurse"].asBoolean();
      settings.noupdate         = parameterObject["noupdate"].asBoolean();
      settings.exclude          = parameterObject["exclude"].asBoolean();

      for (std::vector<std::string>::const_iterator itr = paths.begin(); itr != paths.end(); itr++)
        db.SetScraperForPath(*itr, scraper, settings);
    }
  }
  else if (content == "audio.music")
  {
    CMusicDatabase db;
    db.Open();

    for (std::vector<std::string>::const_iterator itr = paths.begin(); itr != paths.end(); itr++)
      db.AddPath(*itr);

    db.Close();
  }

  return ACK;
}

bool CFileOperations::FillFileItem(const CFileItemPtr &originalItem, CFileItemPtr &item, std::string media /* = "" */, const CVariant &parameterObject /* = CVariant(CVariant::VariantTypeArray) */)
{
  if (originalItem.get() == NULL)
    return false;

  // copy all the available details
  *item = *originalItem;

  bool status = false;
  std::string strFilename = originalItem->GetPath();
  if (!strFilename.empty() && (CDirectory::Exists(strFilename) || CFile::Exists(strFilename)))
  {
    if (media == "video")
      status = CVideoLibrary::FillFileItem(strFilename, item, parameterObject);
    else if (media == "music")
      status = CAudioLibrary::FillFileItem(strFilename, item, parameterObject);

    if (status && item->GetLabel().empty())
    {
      std::string label = originalItem->GetLabel();
      if (label.empty())
      {
        bool isDir = CDirectory::Exists(strFilename);
        label = CUtil::GetTitleFromPath(strFilename, isDir);
        if (label.empty())
          label = URIUtils::GetFileName(strFilename);
      }

      item->SetLabel(label);
    }
    else if (!status)
    {
      if (originalItem->GetLabel().empty())
      {
        bool isDir = CDirectory::Exists(strFilename);
        std::string label = CUtil::GetTitleFromPath(strFilename, isDir);
        if (label.empty())
          return false;

        item->SetLabel(label);
        item->SetPath(strFilename);
        item->m_bIsFolder = isDir;
      }
      else
        *item = *originalItem;

      status = true;
    }
  }

  return status;
}

bool CFileOperations::FillFileItemList(const CVariant &parameterObject, CFileItemList &list)
{
  if (parameterObject.isMember("directory"))
  {
    std::string media =  parameterObject["media"].asString();
    StringUtils::ToLower(media);

    std::string strPath = parameterObject["directory"].asString();
    if (!strPath.empty())
    {
      CFileItemList items;
      std::string extensions;
      std::vector<std::string> regexps;

      if (media == "video")
      {
        regexps = g_advancedSettings.m_videoExcludeFromListingRegExps;
        extensions = g_advancedSettings.m_videoExtensions;
      }
      else if (media == "music")
      {
        regexps = g_advancedSettings.m_audioExcludeFromListingRegExps;
        extensions = g_advancedSettings.GetMusicExtensions();
      }
      else if (media == "pictures")
      {
        regexps = g_advancedSettings.m_pictureExcludeFromListingRegExps;
        extensions = g_advancedSettings.m_pictureExtensions;
      }

      CDirectory directory;
      if (directory.GetDirectory(strPath, items, extensions))
      {
        items.Sort(SortByFile, SortOrderAscending);
        CFileItemList filteredDirectories;
        for (unsigned int i = 0; i < (unsigned int)items.Size(); i++)
        {
          if (CUtil::ExcludeFileOrFolder(items[i]->GetPath(), regexps))
            continue;

          if (items[i]->m_bIsFolder)
            filteredDirectories.Add(items[i]);
          else if ((media == "video" && items[i]->HasVideoInfoTag()) ||
                   (media == "music" && items[i]->HasMusicInfoTag()))
            list.Add(items[i]);
          else
          {
            CFileItemPtr fileItem(new CFileItem());
            if (FillFileItem(items[i], fileItem, media, parameterObject))
              list.Add(fileItem);
            else if (media == "files")
              list.Add(items[i]);
          }
        }

        if (parameterObject.isMember("recursive") && parameterObject["recursive"].isBoolean())
        {
          for (int i = 0; i < filteredDirectories.Size(); i++)
          {
            CVariant val = parameterObject;
            val["directory"] = filteredDirectories[i]->GetPath();
            FillFileItemList(val, list);
          }
        }

        return true;
      }
    }
  }

  return false;
}
