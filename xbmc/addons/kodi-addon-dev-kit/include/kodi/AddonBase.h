/*
 *  Copyright (C) 2005-2018 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#pragma once

#include "c-api/addon_base.h"
#include "versions.h"

#include <assert.h> /* assert */
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <memory>
#include <stdarg.h> /* va_list, va_start, va_arg, va_end */
#include <stdexcept>
#include <string>
#include <vector>

namespace kodi
{

namespace gui
{
struct IRenderHelper;
} // namespace gui

//==============================================================================
/// @ingroup cpp_kodi_addon_addonbase_Defs
/// @defgroup cpp_kodi_addon_addonbase_Defs_CSettingValue class CSettingValue
/// @brief Inside addon main instance used helper class to give settings value.
///
/// This is used on @ref addon::CAddonBase::SetSetting() to inform addon about
/// settings change by used. This becomes then used to give the related value
/// name.
///
/// ----------------------------------------------------------------------------
///
/// @copydetails cpp_kodi_addon_addonbase_Defs_CSettingValue_Help
///
/// ----------------------------------------------------------------------------
///
/// **Here is a code example how this is used:**
///
/// ~~~~~~~~~~~~~{.cpp}
/// #include <kodi/AddonBase.h>
///
/// enum myEnumValue
/// {
///   valueA,
///   valueB,
///   valueC
/// };
///
/// std::string m_myStringValue;
/// int m_myIntegerValue;
/// bool m_myBooleanValue;
/// float m_myFloatingPointValue;
/// myEnumValue m_myEnumValue;
///
///
/// ADDON_STATUS CMyAddon::SetSetting(const std::string& settingName, const kodi::CSettingValue& settingValue)
/// {
///   if (settingName == "my_string_value")
///     m_myStringValue = settingValue.GetString();
///   else if (settingName == "my_integer_value")
///     m_myIntegerValue = settingValue.GetInt();
///   else if (settingName == "my_boolean_value")
///     m_myBooleanValue = settingValue.GetBoolean();
///   else if (settingName == "my_float_value")
///     m_myFloatingPointValue = settingValue.GetFloat();
///   else if (settingName == "my_enum_value")
///     m_myEnumValue = settingValue.GetEnum<myEnumValue>();
/// }
/// ~~~~~~~~~~~~~
///
/// @note The asked type should match the type used on settings.xml.
///
///@{
class CSettingValue
{
public:
  explicit CSettingValue(const void* settingValue) : m_settingValue(settingValue) {}

  bool empty() const { return (m_settingValue == nullptr) ? true : false; }

  /// @defgroup cpp_kodi_addon_addonbase_Defs_CSettingValue_Help Value Help
  /// @ingroup cpp_kodi_addon_addonbase_Defs_CSettingValue
  ///
  /// <b>The following table contains values that can be set with @ref cpp_kodi_addon_addonbase_Defs_CSettingValue :</b>
  /// | Name | Type | Get call
  /// |------|------|----------
  /// | **Settings value as string** | `std::string` | @ref CSettingValue::GetString "GetString"
  /// | **Settings value as integer** | `int` | @ref CSettingValue::GetInt "GetInt"
  /// | **Settings value as unsigned integer** | `unsigned int` | @ref CSettingValue::GetUInt "GetUInt"
  /// | **Settings value as boolean** | `bool` | @ref CSettingValue::GetBoolean "GetBoolean"
  /// | **Settings value as floating point** | `float` | @ref CSettingValue::GetFloat "GetFloat"
  /// | **Settings value as enum** | `enum` | @ref CSettingValue::GetEnum "GetEnum"

  /// @addtogroup cpp_kodi_addon_addonbase_Defs_CSettingValue
  ///@{

  /// @brief To get settings value as string.
  std::string GetString() const { return (const char*)m_settingValue; }

  /// @brief To get settings value as integer.
  int GetInt() const { return *(const int*)m_settingValue; }

  /// @brief To get settings value as unsigned integer.
  unsigned int GetUInt() const { return *(const unsigned int*)m_settingValue; }

  /// @brief To get settings value as boolean.
  bool GetBoolean() const { return *(const bool*)m_settingValue; }

  /// @brief To get settings value as floating point.
  float GetFloat() const { return *(const float*)m_settingValue; }

  /// @brief To get settings value as enum.
  /// @note Inside settings.xml them stored as integer.
  template<typename enumType>
  enumType GetEnum() const
  {
    return static_cast<enumType>(*(const int*)m_settingValue);
  }

  ///@}

private:
  const void* m_settingValue;
};
///@}
//------------------------------------------------------------------------------

namespace addon
{

//==============================================================================
/*
 * Internal class to control various instance types with general parts defined
 * here.
 *
 * Mainly is this currently used to identify requested instance types.
 *
 * @note This class is not need to know during add-on development thats why
 * commented with "*".
 */
class IAddonInstance
{
public:
  explicit IAddonInstance(ADDON_TYPE type, const std::string& version)
    : m_type(type), m_kodiVersion(version)
  {
  }
  virtual ~IAddonInstance() = default;

  virtual ADDON_STATUS CreateInstance(int instanceType,
                                      const std::string& instanceID,
                                      KODI_HANDLE instance,
                                      const std::string& version,
                                      KODI_HANDLE& addonInstance)
  {
    return ADDON_STATUS_NOT_IMPLEMENTED;
  }

