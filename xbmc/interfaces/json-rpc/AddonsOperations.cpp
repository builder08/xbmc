/*
 *      Copyright (C) 2011-2013 Team XBMC
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

#include "AddonsOperations.h"
#include "JSONUtils.h"
#include "addons/AddonManager.h"
#include "addons/AddonDatabase.h"
#include "addons/PluginSource.h"
#include "ApplicationMessenger.h"
#include "TextureCache.h"
#include "filesystem/File.h"

using namespace std;
using namespace JSONRPC;
using namespace ADDON;
using namespace XFILE;

JSONRPC_STATUS CAddonsOperations::GetAddons(const CStdString &method, ITransportLayer *transport, IClient *client, const CVariant &parameterObject, CVariant &result)
{
  vector<TYPE> addonTypes;
  TYPE addonType = TranslateType(parameterObject["type"].asString());
  CPluginSource::Content content = CPluginSource::Translate(parameterObject["content"].asString());
  CVariant enabled = parameterObject["enabled"];

  // ignore the "content" parameter if the type is specified but not a plugin or script
  if (addonType != ADDON_UNKNOWN && addonType != ADDON_PLUGIN && addonType != ADDON_SCRIPT)
    content = CPluginSource::UNKNOWN;

  if (addonType >= ADDON_VIDEO && addonType <= ADDON_EXECUTABLE)
  {
    addonTypes.push_back(ADDON_PLUGIN);
    addonTypes.push_back(ADDON_SCRIPT);

    switch (addonType)
    {
    case ADDON_VIDEO:
      content = CPluginSource::VIDEO;
      break;
    case ADDON_AUDIO:
      content = CPluginSource::AUDIO;
      break;
    case ADDON_IMAGE:
      content = CPluginSource::IMAGE;
      break;
    case ADDON_EXECUTABLE:
      content = CPluginSource::EXECUTABLE;
      break;

    default:
      break;
    }
  }
  else
    addonTypes.push_back(addonType);

  VECADDONS addons;
  for (vector<TYPE>::const_iterator typeIt = addonTypes.begin(); typeIt != addonTypes.end(); typeIt++)
  {
    VECADDONS typeAddons;
    if (*typeIt == ADDON_UNKNOWN)
    {
      if (!enabled.isBoolean())
      {
        CAddonMgr::Get().GetAllAddons(typeAddons, false);
        CAddonMgr::Get().GetAllAddons(typeAddons, true);
      }
      else
        CAddonMgr::Get().GetAllAddons(typeAddons, enabled.asBoolean());
    }
    else
    {
      if (!enabled.isBoolean())
      {
        CAddonMgr::Get().GetAddons(*typeIt, typeAddons, false);
        VECADDONS enabledAddons;
        CAddonMgr::Get().GetAddons(*typeIt, enabledAddons, true);
        typeAddons.insert(typeAddons.end(), enabledAddons.begin(), enabledAddons.end());
      }
      else
        CAddonMgr::Get().GetAddons(*typeIt, typeAddons, enabled.asBoolean());
    }

    addons.insert(addons.end(), typeAddons.begin(), typeAddons.end());
  }

  // remove library addons
  for (int index = 0; index < (int)addons.size(); index++)
  {
    PluginPtr plugin;
    if (content != CPluginSource::UNKNOWN)
      plugin = boost::dynamic_pointer_cast<CPluginSource>(addons.at(index));

    if ((addons.at(index)->Type() <= ADDON_UNKNOWN || addons.at(index)->Type() >= ADDON_VIZ_LIBRARY) ||
       ((content != CPluginSource::UNKNOWN && plugin == NULL) || (plugin != NULL && !plugin->Provides(content))))
    {
      addons.erase(addons.begin() + index);
      index--;
    }
  }

  int start, end;
  HandleLimits(parameterObject, result, addons.size(), start, end);
  
  CAddonDatabase addondb;
  for (int index = start; index < end; index++)
    FillDetails(addons.at(index), parameterObject["properties"], result["addons"], addondb, true);
  
  return OK;
}

JSONRPC_STATUS CAddonsOperations::GetAddonDetails(const CStdString &method, ITransportLayer *transport, IClient *client, const CVariant &parameterObject, CVariant &result)
{
  string id = parameterObject["addonid"].asString();
  AddonPtr addon;
  if (!CAddonMgr::Get().GetAddon(id, addon, ADDON::ADDON_UNKNOWN, false) || addon.get() == NULL ||
      addon->Type() <= ADDON_UNKNOWN || addon->Type() >= ADDON_VIZ_LIBRARY)
    return InvalidParams;
    
  CAddonDatabase addondb;
  FillDetails(addon, parameterObject["properties"], result["addon"], addondb);

  return OK;
}

JSONRPC_STATUS CAddonsOperations::SetAddonEnabled(const CStdString &method, ITransportLayer *transport, IClient *client, const CVariant &parameterObject, CVariant &result)
{
  CAddonDatabase addondatabase;
  if (!addondatabase.Open())
    return InternalError;

  string id = parameterObject["addonid"].asString();
  bool disabled = false;
  if (parameterObject["enabled"].isBoolean())
    disabled = !parameterObject["enabled"].asBoolean();
  // we need to toggle the current disabled state of the addon
  else if (parameterObject["enabled"].isString())
    disabled = !addondatabase.IsAddonDisabled(id);
  else
    return InvalidParams;

  if (!addondatabase.DisableAddon(id, disabled))
      return InvalidParams;

  return ACK;
}

JSONRPC_STATUS CAddonsOperations::ExecuteAddon(const CStdString &method, ITransportLayer *transport, IClient *client, const CVariant &parameterObject, CVariant &result)
{
  string id = parameterObject["addonid"].asString();
  AddonPtr addon;
  if (!CAddonMgr::Get().GetAddon(id, addon) || addon.get() == NULL ||
      addon->Type() < ADDON_VIZ || addon->Type() >= ADDON_VIZ_LIBRARY)
    return InvalidParams;
    
  string argv;
  CVariant params = parameterObject["params"];
  if (params.isObject())
  {
    for (CVariant::const_iterator_map it = params.begin_map(); it != params.end_map(); it++)
    {
      if (it != params.begin_map())
        argv += ",";
      argv += it->first + "=" + it->second.asString();
    }
  }
  else if (params.isArray())
  {
    for (CVariant::const_iterator_array it = params.begin_array(); it != params.end_array(); it++)
    {
      if (it != params.begin_array())
        argv += ",";
      argv += it->asString();
    }
  }
  
  CStdString cmd;
  if (params.size() == 0)
    cmd.Format("RunAddon(%s)", id.c_str());
  else
    cmd.Format("RunAddon(%s, %s)", id.c_str(), argv.c_str());
  CApplicationMessenger::Get().ExecBuiltIn(cmd, parameterObject["wait"].asBoolean());
  
  return ACK;
}

void CAddonsOperations::FillDetails(AddonPtr addon, const CVariant& fields, CVariant &result, CAddonDatabase &addondb, bool append /* = false */)
{
  if (addon.get() == NULL)
    return;
  
  CVariant addonInfo;
  addon->Props().Serialize(addonInfo);

  CVariant object;
  object["addonid"] = addonInfo["addonid"];
  object["type"] = addonInfo["type"];
  
  for (unsigned int index = 0; index < fields.size(); index++)
  {
    string field = fields[index].asString();
    
    // we need to manually retrieve the enabled state of every addon
    // from the addon database because it can't be read from addon.xml
    if (field == "enabled")
    {
      if (!addondb.IsOpen() && !addondb.Open())
        return;
      object[field] = !addondb.IsAddonDisabled(addon->ID());
    }
    else if (field == "fanart" || field == "thumbnail")
    {
      CStdString url = addonInfo[field].asString();
      // We need to check the existence of fanart and thumbnails as the addon simply
      // holds where the art will be, not whether it exists.
      bool needsRecaching;
      CStdString image = CTextureCache::Get().CheckCachedImage(url, false, needsRecaching);
      if (!image.empty() || CFile::Exists(url))
        object[field] = CTextureCache::Get().GetWrappedImageURL(url);
      else
        object[field] = "";
    }
    else if (addonInfo.isMember(field))
      object[field] = addonInfo[field];
  }
  
  if (append)
    result.append(object);
  else
    result = object;
}
