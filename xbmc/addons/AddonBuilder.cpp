/*
 *      Copyright (C) 2016 Team Kodi
 *      http://kodi.tv
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

#include "addons/AddonBuilder.h"
#include "addons/AudioDecoder.h"
#include "addons/AudioEncoder.h"
#include "addons/ContextMenuAddon.h"
#include "addons/GameResource.h"
#include "addons/ImageDecoder.h"
#include "addons/ImageResource.h"
#include "addons/InputStream.h"
#include "addons/LanguageResource.h"
#include "addons/PluginSource.h"
#include "addons/Repository.h"
#include "addons/Scraper.h"
#include "addons/ScreenSaver.h"
#include "addons/Service.h"
#include "addons/Skin.h"
#include "addons/UISoundsResource.h"
#include "addons/VFSEntry.h"
#include "addons/Visualization.h"
#include "addons/Webinterface.h"
#include "cores/AudioEngine/Engines/ActiveAE/AudioDSPAddons/ActiveAEDSP.h"
#include "games/addons/GameClient.h"
#include "games/controllers/Controller.h"
#include "peripherals/addons/PeripheralAddon.h"
#include "addons/PVRClient.h"
#include "utils/StringUtils.h"

using namespace KODI;

namespace ADDON
{

std::shared_ptr<IAddon> CAddonBuilder::Build()
{
  if (m_built)
    throw std::logic_error("Already built");

  if (m_addonInfo->m_id.empty())
    return nullptr;

  m_built = true;

  if (m_addonInfo->m_mainType == ADDON_UNKNOWN)
    return std::make_shared<CAddon>(m_addonInfo);

  if (m_extPoint == nullptr)
    return FromProps(m_addonInfo);

  const TYPE type(m_addonInfo->m_mainType);

  // Handle screensaver special cases
  if (type == ADDON_SCREENSAVER)
  {
    // built in screensaver or python screensaver
    if (StringUtils::StartsWithNoCase(m_extPoint->plugin->identifier, "screensaver.xbmc.builtin.") ||
        URIUtils::HasExtension(CAddonMgr::GetInstance().GetExtValue(m_extPoint->configuration, "@library"), ".py"))
      return std::make_shared<CAddon>(m_addonInfo);
  }

  // Handle audio encoder special cases
  if (type == ADDON_AUDIOENCODER)
  {
    // built in audio encoder
    if (StringUtils::StartsWithNoCase(m_extPoint->plugin->identifier, "audioencoder.xbmc.builtin."))
      return CAudioEncoder::FromExtension(m_addonInfo, m_extPoint);
  }

  // Ensure binary types have a valid library for the platform
  if (type == ADDON_VIZ ||
      type == ADDON_SCREENSAVER ||
      type == ADDON_PVRDLL ||
      type == ADDON_ADSPDLL ||
      type == ADDON_AUDIOENCODER ||
      type == ADDON_AUDIODECODER ||
      type == ADDON_VFS ||
      type == ADDON_IMAGEDECODER ||
      type == ADDON_INPUTSTREAM ||
      type == ADDON_PERIPHERALDLL ||
      type == ADDON_GAMEDLL)
  {
    std::string value = CAddonMgr::GetInstance().GetPlatformLibraryName(m_extPoint->plugin->extensions->configuration);
    if (value.empty())
      return AddonPtr();
  }

  switch (type)
  {
    case ADDON_PLUGIN:
    case ADDON_SCRIPT:
      return CPluginSource::FromExtension(m_addonInfo, m_extPoint);
    case ADDON_SCRIPT_LIBRARY:
    case ADDON_SCRIPT_LYRICS:
    case ADDON_SCRIPT_MODULE:
    case ADDON_SUBTITLE_MODULE:
    case ADDON_SCRIPT_WEATHER:
      return std::make_shared<CAddon>(m_addonInfo);
    case ADDON_WEB_INTERFACE:
      return CWebinterface::FromExtension(m_addonInfo, m_extPoint);
    case ADDON_SERVICE:
      return CService::FromExtension(m_addonInfo, m_extPoint);
    case ADDON_SCRAPER_ALBUMS:
    case ADDON_SCRAPER_ARTISTS:
    case ADDON_SCRAPER_MOVIES:
    case ADDON_SCRAPER_MUSICVIDEOS:
    case ADDON_SCRAPER_TVSHOWS:
    case ADDON_SCRAPER_LIBRARY:
      return CScraper::FromExtension(m_addonInfo, m_extPoint);
    case ADDON_VIZ:
    case ADDON_SCREENSAVER:
      return std::make_shared<CAddonDll>(m_addonInfo);
#ifdef HAS_PVRCLIENTS
    case ADDON_PVRDLL:
      return PVR::CPVRClient::FromExtension(m_addonInfo, m_extPoint);
#endif
    case ADDON_ADSPDLL:
      return std::make_shared<ActiveAE::CActiveAEDSPAddon>(m_addonInfo);
    case ADDON_AUDIOENCODER:
      return CAudioEncoder::FromExtension(m_addonInfo, m_extPoint);
    case ADDON_AUDIODECODER:
      return CAudioDecoder::FromExtension(m_addonInfo, m_extPoint);
    case ADDON_IMAGEDECODER:
      return CImageDecoder::FromExtension(m_addonInfo, m_extPoint);
    case ADDON_INPUTSTREAM:
      return CInputStream::FromExtension(m_addonInfo, m_extPoint);
    case ADDON_PERIPHERALDLL:
      return PERIPHERALS::CPeripheralAddon::FromExtension(m_addonInfo, m_extPoint);
    case ADDON_GAMEDLL:
      return GAME::CGameClient::FromExtension(m_addonInfo, m_extPoint);
    case ADDON_VFS:
      return CVFSEntry::FromExtension(m_addonInfo, m_extPoint);
    case ADDON_SKIN:
      return CSkinInfo::FromExtension(m_addonInfo, m_extPoint);
    case ADDON_RESOURCE_IMAGES:
      return CImageResource::FromExtension(m_addonInfo, m_extPoint);
    case ADDON_RESOURCE_GAMES:
      return CGameResource::FromExtension(m_addonInfo, m_extPoint);
    case ADDON_RESOURCE_LANGUAGE:
      return CLanguageResource::FromExtension(m_addonInfo, m_extPoint);
    case ADDON_RESOURCE_UISOUNDS:
      return std::make_shared<CUISoundsResource>(m_addonInfo);
    case ADDON_REPOSITORY:
      return CRepository::FromExtension(m_addonInfo, m_extPoint);
    case ADDON_CONTEXT_ITEM:
      return CContextMenuAddon::FromExtension(m_addonInfo, m_extPoint);
    case ADDON_GAME_CONTROLLER:
      return GAME::CController::FromExtension(m_addonInfo, m_extPoint);
    default:
      break;
  }
  return AddonPtr();
}


AddonPtr CAddonBuilder::FromProps(const AddonInfoPtr& addonInfo)
{
  // FIXME: there is no need for this as none of the derived classes will contain any useful
  // information. We should return CAddon instances only, however there are several places that
  // down casts, which need to fixed first.
  switch (addonInfo->m_mainType)
  {
    case ADDON_PLUGIN:
    case ADDON_SCRIPT:
      return AddonPtr(new CPluginSource(addonInfo));
    case ADDON_SCRIPT_LIBRARY:
    case ADDON_SCRIPT_LYRICS:
    case ADDON_SCRIPT_WEATHER:
    case ADDON_SCRIPT_MODULE:
    case ADDON_SUBTITLE_MODULE:
      return AddonPtr(new CAddon(addonInfo));
    case ADDON_WEB_INTERFACE:
      return AddonPtr(new CWebinterface(addonInfo));
    case ADDON_SERVICE:
      return AddonPtr(new CService(addonInfo));
    case ADDON_SCRAPER_ALBUMS:
    case ADDON_SCRAPER_ARTISTS:
    case ADDON_SCRAPER_MOVIES:
    case ADDON_SCRAPER_MUSICVIDEOS:
    case ADDON_SCRAPER_TVSHOWS:
    case ADDON_SCRAPER_LIBRARY:
      return AddonPtr(new CScraper(addonInfo));
    case ADDON_SKIN:
      return AddonPtr(new CSkinInfo(addonInfo));
    case ADDON_VIZ:
    case ADDON_SCREENSAVER:
      return AddonPtr(new CAddonDll(addonInfo));
    case ADDON_PVRDLL:
      return AddonPtr(new PVR::CPVRClient(addonInfo));
    case ADDON_ADSPDLL:
      return AddonPtr(new ActiveAE::CActiveAEDSPAddon(addonInfo));
    case ADDON_AUDIOENCODER:
      return AddonPtr(new CAudioEncoder(addonInfo));
    case ADDON_AUDIODECODER:
      return AddonPtr(new CAudioDecoder(addonInfo));
    case ADDON_RESOURCE_IMAGES:
      return AddonPtr(new CImageResource(addonInfo));
    case ADDON_RESOURCE_GAMES:
      return AddonPtr(new CGameResource(addonInfo));
    case ADDON_RESOURCE_LANGUAGE:
      return AddonPtr(new CLanguageResource(addonInfo));
    case ADDON_RESOURCE_UISOUNDS:
      return AddonPtr(new CUISoundsResource(addonInfo));
    case ADDON_REPOSITORY:
      return AddonPtr(new CRepository(addonInfo));
    case ADDON_CONTEXT_ITEM:
      return AddonPtr(new CContextMenuAddon(addonInfo));
    case ADDON_INPUTSTREAM:
      return AddonPtr(new CInputStream(addonInfo));
    case ADDON_PERIPHERALDLL:
      return AddonPtr(new PERIPHERALS::CPeripheralAddon(addonInfo, false, false)); //! @todo implement
    case ADDON_GAME_CONTROLLER:
      return AddonPtr(new GAME::CController(addonInfo));
    case ADDON_GAMEDLL:
      return AddonPtr(new GAME::CGameClient(addonInfo));
    case ADDON_IMAGEDECODER:
      return AddonPtr(new CImageDecoder(addonInfo));
    case ADDON_VFS:
      return AddonPtr(new CVFSEntry(addonInfo,"","",false,false,false));
    default:
      break;
  }
  return AddonPtr();
}
}