  const ADDON_TYPE m_type;
  const std::string m_kodiVersion;
  std::string m_id;
};

/*
 * Internally used helper class to manage processing of a "C" structure in "CPP"
 * class.
 *
 * At constant, the "C" structure is copied, otherwise the given pointer is
 * superseded and is changeable.
 *
 * -----------------------------------------------------------------------------
 *
 * Example:
 *
 * ~~~~~~~~~~~~~{.cpp}
 * extern "C" typedef struct C_SAMPLE_DATA
 * {
 *   unsigned int iUniqueId;
 * } C_SAMPLE_DATA;
 *
 * class CPPSampleData : public CStructHdl<CPPSampleData, C_SAMPLE_DATA>
 * {
 * public:
 *   CPPSampleData() = default;
 *   CPPSampleData(const CPPSampleData& sample) : CStructHdl(sample) { }
 *   CPPSampleData(const C_SAMPLE_DATA* sample) : CStructHdl(sample) { }
 *   CPPSampleData(C_SAMPLE_DATA* sample) : CStructHdl(sample) { }
 *
 *   void SetUniqueId(unsigned int uniqueId) { m_cStructure->iUniqueId = uniqueId; }
 *   unsigned int GetUniqueId() const { return m_cStructure->iUniqueId; }
 * };
 *
 * ~~~~~~~~~~~~~
 *
 * It also works with the following example:
 *
 * ~~~~~~~~~~~~~{.cpp}
 * CPPSampleData test;
 * // Some work
 * C_SAMPLE_DATA* data = test;
 * // Give "data" to Kodi
 * ~~~~~~~~~~~~~
 */
template<class CPP_CLASS, typename C_STRUCT>
class CStructHdl
{
public:
  CStructHdl() : m_cStructure(new C_STRUCT()), m_owner(true) {}

  CStructHdl(const CPP_CLASS& cppClass)
    : m_cStructure(new C_STRUCT(*cppClass.m_cStructure)), m_owner(true)
  {
  }

  CStructHdl(const C_STRUCT* cStructure) : m_cStructure(new C_STRUCT(*cStructure)), m_owner(true) {}

  CStructHdl(C_STRUCT* cStructure) : m_cStructure(cStructure) { assert(cStructure); }

  const CStructHdl& operator=(const CStructHdl& right)
  {
    assert(&right.m_cStructure);
    if (m_cStructure && !m_owner)
    {
      memcpy(m_cStructure, right.m_cStructure, sizeof(C_STRUCT));
    }
    else
    {
      if (m_owner)
        delete m_cStructure;
      m_owner = true;
      m_cStructure = new C_STRUCT(*right.m_cStructure);
    }
    return *this;
  }

  const CStructHdl& operator=(const C_STRUCT& right)
  {
    assert(&right);
    if (m_cStructure && !m_owner)
    {
      memcpy(m_cStructure, &right, sizeof(C_STRUCT));
    }
    else
    {
      if (m_owner)
        delete m_cStructure;
      m_owner = true;
      m_cStructure = new C_STRUCT(*right);
    }
    return *this;
  }

  virtual ~CStructHdl()
  {
    if (m_owner)
      delete m_cStructure;
  }

  operator C_STRUCT*() { return m_cStructure; }
  operator const C_STRUCT*() const { return m_cStructure; }

  const C_STRUCT* GetCStructure() const { return m_cStructure; }

protected:
  C_STRUCT* m_cStructure = nullptr;

private:
  bool m_owner = false;
};

/// Add-on main instance class.
class ATTRIBUTE_HIDDEN CAddonBase
{
public:
  CAddonBase()
  {
    m_interface->toAddon->destroy = ADDONBASE_Destroy;
    m_interface->toAddon->get_status = ADDONBASE_GetStatus;
    m_interface->toAddon->create_instance = ADDONBASE_CreateInstance;
    m_interface->toAddon->destroy_instance = ADDONBASE_DestroyInstance;
    m_interface->toAddon->set_setting = ADDONBASE_SetSetting;
  }

  virtual ~CAddonBase() = default;

  virtual ADDON_STATUS Create() { return ADDON_STATUS_OK; }

  virtual ADDON_STATUS GetStatus() { return ADDON_STATUS_OK; }

