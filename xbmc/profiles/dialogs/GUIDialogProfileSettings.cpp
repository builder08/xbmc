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

#ifndef DIALOGS_GUIDIALOGPROFILESETTINGS_H_INCLUDED
#define DIALOGS_GUIDIALOGPROFILESETTINGS_H_INCLUDED
#include "GUIDialogProfileSettings.h"
#endif

#ifndef DIALOGS_DIALOGS_GUIDIALOGFILEBROWSER_H_INCLUDED
#define DIALOGS_DIALOGS_GUIDIALOGFILEBROWSER_H_INCLUDED
#include "dialogs/GUIDialogFileBrowser.h"
#endif

#ifndef DIALOGS_GUILIB_GUIKEYBOARDFACTORY_H_INCLUDED
#define DIALOGS_GUILIB_GUIKEYBOARDFACTORY_H_INCLUDED
#include "guilib/GUIKeyboardFactory.h"
#endif

#ifndef DIALOGS_PROFILES_PROFILESMANAGER_H_INCLUDED
#define DIALOGS_PROFILES_PROFILESMANAGER_H_INCLUDED
#include "profiles/ProfilesManager.h"
#endif

#ifndef DIALOGS_PROFILES_DIALOGS_GUIDIALOGLOCKSETTINGS_H_INCLUDED
#define DIALOGS_PROFILES_DIALOGS_GUIDIALOGLOCKSETTINGS_H_INCLUDED
#include "profiles/dialogs/GUIDialogLockSettings.h"
#endif

#ifndef DIALOGS_GUILIB_GUIIMAGE_H_INCLUDED
#define DIALOGS_GUILIB_GUIIMAGE_H_INCLUDED
#include "guilib/GUIImage.h"
#endif

#ifndef DIALOGS_GUILIB_GUIWINDOWMANAGER_H_INCLUDED
#define DIALOGS_GUILIB_GUIWINDOWMANAGER_H_INCLUDED
#include "guilib/GUIWindowManager.h"
#endif

#ifndef DIALOGS_STORAGE_MEDIAMANAGER_H_INCLUDED
#define DIALOGS_STORAGE_MEDIAMANAGER_H_INCLUDED
#include "storage/MediaManager.h"
#endif

#ifndef DIALOGS_UTIL_H_INCLUDED
#define DIALOGS_UTIL_H_INCLUDED
#include "Util.h"
#endif

#ifndef DIALOGS_UTILS_URIUTILS_H_INCLUDED
#define DIALOGS_UTILS_URIUTILS_H_INCLUDED
#include "utils/URIUtils.h"
#endif

#ifndef DIALOGS_GUIPASSWORD_H_INCLUDED
#define DIALOGS_GUIPASSWORD_H_INCLUDED
#include "GUIPassword.h"
#endif

#ifndef DIALOGS_DIALOGS_GUIDIALOGYESNO_H_INCLUDED
#define DIALOGS_DIALOGS_GUIDIALOGYESNO_H_INCLUDED
#include "dialogs/GUIDialogYesNo.h"
#endif

#ifndef DIALOGS_FILESYSTEM_DIRECTORY_H_INCLUDED
#define DIALOGS_FILESYSTEM_DIRECTORY_H_INCLUDED
#include "filesystem/Directory.h"
#endif

#ifndef DIALOGS_FILESYSTEM_FILE_H_INCLUDED
#define DIALOGS_FILESYSTEM_FILE_H_INCLUDED
#include "filesystem/File.h"
#endif

#ifndef DIALOGS_FILEITEM_H_INCLUDED
#define DIALOGS_FILEITEM_H_INCLUDED
#include "FileItem.h"
#endif

#ifndef DIALOGS_SETTINGS_SETTINGS_H_INCLUDED
#define DIALOGS_SETTINGS_SETTINGS_H_INCLUDED
#include "settings/Settings.h"
#endif

#ifndef DIALOGS_GUILIB_LOCALIZESTRINGS_H_INCLUDED
#define DIALOGS_GUILIB_LOCALIZESTRINGS_H_INCLUDED
#include "guilib/LocalizeStrings.h"
#endif

#ifndef DIALOGS_TEXTURECACHE_H_INCLUDED
#define DIALOGS_TEXTURECACHE_H_INCLUDED
#include "TextureCache.h"
#endif


using namespace XFILE;

#define CONTROL_PROFILE_IMAGE       2
#define CONTROL_START              30

CGUIDialogProfileSettings::CGUIDialogProfileSettings(void)
    : CGUIDialogSettings(WINDOW_DIALOG_PROFILE_SETTINGS, "ProfileSettings.xml")
{
  m_bNeedSave = false;
}

