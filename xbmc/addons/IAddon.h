/*
 *  Copyright (C) 2005-2018 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#pragma once

#include "addons/addoninfo/AddonInfo.h"

#include <memory>
#include <set>

class TiXmlElement;

namespace ADDON
{

using AddonInstanceId = uint32_t;

/*!
 * @brief To set on @ref IAddon::OnPostInstall additional values, these can vary
 * between individual add-on types.
 */
enum class AddonOptPostInstValue
{
  //! Default value if unused on add-on type.
  UNUSED = -1
};

constexpr const char* ADDON_SETTING_INSTANCE_GROUP = "kodi_addon_instance";
constexpr const char* ADDON_SETTING_INSTANCE_NAME_VALUE = "kodi_addon_instance_name";
constexpr const char* ADDON_SETTING_INSTANCE_ENABLED_VALUE = "kodi_addon_instance_enabled";

/*!
 * @brief Identifier denoting default add-on instance.
 *
 * All numbers greater than 0 denote add-ons with support for multiple instances.
 */
constexpr AddonInstanceId ADDON_SINGLETON_INSTANCE_ID = 0;

/*!
 * @brief Identifier denoting initial first add-on instance.
 */
constexpr AddonInstanceId ADDON_FIRST_INSTANCE_ID = 1;

/*!
 * @brief Identifier denoting add-on instance id as unused.
 *
 * @sa ADDON::IAddonInstanceHandler
 */
constexpr AddonInstanceId ADDON_INSTANCE_ID_UNUSED = ADDON_SINGLETON_INSTANCE_ID;

/*!
 * @brief Identifier denoting default add-on settings.xml.
 *
 * All numbers greater than 0 denote add-on instances with an individual set of settings.
 */
constexpr AddonInstanceId ADDON_SETTINGS_ID = ADDON_SINGLETON_INSTANCE_ID;

class IAddon;
typedef std::shared_ptr<IAddon> AddonPtr;
class CInstanceVisualization;
typedef std::shared_ptr<CInstanceVisualization> VizPtr;
class CSkinInfo;
typedef std::shared_ptr<CSkinInfo> SkinPtr;
class CPluginSource;
typedef std::shared_ptr<CPluginSource> PluginPtr;

class CAddonMgr;
class CAddonSettings;

class IAddon : public std::enable_shared_from_this<IAddon>
{
public:
  virtual ~IAddon() = default;
  virtual TYPE MainType() const = 0;
  virtual TYPE Type() const = 0;
  virtual bool HasType(TYPE type) const = 0;
  virtual bool HasMainType(TYPE type) const = 0;
  virtual std::string ID() const = 0;
  virtual std::string Name() const = 0;
  virtual bool IsInUse() const = 0;
  virtual bool IsBinary() const = 0;
  virtual AddonVersion Version() const = 0;
  virtual AddonVersion MinVersion() const = 0;
  virtual std::string Summary() const = 0;
  virtual std::string Description() const = 0;
  virtual std::string Path() const = 0;
  virtual std::string Profile() const = 0;
  virtual std::string LibPath() const = 0;
  virtual std::string ChangeLog() const = 0;
  virtual std::string FanArt() const = 0;
  virtual ArtMap Art() const = 0;
  virtual std::vector<std::string> Screenshots() const = 0;
  virtual std::string Author() const = 0;
  virtual std::string Icon() const = 0;
  virtual std::string Disclaimer() const = 0;
  virtual AddonLifecycleState LifecycleState() const = 0;
  virtual std::string LifecycleStateDescription() const = 0;
  virtual CDateTime InstallDate() const = 0;
  virtual CDateTime LastUpdated() const = 0;
  virtual CDateTime LastUsed() const = 0;
  virtual std::string Origin() const = 0;
  virtual std::string OriginName() const = 0;
  virtual uint64_t PackageSize() const = 0;
  virtual const InfoMap& ExtraInfo() const = 0;
  virtual bool SupportsMultipleInstances() const = 0;
  virtual AddonInstanceSupport InstanceUseType() const = 0;
  virtual std::vector<AddonInstanceId> GetKnownInstanceIds() const = 0;
  virtual bool SupportsInstanceSettings() const = 0;
  virtual bool DeleteInstanceSettings(AddonInstanceId instance) = 0;
  virtual bool CanHaveAddonOrInstanceSettings() = 0;
  virtual bool HasSettings(AddonInstanceId id = ADDON_SETTINGS_ID) = 0;
  virtual bool HasUserSettings(AddonInstanceId id = ADDON_SETTINGS_ID) = 0;
  virtual void SaveSettings(AddonInstanceId id = ADDON_SETTINGS_ID) = 0;
  virtual void UpdateSetting(const std::string& key,
                             const std::string& value,
                             AddonInstanceId id = ADDON_SETTINGS_ID) = 0;
  virtual bool UpdateSettingBool(const std::string& key,
                                 bool value,
                                 AddonInstanceId id = ADDON_SETTINGS_ID) = 0;
  virtual bool UpdateSettingInt(const std::string& key,
                                int value,
                                AddonInstanceId id = ADDON_SETTINGS_ID) = 0;
  virtual bool UpdateSettingNumber(const std::string& key,
                                   double value,
                                   AddonInstanceId id = ADDON_SETTINGS_ID) = 0;
  virtual bool UpdateSettingString(const std::string& key,
                                   const std::string& value,
                                   AddonInstanceId id = ADDON_SETTINGS_ID) = 0;
  virtual std::string GetSetting(const std::string& key,
                                 AddonInstanceId id = ADDON_SETTINGS_ID) = 0;
  virtual bool GetSettingBool(const std::string& key,
                              bool& value,
                              AddonInstanceId id = ADDON_SETTINGS_ID) = 0;
  virtual bool GetSettingInt(const std::string& key,
                             int& value,
                             AddonInstanceId id = ADDON_SETTINGS_ID) = 0;
  virtual bool GetSettingNumber(const std::string& key,
                                double& value,
                                AddonInstanceId id = ADDON_SETTINGS_ID) = 0;
  virtual bool GetSettingString(const std::string& key,
                                std::string& value,
                                AddonInstanceId id = ADDON_SETTINGS_ID) = 0;
  virtual std::shared_ptr<CAddonSettings> GetSettings(AddonInstanceId id = ADDON_SETTINGS_ID) = 0;
  virtual const std::vector<DependencyInfo>& GetDependencies() const = 0;
  virtual AddonVersion GetDependencyVersion(const std::string& dependencyID) const = 0;
  virtual bool MeetsVersion(const AddonVersion& versionMin, const AddonVersion& version) const = 0;
  virtual bool ReloadSettings(AddonInstanceId id = ADDON_SETTINGS_ID) = 0;
  virtual void ResetSettings(AddonInstanceId id = ADDON_SETTINGS_ID) = 0;
  virtual AddonPtr GetRunningInstance() const = 0;
  virtual void OnPreInstall() = 0;
  virtual void OnPostInstall(bool update, bool modal, AddonOptPostInstValue optValue) = 0;
  virtual void OnPreUnInstall() = 0;
  virtual void OnPostUnInstall() = 0;
};

}; // namespace ADDON