  //============================================================================
  /// @ingroup cpp_kodi_addon_addonbase
  /// @brief To inform addon about changed settings values.
  ///
  /// This becomes called for every entry defined inside his settings.xml and
  /// as **last** call the one where last in xml (to identify end of calls).
  ///
  /// --------------------------------------------------------------------------
  ///
  /// @copydetails cpp_kodi_addon_addonbase_Defs_CSettingValue_Help
  ///
  ///
  /// --------------------------------------------------------------------------
  ///
  /// **Here is a code example how this is used:**
  ///
  /// ~~~~~~~~~~~~~{.cpp}
  /// #include <kodi/AddonBase.h>
  ///
  /// enum myEnumValue
  /// {
  ///   valueA,
  ///   valueB,
  ///   valueC
  /// };
  ///
  /// std::string m_myStringValue;
  /// int m_myIntegerValue;
  /// bool m_myBooleanValue;
  /// float m_myFloatingPointValue;
  /// myEnumValue m_myEnumValue;
  ///
  ///
  /// ADDON_STATUS CMyAddon::SetSetting(const std::string& settingName, const kodi::CSettingValue& settingValue)
  /// {
  ///   if (settingName == "my_string_value")
  ///     m_myStringValue = settingValue.GetString();
  ///   else if (settingName == "my_integer_value")
  ///     m_myIntegerValue = settingValue.GetInt();
  ///   else if (settingName == "my_boolean_value")
  ///     m_myBooleanValue = settingValue.GetBoolean();
  ///   else if (settingName == "my_float_value")
  ///     m_myFloatingPointValue = settingValue.GetFloat();
  ///   else if (settingName == "my_enum_value")
  ///     m_myEnumValue = settingValue.GetEnum<myEnumValue>();
  /// }
  /// ~~~~~~~~~~~~~
  ///
  /// @note The asked type should match the type used on settings.xml.
  ///
  virtual ADDON_STATUS SetSetting(const std::string& settingName,
                                  const kodi::CSettingValue& settingValue)
  {
    return ADDON_STATUS_UNKNOWN;
  }
  //----------------------------------------------------------------------------

  //==========================================================================
  /// @ingroup cpp_kodi_addon_addonbase
  /// @brief Instance created
  ///
  /// @param[in] instanceType The requested type of required instance, see \ref ADDON_TYPE.
  /// @param[in] instanceID An individual identification key string given by Kodi.
  /// @param[in] instance The instance handler used by Kodi must be passed to
  ///                     the classes created here. See in the example.
  /// @param[in] version The from Kodi used version of instance. This can be
  ///                    used to allow compatibility to older versions of
  ///                    them. Further is this given to the parent instance
  ///                    that it can handle differences.
  /// @param[out] addonInstance The pointer to instance class created in addon.
  ///                           Needed to be able to identify them on calls.
  /// @return                   \ref ADDON_STATUS_OK if correct, for possible errors
  ///                           see \ref ADDON_STATUS
  ///
  ///
  /// --------------------------------------------------------------------------
  ///
  /// **Here is a code example how this is used:**
  ///
  /// ~~~~~~~~~~~~~{.cpp}
  /// #include <kodi/AddonBase.h>
  ///
  /// ...
  ///
  /// /* If you use only one instance in your add-on, can be instanceType and
  ///  * instanceID ignored */
  /// ADDON_STATUS CMyAddon::CreateInstance(int instanceType,
  ///                                       const std::string& instanceID,
  ///                                       KODI_HANDLE instance,
  ///                                       const std::string& version,
  ///                                       KODI_HANDLE& addonInstance)
  /// {
  ///   if (instanceType == ADDON_INSTANCE_SCREENSAVER)
  ///   {
  ///     kodi::Log(ADDON_LOG_NOTICE, "Creating my Screensaver");
  ///     addonInstance = new CMyScreensaver(instance);
  ///     return ADDON_STATUS_OK;
  ///   }
  ///   else if (instanceType == ADDON_INSTANCE_VISUALIZATION)
  ///   {
  ///     kodi::Log(ADDON_LOG_NOTICE, "Creating my Visualization");
  ///     addonInstance = new CMyVisualization(instance);
  ///     return ADDON_STATUS_OK;
  ///   }
  ///   else if (...)
  ///   {
  ///     ...
  ///   }
  ///   return ADDON_STATUS_UNKNOWN;
  /// }
  ///
  /// ...
  ///
  /// ~~~~~~~~~~~~~
  ///
  virtual ADDON_STATUS CreateInstance(int instanceType,
                                      const std::string& instanceID,
                                      KODI_HANDLE instance,
                                      const std::string& version,
                                      KODI_HANDLE& addonInstance)
  {
    return ADDON_STATUS_NOT_IMPLEMENTED;
  }
  //--------------------------------------------------------------------------

  //==========================================================================
  /// @ingroup cpp_kodi_addon_addonbase
  /// @brief Instance destroy
  ///
  /// This function is optional and intended to notify addon that the instance
  /// is terminating.
  ///
  /// @param[in] instanceType   The requested type of required instance, see \ref ADDON_TYPE.
  /// @param[in] instanceID     An individual identification key string given by Kodi.
  /// @param[in] addonInstance  The pointer to instance class created in addon.
  ///
  /// @warning This call is only used to inform that the associated instance
  /// is terminated. The deletion is carried out in the background.
  ///
  virtual void DestroyInstance(int instanceType,
                               const std::string& instanceID,
                               KODI_HANDLE addonInstance)
  {
  }
  //--------------------------------------------------------------------------

  /* Background helper for GUI render systems, e.g. Screensaver or Visualization */
  std::shared_ptr<kodi::gui::IRenderHelper> m_renderHelper;

  /* Global variables of class */
  static AddonGlobalInterface*
      m_interface; // Interface function table to hold addresses on add-on and from kodi

  /*private:*/ /* Needed public as long the old call functions becomes used! */
  static inline void ADDONBASE_Destroy()
  {
    delete static_cast<CAddonBase*>(m_interface->addonBase);
    m_interface->addonBase = nullptr;
  }