CGUIDialogProfileSettings::~CGUIDialogProfileSettings(void)
{
}

bool CGUIDialogProfileSettings::OnMessage(CGUIMessage &message)
{
  if (message.GetMessage() == GUI_MSG_CLICKED)
  {
    int iControl = message.GetSenderId();
    if (iControl == 500)
      Close();
    if (iControl == 501)
    {
      m_bNeedSave = false;
      Close();
    }
  }
  return CGUIDialogSettings::OnMessage(message);
}

void CGUIDialogProfileSettings::OnWindowLoaded()
{
  CGUIDialogSettings::OnWindowLoaded();
  CGUIImage *pImage = (CGUIImage*)GetControl(2);
  m_strDefaultImage = pImage ? pImage->GetFileName() : "";
}

void CGUIDialogProfileSettings::SetupPage()
{
  CGUIDialogSettings::SetupPage();
  SET_CONTROL_LABEL(1000,m_strName);
  SET_CONTROL_LABEL(1001,m_strDirectory);
  CGUIImage *pImage = (CGUIImage*)GetControl(2);
  if (pImage)
    pImage->SetFileName(!m_strThumb.empty() ? m_strThumb : m_strDefaultImage);
}

void CGUIDialogProfileSettings::CreateSettings()
{
  // clear out any old settings
  m_settings.clear();

  AddButton(1,20093);
  AddButton(2,20065);
  if (!m_bIsDefault && m_bShowDetails)
    AddButton(3,20070);

  if (m_bShowDetails)
    AddButton(4,20066);
  if (!m_bShowDetails && m_locks.mode == LOCK_MODE_EVERYONE && CProfilesManager::Get().GetMasterProfile().getLockMode() != LOCK_MODE_EVERYONE)
    AddButton(4,20066);

  if (!m_bIsDefault && m_bShowDetails)
  {
    SettingInfo setting;
    setting.id = 5;
    setting.name = g_localizeStrings.Get(20060);
    setting.data = &m_iDbMode;
    setting.type = SettingInfo::SPIN;
    setting.entry.push_back(make_pair(setting.entry.size(), g_localizeStrings.Get(20062)));
    setting.entry.push_back(make_pair(setting.entry.size(), g_localizeStrings.Get(20063)));
    setting.entry.push_back(make_pair(setting.entry.size(), g_localizeStrings.Get(20061)));
    if (CProfilesManager::Get().GetMasterProfile().getLockMode() != LOCK_MODE_EVERYONE)
      setting.entry.push_back(make_pair(setting.entry.size(), g_localizeStrings.Get(20107)));

    m_settings.push_back(setting);

    SettingInfo setting2;
    setting2.id = 6;
    setting2.name = g_localizeStrings.Get(20094);
    setting2.data = &m_iSourcesMode;
    setting2.type = SettingInfo::SPIN;
    setting2.entry.push_back(make_pair(setting2.entry.size(), g_localizeStrings.Get(20062)));
    setting2.entry.push_back(make_pair(setting2.entry.size(), g_localizeStrings.Get(20063)));
    setting2.entry.push_back(make_pair(setting2.entry.size(), g_localizeStrings.Get(20061)));
    if (CProfilesManager::Get().GetMasterProfile().getLockMode() != LOCK_MODE_EVERYONE)
      setting2.entry.push_back(make_pair(setting2.entry.size(), g_localizeStrings.Get(20107)));

    m_settings.push_back(setting2);
  }
}

void CGUIDialogProfileSettings::OnSettingChanged(unsigned int num)
{
  // setting has changed - update anything that needs it
  if (num >= m_settings.size()) return;
  OnSettingChanged(m_settings.at(num));
}