  static inline ADDON_STATUS ADDONBASE_GetStatus()
  {
    return static_cast<CAddonBase*>(m_interface->addonBase)->GetStatus();
  }

  static inline ADDON_STATUS ADDONBASE_SetSetting(const char* settingName, const void* settingValue)
  {
    return static_cast<CAddonBase*>(m_interface->addonBase)
        ->SetSetting(settingName, CSettingValue(settingValue));
  }

private:
  static inline ADDON_STATUS ADDONBASE_CreateInstance(int instanceType,
                                                      const char* instanceID,
                                                      KODI_HANDLE instance,
                                                      const char* version,
                                                      KODI_HANDLE* addonInstance,
                                                      KODI_HANDLE parent)
  {
    CAddonBase* base = static_cast<CAddonBase*>(m_interface->addonBase);

    ADDON_STATUS status = ADDON_STATUS_NOT_IMPLEMENTED;

    /* Check about single instance usage:
     * 1. The kodi side instance pointer must be equal to first one
     * 2. The addon side instance pointer must be set
     * 3. And the requested type must be equal with used add-on class
     */
    if (m_interface->firstKodiInstance == instance && m_interface->globalSingleInstance &&
        static_cast<IAddonInstance*>(m_interface->globalSingleInstance)->m_type == instanceType)
    {
      /* The handling here is intended for the case of the add-on only one
       * instance and this is integrated in the add-on base class.
       */
      *addonInstance = m_interface->globalSingleInstance;
      status = ADDON_STATUS_OK;
    }
    else
    {
      /* Here it should use the CreateInstance instance function to allow
       * creation of several on one addon.
       */

      /* Check first a parent is defined about (e.g. Codec within inputstream) */
      if (parent != nullptr)
        status = static_cast<IAddonInstance*>(parent)->CreateInstance(
            instanceType, instanceID, instance, version, *addonInstance);

      /* if no parent call the main instance creation function to get it */
      if (status == ADDON_STATUS_NOT_IMPLEMENTED)
      {
        status = base->CreateInstance(instanceType, instanceID, instance, version, *addonInstance);
      }
    }

    if (*addonInstance == nullptr)
    {
      if (status == ADDON_STATUS_OK)
      {
        m_interface->toKodi->addon_log_msg(m_interface->toKodi->kodiBase, ADDON_LOG_FATAL,
                                           "kodi::addon::CAddonBase CreateInstance returned an "
                                           "empty instance pointer, but reported OK!");
        return ADDON_STATUS_PERMANENT_FAILURE;
      }
      else
      {
        return status;
      }
    }

    if (static_cast<IAddonInstance*>(*addonInstance)->m_type != instanceType)
    {
      m_interface->toKodi->addon_log_msg(
          m_interface->toKodi->kodiBase, ADDON_LOG_FATAL,
          "kodi::addon::CAddonBase CreateInstance difference between given and returned");
      delete static_cast<IAddonInstance*>(*addonInstance);
      *addonInstance = nullptr;
      return ADDON_STATUS_PERMANENT_FAILURE;
    }

    // Store the used ID inside instance, to have on destroy calls by addon to identify
    static_cast<IAddonInstance*>(*addonInstance)->m_id = instanceID;

    return status;
  }

  static inline void ADDONBASE_DestroyInstance(int instanceType, KODI_HANDLE instance)
  {
    CAddonBase* base = static_cast<CAddonBase*>(m_interface->addonBase);

    if (m_interface->globalSingleInstance == nullptr && instance != base)
    {
      base->DestroyInstance(instanceType, static_cast<IAddonInstance*>(instance)->m_id, instance);
      delete static_cast<IAddonInstance*>(instance);
    }
  }
};

} /* namespace addon */

//==============================================================================
/// @ingroup cpp_kodi_addon
/// @brief To get used version inside Kodi itself about asked type.
///
/// This thought to allow a addon a handling of newer addon versions within
/// older Kodi until the type min version not changed.
///
/// @param[in] type The wanted type of @ref ADDON_TYPE to ask
/// @return The version string about type in MAJOR.MINOR.PATCH style.
///
inline std::string GetKodiTypeVersion(int type)
{
  using namespace kodi::addon;

  char* str = CAddonBase::m_interface->toKodi->get_type_version(
      CAddonBase::m_interface->toKodi->kodiBase, type);
  std::string ret = str;
  CAddonBase::m_interface->toKodi->free_string(CAddonBase::m_interface->toKodi->kodiBase, str);
  return ret;
}
//------------------------------------------------------------------------------

//==============================================================================
///
inline std::string GetAddonPath(const std::string& append = "")
{
  using namespace kodi::addon;

  char* str =
      CAddonBase::m_interface->toKodi->get_addon_path(CAddonBase::m_interface->toKodi->kodiBase);
  std::string ret = str;
  CAddonBase::m_interface->toKodi->free_string(CAddonBase::m_interface->toKodi->kodiBase, str);
  if (!append.empty())
  {
    if (append.at(0) != '\\' && append.at(0) != '/')
#ifdef TARGET_WINDOWS
      ret.append("\\");
#else
      ret.append("/");
#endif
    ret.append(append);
  }
  return ret;
}
//------------------------------------------------------------------------------

//==============================================================================
///
inline std::string GetBaseUserPath(const std::string& append = "")
{
  using namespace kodi::addon;

  char* str = CAddonBase::m_interface->toKodi->get_base_user_path(
      CAddonBase::m_interface->toKodi->kodiBase);
  std::string ret = str;
  CAddonBase::m_interface->toKodi->free_string(CAddonBase::m_interface->toKodi->kodiBase, str);
  if (!append.empty())
  {
    if (append.at(0) != '\\' && append.at(0) != '/')
#ifdef TARGET_WINDOWS
      ret.append("\\");
#else
      ret.append("/");
#endif
    ret.append(append);
  }
  return ret;
}
//------------------------------------------------------------------------------

//==============================================================================
///
inline std::string GetLibPath()
{
  using namespace kodi::addon;

  return CAddonBase::m_interface->libBasePath;
}
//------------------------------------------------------------------------------

//==============================================================================
/// @ingroup cpp_kodi
/// @brief Add a message to Kodi's log.
///
/// @param[in] loglevel The log level of the message.
/// @param[in] format The format of the message to pass to Kodi.
/// @param[in] ... Additional text to insert in format text
///
///
/// @note This method uses limited buffer (16k) for the formatted output.
/// So data, which will not fit into it, will be silently discarded.
///
///
/// ----------------------------------------------------------------------------
///
/// **Example:**
/// ~~~~~~~~~~~~~{.cpp}
/// #include <kodi/General.h>
///
/// kodi::Log(ADDON_LOG_ERROR, "%s: There is an error occurred!", __func__);
///
/// ~~~~~~~~~~~~~
///
inline void Log(const AddonLog loglevel, const char* format, ...)
{
  using namespace kodi::addon;

  char buffer[16384];
  va_list args;
  va_start(args, format);
  vsnprintf(buffer, sizeof(buffer), format, args);
  va_end(args);
  CAddonBase::m_interface->toKodi->addon_log_msg(CAddonBase::m_interface->toKodi->kodiBase,
                                                 loglevel, buffer);
}
//------------------------------------------------------------------------------

//##############################################################################
/// @ingroup cpp_kodi
/// @defgroup cpp_kodi_settings 1. Setting control
/// @brief **Functions to handle settings access**\n
/// This can be used to get and set the addon related values inside his
/// settings.xml.
///
/// The settings style is given with installed part on e.g.
/// <b>`$HOME/.kodi/addons/myspecial.addon/resources/settings.xml`</b>. The
/// related edit becomes then stored inside
/// <b>`$HOME/.kodi/userdata/addon_data/myspecial.addon/settings.xml`</b>.
///
/*!@{*/

//==============================================================================
/// @brief Check and get a string setting value.
///
/// The setting name relate to names used in his <b>settings.xml</b> file.
///
/// @param[in] settingName The name of asked setting
/// @param[out] settingValue The given setting value
/// @return true if setting was successfully found and "settingValue" is set
///
/// @note If returns false, the "settingValue" is not changed.
///
///
/// ----------------------------------------------------------------------------
///
/// **Example:**
/// ~~~~~~~~~~~~~{.cpp}
/// #include <kodi/General.h>
///
/// std::string value;
/// if (!kodi::CheckSettingString("my_string_value", value))
///   value = "my_default_if_setting_not_work";
/// ~~~~~~~~~~~~~
///
inline bool CheckSettingString(const std::string& settingName, std::string& settingValue)
{
  using namespace kodi::addon;

  char* buffer = nullptr;
  bool ret = CAddonBase::m_interface->toKodi->get_setting_string(
      CAddonBase::m_interface->toKodi->kodiBase, settingName.c_str(), &buffer);
  if (buffer)
  {
    if (ret)
      settingValue = buffer;
    CAddonBase::m_interface->toKodi->free_string(CAddonBase::m_interface->toKodi->kodiBase, buffer);
  }
  return ret;
}
//------------------------------------------------------------------------------

//==============================================================================
/// @brief Get string setting value.
///
/// The setting name relate to names used in his <b>settings.xml</b> file.
///
/// @param[in] settingName The name of asked setting
/// @param[in] defaultValue [opt] Default value if not found
/// @return The value of setting, empty if not found;
///
///
/// ----------------------------------------------------------------------------
///
/// **Example:**
/// ~~~~~~~~~~~~~{.cpp}
/// #include <kodi/General.h>
///
/// std::string value = kodi::GetSettingString("my_string_value");
/// ~~~~~~~~~~~~~
///
inline std::string GetSettingString(const std::string& settingName,
                                    const std::string& defaultValue = "")
{
  std::string settingValue = defaultValue;
  CheckSettingString(settingName, settingValue);
  return settingValue;
}
//------------------------------------------------------------------------------

//==============================================================================
/// @brief Set string setting of addon.
///
/// The setting name relate to names used in his <b>settings.xml</b> file.
///
/// @param[in] settingName The name of setting
/// @param[in] settingValue The setting value to write
///
///
/// ----------------------------------------------------------------------------
///
/// **Example:**
/// ~~~~~~~~~~~~~{.cpp}
/// #include <kodi/General.h>
///
/// std::string value = "my_new_name for";
/// kodi::SetSettingString("my_string_value", value);
/// ~~~~~~~~~~~~~
///
inline void SetSettingString(const std::string& settingName, const std::string& settingValue)
{
  using namespace kodi::addon;

  CAddonBase::m_interface->toKodi->set_setting_string(CAddonBase::m_interface->toKodi->kodiBase,
                                                      settingName.c_str(), settingValue.c_str());
}
//------------------------------------------------------------------------------

//==============================================================================
/// @brief Check and get a integer setting value.
///
/// The setting name relate to names used in his <b>settings.xml</b> file.
///
/// @param[in] settingName The name of asked setting
/// @param[out] settingValue The given setting value
/// @return true if setting was successfully found and "settingValue" is set
///
/// @note If returns false, the "settingValue" is not changed.
///
///
/// ----------------------------------------------------------------------------
///
/// **Example:**
/// ~~~~~~~~~~~~~{.cpp}
/// #include <kodi/General.h>
///
/// int value = 0;
/// if (!kodi::CheckSettingInt("my_integer_value", value))
///   value = 123; // My default of them
/// ~~~~~~~~~~~~~
///
inline bool CheckSettingInt(const std::string& settingName, int& settingValue)
{
  using namespace kodi::addon;

  return CAddonBase::m_interface->toKodi->get_setting_int(CAddonBase::m_interface->toKodi->kodiBase,
                                                          settingName.c_str(), &settingValue);
}
//------------------------------------------------------------------------------

//==============================================================================
/// @brief Get integer setting value.
///
/// The setting name relate to names used in his <b>settings.xml</b> file.
///
/// @param[in] settingName The name of asked setting
/// @param[in] defaultValue [opt] Default value if not found
/// @return The value of setting, <b>`0`</b> or defaultValue if not found
///
///
/// ----------------------------------------------------------------------------
///
/// **Example:**
/// ~~~~~~~~~~~~~{.cpp}
/// #include <kodi/General.h>
///
/// int value = kodi::GetSettingInt("my_integer_value");
/// ~~~~~~~~~~~~~
///
inline int GetSettingInt(const std::string& settingName, int defaultValue = 0)
{
  int settingValue = defaultValue;
  CheckSettingInt(settingName, settingValue);
  return settingValue;
}
//------------------------------------------------------------------------------

//==============================================================================
/// @brief Set integer setting of addon.
///
/// The setting name relate to names used in his <b>settings.xml</b> file.
///
/// @param[in] settingName The name of setting
/// @param[in] settingValue The setting value to write
///
///
/// ----------------------------------------------------------------------------
///
/// **Example:**
/// ~~~~~~~~~~~~~{.cpp}
/// #include <kodi/General.h>
///
/// int value = 123;
/// kodi::SetSettingInt("my_integer_value", value);
/// ~~~~~~~~~~~~~
///
inline void SetSettingInt(const std::string& settingName, int settingValue)
{
  using namespace kodi::addon;

  CAddonBase::m_interface->toKodi->set_setting_int(CAddonBase::m_interface->toKodi->kodiBase,
                                                   settingName.c_str(), settingValue);
}
//------------------------------------------------------------------------------

//==============================================================================
/// @brief Check and get a boolean setting value.
///
/// The setting name relate to names used in his <b>settings.xml</b> file.
///
/// @param[in] settingName The name of asked setting
/// @param[out] settingValue The given setting value
/// @return true if setting was successfully found and "settingValue" is set
///
/// @note If returns false, the "settingValue" is not changed.
///
///
/// ----------------------------------------------------------------------------
///
/// **Example:**
/// ~~~~~~~~~~~~~{.cpp}
/// #include <kodi/General.h>
///
/// bool value = false;
/// if (!kodi::CheckSettingBoolean("my_boolean_value", value))
///   value = true; // My default of them
/// ~~~~~~~~~~~~~
///
inline bool CheckSettingBoolean(const std::string& settingName, bool& settingValue)
{
  using namespace kodi::addon;

  return CAddonBase::m_interface->toKodi->get_setting_bool(
      CAddonBase::m_interface->toKodi->kodiBase, settingName.c_str(), &settingValue);
}
//------------------------------------------------------------------------------

//==============================================================================
/// @brief Get boolean setting value.
///
/// The setting name relate to names used in his <b>settings.xml</b> file.
///
/// @param[in] settingName The name of asked setting
/// @param[in] defaultValue [opt] Default value if not found
/// @return The value of setting, <b>`false`</b> or defaultValue if not found
///
///
/// ----------------------------------------------------------------------------
///
/// **Example:**
/// ~~~~~~~~~~~~~{.cpp}
/// #include <kodi/General.h>
///
/// bool value = kodi::GetSettingBoolean("my_boolean_value");
/// ~~~~~~~~~~~~~
///
inline bool GetSettingBoolean(const std::string& settingName, bool defaultValue = false)
{
  bool settingValue = defaultValue;
  CheckSettingBoolean(settingName, settingValue);
  return settingValue;
}
//------------------------------------------------------------------------------

//==============================================================================
/// @brief Set boolean setting of addon.
///
/// The setting name relate to names used in his <b>settings.xml</b> file.
///
/// @param[in] settingName The name of setting
/// @param[in] settingValue The setting value to write
///
///
/// ----------------------------------------------------------------------------
///
/// **Example:**
/// ~~~~~~~~~~~~~{.cpp}
/// #include <kodi/General.h>
///
/// bool value = true;
/// kodi::SetSettingBoolean("my_boolean_value", value);
/// ~~~~~~~~~~~~~
///
inline void SetSettingBoolean(const std::string& settingName, bool settingValue)
{
  using namespace kodi::addon;

  CAddonBase::m_interface->toKodi->set_setting_bool(CAddonBase::m_interface->toKodi->kodiBase,
                                                    settingName.c_str(), settingValue);
}
//------------------------------------------------------------------------------

//==============================================================================
/// @brief Check and get a floating point setting value.
///
/// The setting name relate to names used in his <b>settings.xml</b> file.
///
/// @param[in] settingName The name of asked setting
/// @param[out] settingValue The given setting value
/// @return true if setting was successfully found and "settingValue" is set
///
/// @note If returns false, the "settingValue" is not changed.
///
///
/// ----------------------------------------------------------------------------
///
/// **Example:**
/// ~~~~~~~~~~~~~{.cpp}
/// #include <kodi/General.h>
///
/// float value = 0.0f;
/// if (!kodi::CheckSettingBoolean("my_float_value", value))
///   value = 1.0f; // My default of them
/// ~~~~~~~~~~~~~
///
inline bool CheckSettingFloat(const std::string& settingName, float& settingValue)
{
  using namespace kodi::addon;

  return CAddonBase::m_interface->toKodi->get_setting_float(
      CAddonBase::m_interface->toKodi->kodiBase, settingName.c_str(), &settingValue);
}
//------------------------------------------------------------------------------

//==============================================================================
/// @brief Get floating point setting value.
///
/// The setting name relate to names used in his <b>settings.xml</b> file.
///
/// @param[in] settingName The name of asked setting
/// @param[in] defaultValue [opt] Default value if not found
/// @return The value of setting, <b>`0.0`</b> or defaultValue if not found
///
///
/// ----------------------------------------------------------------------------
///
/// **Example:**
/// ~~~~~~~~~~~~~{.cpp}
/// #include <kodi/General.h>
///
/// float value = kodi::GetSettingFloat("my_float_value");
/// ~~~~~~~~~~~~~
///
inline float GetSettingFloat(const std::string& settingName, float defaultValue = 0.0f)
{
  float settingValue = defaultValue;
  CheckSettingFloat(settingName, settingValue);
  return settingValue;
}
//------------------------------------------------------------------------------

//==============================================================================
/// @brief Set floating point setting of addon.
///
/// The setting name relate to names used in his <b>settings.xml</b> file.
///
/// @param[in] settingName The name of setting
/// @param[in] settingValue The setting value to write
///
///
/// ----------------------------------------------------------------------------
///
/// **Example:**
/// ~~~~~~~~~~~~~{.cpp}
/// #include <kodi/General.h>
///
/// float value = 1.0f;
/// kodi::SetSettingFloat("my_float_value", value);
/// ~~~~~~~~~~~~~
///
inline void SetSettingFloat(const std::string& settingName, float settingValue)
{
  using namespace kodi::addon;

  CAddonBase::m_interface->toKodi->set_setting_float(CAddonBase::m_interface->toKodi->kodiBase,
                                                     settingName.c_str(), settingValue);
}
//------------------------------------------------------------------------------

//==============================================================================
/// @brief Check and get a enum setting value.
///
/// The setting name relate to names used in his <b>settings.xml</b> file.
///
/// @param[in] settingName The name of asked setting
/// @param[out] settingValue The given setting value
/// @return true if setting was successfully found and "settingValue" is set
///
/// @remark The enums are used as integer inside settings.xml.
/// @note If returns false, the "settingValue" is not changed.
///
///
/// ----------------------------------------------------------------------------
///
/// **Example:**
/// ~~~~~~~~~~~~~{.cpp}
/// #include <kodi/General.h>
///
/// enum myEnumValue
/// {
///   valueA,
///   valueB,
///   valueC
/// };
///
/// myEnumValue value;
/// if (!kodi::CheckSettingEnum<myEnumValue>("my_enum_value", value))
///   value = valueA; // My default of them
/// ~~~~~~~~~~~~~
///
template<typename enumType>
inline bool CheckSettingEnum(const std::string& settingName, enumType& settingValue)
{
  using namespace kodi::addon;

  int settingValueInt = static_cast<int>(settingValue);
  bool ret = CAddonBase::m_interface->toKodi->get_setting_int(
      CAddonBase::m_interface->toKodi->kodiBase, settingName.c_str(), &settingValueInt);
  if (ret)
    settingValue = static_cast<enumType>(settingValueInt);
  return ret;
}
//------------------------------------------------------------------------------

//==============================================================================
/// @brief Get enum setting value.
///
/// The setting name relate to names used in his <b>settings.xml</b> file.
///
/// @param[in] settingName The name of asked setting
/// @param[in] defaultValue [opt] Default value if not found
/// @return The value of setting, forced to <b>`0`</b> or defaultValue if not found
///
/// @remark The enums are used as integer inside settings.xml.
///
///
/// ----------------------------------------------------------------------------
///
/// **Example:**
/// ~~~~~~~~~~~~~{.cpp}
/// #include <kodi/General.h>
///
/// enum myEnumValue
/// {
///   valueA,
///   valueB,
///   valueC
/// };
///
/// myEnumValue value = kodi::GetSettingEnum<myEnumValue>("my_enum_value");
/// ~~~~~~~~~~~~~
///
template<typename enumType>
inline enumType GetSettingEnum(const std::string& settingName,
                               enumType defaultValue = static_cast<enumType>(0))
{
  enumType settingValue = defaultValue;
  CheckSettingEnum(settingName, settingValue);
  return settingValue;
}
//------------------------------------------------------------------------------

//==============================================================================
/// @brief Set enum setting of addon.
///
/// The setting name relate to names used in his <b>settings.xml</b> file.
///
/// @param[in] settingName The name of setting
/// @param[in] settingValue The setting value to write
///
/// @remark The enums are used as integer inside settings.xml.
///
///
/// ----------------------------------------------------------------------------
///
/// **Example:**
/// ~~~~~~~~~~~~~{.cpp}
/// #include <kodi/General.h>
///
/// enum myEnumValue
/// {
///   valueA,
///   valueB,
///   valueC
/// };
///
/// myEnumValue value = valueA;
/// kodi::SetSettingEnum<myEnumValue>("my_enum_value", value);
/// ~~~~~~~~~~~~~
///
template<typename enumType>
inline void SetSettingEnum(const std::string& settingName, enumType settingValue)
{
  using namespace kodi::addon;

  CAddonBase::m_interface->toKodi->set_setting_int(CAddonBase::m_interface->toKodi->kodiBase,
                                                   settingName.c_str(),
                                                   static_cast<int>(settingValue));
}
//------------------------------------------------------------------------------

/*!@}*/

//============================================================================
///
inline std::string TranslateAddonStatus(ADDON_STATUS status)
{
  switch (status)
  {
    case ADDON_STATUS_OK:
      return "OK";
    case ADDON_STATUS_LOST_CONNECTION:
      return "Lost Connection";
    case ADDON_STATUS_NEED_RESTART:
      return "Need Restart";
    case ADDON_STATUS_NEED_SETTINGS:
      return "Need Settings";
    case ADDON_STATUS_UNKNOWN:
      return "Unknown error";
    case ADDON_STATUS_PERMANENT_FAILURE:
      return "Permanent failure";
    case ADDON_STATUS_NOT_IMPLEMENTED:
      return "Not implemented";
    default:
      break;
  }
  return "Unknown";
}
//----------------------------------------------------------------------------

//==============================================================================
/// @ingroup cpp_kodi
/// @brief Returns a function table to a named interface
///
/// @return pointer to struct containing interface functions
///
///
/// ------------------------------------------------------------------------
///
/// **Example:**
/// ~~~~~~~~~~~~~{.cpp}
/// #include <kodi/General.h>
/// #include <kodi/platform/foo.h>
/// ...
/// FuncTable_foo *table = kodi::GetPlatformInfo(foo_name, foo_version);
/// ...
/// ~~~~~~~~~~~~~
///
inline void* GetInterface(const std::string& name, const std::string& version)
{
  using namespace kodi::addon;

  AddonToKodiFuncTable_Addon* toKodi = CAddonBase::m_interface->toKodi;

  return toKodi->get_interface(toKodi->kodiBase, name.c_str(), version.c_str());
}
//----------------------------------------------------------------------------

} /* namespace kodi */