void CGUIDialogProfileSettings::OnSettingChanged(SettingInfo &setting)
{
  // check and update anything that needs it
  if (setting.id == 1)
  {
    if (CGUIKeyboardFactory::ShowAndGetInput(m_strName,g_localizeStrings.Get(20093),false))
    {
      m_bNeedSave = true;
      SET_CONTROL_LABEL(1000,m_strName);
    }
  }
  if (setting.id == 2)
  {
    CStdString strThumb;
    VECSOURCES shares;
    g_mediaManager.GetLocalDrives(shares);
    CFileItemList items;
    if (!m_strThumb.empty())
    {
      CFileItemPtr item(new CFileItem("thumb://Current", false));
      item->SetArt("thumb", m_strThumb);
      item->SetLabel(g_localizeStrings.Get(20016));
      items.Add(item);
    }
    CFileItemPtr item(new CFileItem("thumb://None", false));
    item->SetArt("thumb", m_strDefaultImage);
    item->SetLabel(g_localizeStrings.Get(20018));
    items.Add(item);
    if (CGUIDialogFileBrowser::ShowAndGetImage(items,shares,g_localizeStrings.Get(1030),strThumb) &&
        !strThumb.Equals("thumb://Current"))
    {
      m_bNeedSave = true;
      m_strThumb = strThumb.Equals("thumb://None") ? "" : strThumb;

      CGUIImage *pImage = (CGUIImage*)GetControl(2);
      if (pImage)
      {
        pImage->SetFileName("");
        pImage->SetInvalid();
        pImage->SetFileName(!m_strThumb.empty() ? m_strThumb : m_strDefaultImage);
      }
    }
  }
  if (setting.id == 3)
  {
    if (OnProfilePath(m_strDirectory, m_bIsDefault))
    {
      m_bNeedSave = true;
      SET_CONTROL_LABEL(1001,m_strDirectory);
    }
  }

  if (setting.id == 4)
  {
    if (m_bShowDetails)
    {
      if (CProfilesManager::Get().GetMasterProfile().getLockMode() == LOCK_MODE_EVERYONE && !m_bIsDefault)
      {
        if (CGUIDialogYesNo::ShowAndGetInput(20066,20118,20119,20022))
          g_passwordManager.SetMasterLockMode(false);
        if (CProfilesManager::Get().GetMasterProfile().getLockMode() == LOCK_MODE_EVERYONE)
          return;
      }
      if (CGUIDialogLockSettings::ShowAndGetLock(m_locks, m_bIsDefault ? 12360 : 20068, CProfilesManager::Get().GetMasterProfile().getLockMode() == LOCK_MODE_EVERYONE || m_bIsDefault))
        m_bNeedSave = true;
    }
    else
    {
      if (CGUIDialogLockSettings::ShowAndGetLock(m_locks, m_bIsDefault ? 12360 : 20068, false, false))
        m_bNeedSave = true;
    }
  }
  if (setting.id > 4)
    m_bNeedSave = true;
}

void CGUIDialogProfileSettings::OnCancel()
{
  m_bNeedSave = false;
}

bool CGUIDialogProfileSettings::OnProfilePath(CStdString &dir, bool isDefault)
{
  VECSOURCES shares;
  CMediaSource share;
  share.strName = "Profiles";
  share.strPath = "special://masterprofile/profiles/";
  shares.push_back(share);
  CStdString strDirectory;
  if (dir.empty())
    strDirectory = share.strPath;
  else
    strDirectory = URIUtils::AddFileToFolder("special://masterprofile/", dir);
  if (CGUIDialogFileBrowser::ShowAndGetDirectory(shares,g_localizeStrings.Get(657),strDirectory,true))
  {
    dir = strDirectory;
    if (!isDefault)
      dir.erase(0,24);
    return true;
  }
  return false;
}

bool CGUIDialogProfileSettings::ShowForProfile(unsigned int iProfile, bool firstLogin)
{
  CGUIDialogProfileSettings *dialog = (CGUIDialogProfileSettings *)g_windowManager.GetWindow(WINDOW_DIALOG_PROFILE_SETTINGS);
  if (!dialog) return false;
  if (iProfile == 0)
    dialog->m_bIsDefault = true;
  else
    dialog->m_bIsDefault = false;
  if (firstLogin && iProfile > CProfilesManager::Get().GetNumberOfProfiles())
    return false;

  dialog->m_bNeedSave = false;
  dialog->m_bShowDetails = !firstLogin;
  dialog->SetProperty("heading", g_localizeStrings.Get(firstLogin ? 20255 : 20067));

  const CProfile *profile = CProfilesManager::Get().GetProfile(iProfile);

  if (!profile)
  { // defaults
    dialog->m_strName.clear();
    dialog->m_iDbMode = 2;
    dialog->m_iSourcesMode = 2;
    dialog->m_locks = CProfile::CLock();

    bool bLock = CProfilesManager::Get().GetMasterProfile().getLockMode() != LOCK_MODE_EVERYONE && !g_passwordManager.bMasterUser;
    dialog->m_locks.addonManager = bLock;
    dialog->m_locks.settings = (bLock) ? LOCK_LEVEL::ALL : LOCK_LEVEL::NONE;
    dialog->m_locks.files = bLock;

    dialog->m_strDirectory.clear();
    dialog->m_strThumb.clear();
    // prompt for a name
    if (!CGUIKeyboardFactory::ShowAndGetInput(dialog->m_strName,g_localizeStrings.Get(20093),false) || dialog->m_strName.empty())
      return false;
    // create a default path
    CStdString defaultDir = URIUtils::AddFileToFolder("profiles",CUtil::MakeLegalFileName(dialog->m_strName));
    URIUtils::AddSlashAtEnd(defaultDir);
    CDirectory::Create(URIUtils::AddFileToFolder("special://masterprofile/", defaultDir));
    // prompt for the user to change it if they want
    CStdString userDir = defaultDir;
    if (dialog->OnProfilePath(userDir, false)) // can't be the master user
    {
      if (!StringUtils::StartsWith(userDir, defaultDir)) // user chose a different folder
        CDirectory::Remove(URIUtils::AddFileToFolder("special://masterprofile/", defaultDir));
    }
    dialog->m_strDirectory = userDir;
    dialog->m_bNeedSave = true;
  }
  else
  {
    dialog->m_strName = profile->getName();
    dialog->m_strThumb = profile->getThumb();
    dialog->m_strDirectory = profile->getDirectory();
    dialog->m_iDbMode = profile->canWriteDatabases()?0:1;
    dialog->m_iSourcesMode = profile->canWriteSources()?0:1;
    if (profile->hasDatabases())
      dialog->m_iDbMode += 2;
    if (profile->hasSources())
      dialog->m_iSourcesMode += 2;

    dialog->m_locks = profile->GetLocks();
  }
  dialog->DoModal();
  if (dialog->m_bNeedSave)
  {
    if (iProfile >= CProfilesManager::Get().GetNumberOfProfiles())
    {
      if (dialog->m_strName.empty() || dialog->m_strDirectory.empty())
        return false;
      /*CStdString strLabel;
      strLabel.Format(g_localizeStrings.Get(20047),dialog->m_strName);
      if (!CGUIDialogYesNo::ShowAndGetInput(g_localizeStrings.Get(20058),strLabel,dialog->m_strDirectory,""))
      {
        CDirectory::Remove(URIUtils::AddFileToFolder(CProfilesManager::Get().GetUserDataFolder(), dialog->m_strDirectory));
        return false;
      }*/

      // check for old profile settings
      CProfile profile(dialog->m_strDirectory, dialog->m_strName, CProfilesManager::Get().GetNextProfileId());
      CProfilesManager::Get().AddProfile(profile);
      bool bExists = CFile::Exists(URIUtils::AddFileToFolder("special://masterprofile/",
                                                          dialog->m_strDirectory+"/guisettings.xml"));

      if (bExists)
        if (!CGUIDialogYesNo::ShowAndGetInput(20058,20104,20105,20022))
          bExists = false;

      if (!bExists)
      {
        // copy masterprofile guisettings to new profile guisettings
        // If the user selects 'start fresh', do nothing as a fresh
        // guisettings.xml will be created on first profile use.
        if (CGUIDialogYesNo::ShowAndGetInput(20058,20048,20102,20022,20044,20064))
        {
          CFile::Cache(URIUtils::AddFileToFolder("special://masterprofile/","guisettings.xml"),
                       URIUtils::AddFileToFolder("special://masterprofile/",
                                              dialog->m_strDirectory+"/guisettings.xml"));
        }
      }

      bExists = CFile::Exists(URIUtils::AddFileToFolder("special://masterprofile/",
                                                     dialog->m_strDirectory+"/sources.xml"));
      if (bExists)
        if (!CGUIDialogYesNo::ShowAndGetInput(20058,20106,20105,20022))
          bExists = false;

      if (!bExists)
      {
        if ((dialog->m_iSourcesMode & 2) == 2)
          // prompt user to copy masterprofile's sources.xml file
          // If 'start fresh' (no) is selected, do nothing.
          if (CGUIDialogYesNo::ShowAndGetInput(20058,20071,20102,20022,20044,20064))
          {
            CFile::Cache(URIUtils::AddFileToFolder("special://masterprofile/","sources.xml"),
                         URIUtils::AddFileToFolder("special://masterprofile/",
                         dialog->m_strDirectory+"/sources.xml"));
          }
      }
    }

    /*if (!dialog->m_bIsNewUser)
      if (!CGUIDialogYesNo::ShowAndGetInput(20067,20103,20022,20022))
        return false;*/

    CProfile *profile = CProfilesManager::Get().GetProfile(iProfile);
    assert(profile);
    profile->setName(dialog->m_strName);
    profile->setDirectory(dialog->m_strDirectory);
    profile->setThumb(dialog->m_strThumb);
    profile->setWriteDatabases(!((dialog->m_iDbMode & 1) == 1));
    profile->setWriteSources(!((dialog->m_iSourcesMode & 1) == 1));
    profile->setDatabases((dialog->m_iDbMode & 2) == 2);
    profile->setSources((dialog->m_iSourcesMode & 2) == 2);
    profile->SetLocks(dialog->m_locks);

    CProfilesManager::Get().Save();
    return true;
  }

  return !dialog->m_bNeedSave;
}