/*! addon creation macro
 * @todo cleanup this stupid big macro
 * This macro includes now all for add-on's needed functions. This becomes a bigger
 * rework after everything is done on Kodi itself, currently is this way needed
 * to have compatibility with not reworked interfaces.
 *
 * Becomes really cleaned up soon :D
 */
#define ADDONCREATOR(AddonClass) \
  extern "C" __declspec(dllexport) ADDON_STATUS ADDON_Create( \
      KODI_HANDLE addonInterface, const char* globalApiVersion, void* unused) \
  { \
    kodi::addon::CAddonBase::m_interface = static_cast<AddonGlobalInterface*>(addonInterface); \
    kodi::addon::CAddonBase::m_interface->addonBase = new AddonClass; \
    return static_cast<kodi::addon::CAddonBase*>(kodi::addon::CAddonBase::m_interface->addonBase) \
        ->Create(); \
  } \
  extern "C" __declspec(dllexport) void ADDON_Destroy() \
  { \
    kodi::addon::CAddonBase::ADDONBASE_Destroy(); \
  } \
  extern "C" __declspec(dllexport) ADDON_STATUS ADDON_GetStatus() \
  { \
    return kodi::addon::CAddonBase::ADDONBASE_GetStatus(); \
  } \
  extern "C" __declspec(dllexport) ADDON_STATUS ADDON_SetSetting(const char* settingName, \
                                                                 const void* settingValue) \
  { \
    return kodi::addon::CAddonBase::ADDONBASE_SetSetting(settingName, settingValue); \
  } \
  extern "C" __declspec(dllexport) const char* ADDON_GetTypeVersion(int type) \
  { \
    return kodi::addon::GetTypeVersion(type); \
  } \
  extern "C" __declspec(dllexport) const char* ADDON_GetTypeMinVersion(int type) \
  { \
    return kodi::addon::GetTypeMinVersion(type); \
  } \
  AddonGlobalInterface* kodi::addon::CAddonBase::m_interface = nullptr;
