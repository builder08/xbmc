/*
 *      Copyright (C) 2013 Team XBMC
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

#include <limits.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "system.h"
#include "network/Network.h"
#include "Application.h"
#include "DNSNameCache.h"
#include "dialogs/GUIDialogProgress.h"
#include "dialogs/GUIDialogKaiToast.h"
#include "filesystem/SpecialProtocol.h"
#include "guilib/GUIWindowManager.h"
#include "guilib/LocalizeStrings.h"
#include "settings/AdvancedSettings.h"
#include "settings/Settings.h"
#include "settings/MediaSourceSettings.h"
#include "utils/JobManager.h"
#include "utils/log.h"
#include "utils/XMLUtils.h"
#include "utils/URIUtils.h"
#include "utils/StringUtils.h"

#include "WakeOnAccess.h"

using namespace std;

#define DEFAULT_NETWORK_INIT_SEC      (20)   // wait 20 sec for network after startup or resume
#define DEFAULT_NETWORK_SETTLE_MS     (500)  // require 500ms of consistent network availability before trusting it

#define DEFAULT_TIMEOUT_SEC (5*60)           // at least 5 minutes between each magic packets
#define DEFAULT_WAIT_FOR_ONLINE_SEC_1 (40)   // wait at 40 seconds after sending magic packet
#define DEFAULT_WAIT_FOR_ONLINE_SEC_2 (40)   // same for extended wait
#define DEFAULT_WAIT_FOR_SERVICES_SEC (5)    // wait 5 seconds after host go online to launch file sharing deamons

static int GetTotalSeconds(const CDateTimeSpan& ts)
{
  int hours = ts.GetHours() + ts.GetDays() * 24;
  int minutes = ts.GetMinutes() + hours * 60;
  return ts.GetSeconds() + minutes * 60;
}

static unsigned long HostToIP(const CStdString& host)
{
  CStdString ip;
  CDNSNameCache::Lookup(host, ip);
  return inet_addr(ip.c_str());
}

#ifdef HAS_UPNP

#include "network/upnp/UPnP.h"
#include <Platinum/Source/Devices/MediaServer/PltSyncMediaBrowser.h>

struct UPnpServer
{
  UPnpServer() {}
  UPnpServer(const CStdString& _uuid) : uuid(_uuid) { LookupHost(); }
  UPnpServer(const CStdString& _uuid, const CStdString& _host) : uuid(_uuid), host(_host) {}

  CStdString uuid;
  CStdString host;

private:
  void LookupHost ()
  {
    PLT_SyncMediaBrowser* browser = UPNP::CUPnP::GetInstance()->m_MediaBrowser;

    PLT_DeviceDataReference device;

    if (browser && NPT_SUCCEEDED(browser->FindServer(this->uuid.c_str(), device)) && !device.IsNull())
      this->host = device->GetURLBase().GetHost();
  }
};

typedef std::vector<UPnpServer> upnp_server_list_t;

static CStdString LookupHost (const upnp_server_list_t& list, const CStdString& uuid)
{
  for (upnp_server_list_t::const_iterator i = list.begin(); i != list.end(); ++i)
    if (i->uuid == uuid)
      return i->host;

  return "";
}

class UPnPServerList : public upnp_server_list_t
{
public:
  bool m_is_dirty;

  CCriticalSection m_crit_section;

  void Clear ()
  {
    CSingleLock lock (m_crit_section);

    clear();
  }
  CStdString LookupHost (const CStdString& uuid) const
  {
    CSingleLock lock (m_crit_section);

    return ::LookupHost (*this, uuid);
  }

  void StoreServer (const UPnpServer& server)
  {
    CSingleLock lock (m_crit_section);

    for (iterator i = begin(); i != end(); ++i)
    {
      if (i->uuid == server.uuid)
      {
        if (!server.host.empty()) 
        {
          if (i->host != server.host)
            m_is_dirty = true;

          *i = server;
        }
        return;
      }
    }

    push_back(server);
    m_is_dirty = true;
  }
  void MoveTo (upnp_server_list_t& list)
  {
    CSingleLock lock (m_crit_section);

    list = * this;

    if (size() > 0)
      m_is_dirty = true;

    clear();
  }
  void LearnFrom (const upnp_server_list_t& list)
  {
    CSingleLock lock (m_crit_section);

    for (iterator i = begin(); i != end(); ++i)
      if (i->host.empty())
        i->host = ::LookupHost(list, i->uuid);
  }
};

static UPnPServerList upnp_servers;

#endif // HAS_UPNP

static CStdString GetAndStoreUrlHost (const CURL& url)
{
#ifdef HAS_UPNP
  if (url.GetProtocol() == "upnp") 
  {
    UPnpServer server (url.GetHostName()); // url.hostname is server.uuid

    upnp_servers.StoreServer (server);

    return server.host;
  }
#endif

  return url.GetHostName();
}

static CStdString GetStoredUrlHost (const CURL& url)
{
#ifdef HAS_UPNP
  if (url.GetProtocol() == "upnp") 
    return upnp_servers.LookupHost(url.GetHostName());
#endif

  return url.GetHostName();
}

//

CWakeOnAccess::WakeUpEntry::WakeUpEntry (bool isAwake)
  : timeout (0, 0, 0, DEFAULT_TIMEOUT_SEC)
  , wait_online1_sec(DEFAULT_WAIT_FOR_ONLINE_SEC_1)
  , wait_online2_sec(DEFAULT_WAIT_FOR_ONLINE_SEC_2)
  , wait_services_sec(DEFAULT_WAIT_FOR_SERVICES_SEC)
  , ping_port(0), ping_mode(0)
{
  nextWake = CDateTime::GetCurrentDateTime();

  if (isAwake)
    nextWake += timeout;
}

//**

class CMACDiscoveryJob : public CJob
{
public:
  CMACDiscoveryJob(const CStdString& host) : m_host(host) {}

  virtual bool DoWork();

  const CStdString& GetMAC() const { return m_macAddres; }
  const CStdString& GetHost() const { return m_host; }

private:
  CStdString m_macAddres;
  CStdString m_host;
};

bool CMACDiscoveryJob::DoWork()
{
  unsigned long ipAddress = HostToIP(m_host);

  if (ipAddress == INADDR_NONE)
  {
    CLog::Log(LOGERROR, "%s - can't determine ip of '%s'", __FUNCTION__, m_host.c_str());
    return false;
  }

  vector<CNetworkInterface*>& ifaces = g_application.getNetwork().GetInterfaceList();
  for (vector<CNetworkInterface*>::const_iterator it = ifaces.begin(); it != ifaces.end(); ++it)
  {
    if ((*it)->GetHostMacAddress(ipAddress, m_macAddres))
      return true;
  }

  return false;
}

//**

class WaitCondition
{
public:
  virtual bool SuccessWaiting () const { return false; }
};

//

class NestDetect
{
public:
  NestDetect() : m_gui_thread (g_application.IsCurrentThread())
  {
    if (m_gui_thread)
      ++m_nest;
  }
  ~NestDetect()
  {
    if (m_gui_thread)
      m_nest--;
  }
  static int Level()
  {
    return m_nest;
  }
  bool IsNested() const
  {
    return m_gui_thread && m_nest > 1;
  }

private:
  static int m_nest;
  const bool m_gui_thread;
};
int NestDetect::m_nest = 0;

//

class ProgressDialogHelper
{
public:
  ProgressDialogHelper (const CStdString& heading) : m_dialog(0)
  {
    if (g_application.IsCurrentThread())
      m_dialog = (CGUIDialogProgress*) g_windowManager.GetWindow(WINDOW_DIALOG_PROGRESS);

    if (m_dialog)
    {
      m_dialog->SetHeading (heading); 
      m_dialog->SetLine(0, "");
      m_dialog->SetLine(1, "");
      m_dialog->SetLine(2, "");

      int nest_level = NestDetect::Level();
      if (nest_level > 1)
      {
        CStdString nest = StringUtils::Format("Nesting:%d", nest_level);
        m_dialog->SetLine(2, nest);
      }
    }
  }
  ~ProgressDialogHelper ()
  {
    if (m_dialog)
      m_dialog->Close();
  }

  bool HasDialog() const { return m_dialog != 0; }

  enum wait_result { TimedOut, Canceled, Success };

  wait_result ShowAndWait (const WaitCondition& waitObj, unsigned timeOutSec, const CStdString& line1)
  {
    unsigned timeOutMs = timeOutSec * 1000;

    if (m_dialog)
    {
      m_dialog->SetLine(1, line1);

      m_dialog->SetPercentage(1); // avoid flickering by starting at 1% ..
    }

    XbmcThreads::EndTime end_time (timeOutMs);

    while (!end_time.IsTimePast())
    {
      if (waitObj.SuccessWaiting())
        return Success;
            
      if (m_dialog)
      {
        if (!m_dialog->IsActive())
          m_dialog->StartModal();

        if (m_dialog->IsCanceled())
          return Canceled;

        m_dialog->Progress();

        unsigned ms_passed = timeOutMs - end_time.MillisLeft();

        int percentage = (ms_passed * 100) / timeOutMs;
        m_dialog->SetPercentage(max(percentage, 1)); // avoid flickering , keep minimum 1%
      }

      Sleep (m_dialog ? 20 : 200);
    }

    return TimedOut;
  }

private:
  CGUIDialogProgress* m_dialog;
};

class NetworkStartWaiter : public WaitCondition
{
public:
  NetworkStartWaiter (unsigned settle_time_ms, const CStdString& host) : m_settle_time_ms (settle_time_ms), m_host(host)
  {
  }
  virtual bool SuccessWaiting () const
  {
    unsigned long address = ntohl(HostToIP(m_host));
    bool online = g_application.getNetwork().HasInterfaceForIP(address);

    if (!online) // setup endtime so we dont return true until network is consistently connected
      m_end.Set (m_settle_time_ms);

    return online && m_end.IsTimePast();
  }
private:
  mutable XbmcThreads::EndTime m_end;
  unsigned m_settle_time_ms;
  const CStdString m_host;
};

class PingResponseWaiter : public WaitCondition, private IJobCallback
{
public:
  PingResponseWaiter (bool async, const CWakeOnAccess::WakeUpEntry& server) 
    : m_server(server), m_jobId(0), m_hostOnline(false)
  {
    if (async)
    {
      CJob* job = new CHostProberJob(server);
      m_jobId = CJobManager::GetInstance().AddJob(job, this);
    }
  }
  ~PingResponseWaiter()
  {
    CJobManager::GetInstance().CancelJob(m_jobId);
  }
  virtual bool SuccessWaiting () const
  {
    return m_jobId ? m_hostOnline : Ping(m_server);
  }

  virtual void OnJobComplete(unsigned int jobID, bool success, CJob *job)
  {
    m_hostOnline = success;
  }

  static bool Ping (const CWakeOnAccess::WakeUpEntry& server)
  {
    ULONG dst_ip = HostToIP(server.host);

    return g_application.getNetwork().PingHost(dst_ip, server.ping_port, 2000, server.ping_mode & 1);
  }

private:
  class CHostProberJob : public CJob
  {
    public:
      CHostProberJob(const CWakeOnAccess::WakeUpEntry& server) : m_server (server) {}

      virtual bool DoWork()
      {
        while (!ShouldCancel(0,0))
        {
          if (PingResponseWaiter::Ping(m_server))
            return true;
        }
        return false;
      }

    private:
      const CWakeOnAccess::WakeUpEntry& m_server;
  };

  const CWakeOnAccess::WakeUpEntry& m_server;
  unsigned int m_jobId;
  bool m_hostOnline;
};

//

CWakeOnAccess::CWakeOnAccess()
  : m_netinit_sec(DEFAULT_NETWORK_INIT_SEC)    // wait for network to connect
  , m_netsettle_ms(DEFAULT_NETWORK_SETTLE_MS)  // wait for network to settle
  , m_enabled(false)
{
}

CWakeOnAccess &CWakeOnAccess::Get()
{
  static CWakeOnAccess sWakeOnAccess;
  return sWakeOnAccess;
}

bool CWakeOnAccess::WakeUpHost(const CURL& url)
{
  CStdString hostName = GetStoredUrlHost (url);

  if (!hostName.empty())
    return WakeUpHost (hostName, url.Get());
  return true;
}

bool CWakeOnAccess::WakeUpHost (const CStdString& hostName, const string& customMessage)
{
  if (!IsEnabled())
    return true; // bail if feature is turned off

  WakeUpEntry server;

  if (FindOrTouchHostEntry(hostName, server))
  {
    CLog::Log(LOGNOTICE,"WakeOnAccess [%s] trigged by accessing : %s", hostName.c_str(), customMessage.c_str());

    NestDetect nesting ; // detect recursive calls on gui thread..

    if (nesting.IsNested()) // we might get in trouble if it gets called back in loop
      CLog::Log(LOGWARNING,"WakeOnAccess recursively called on gui-thread [%d]", NestDetect::Level());

    bool ret = WakeUpHost(server);

    if (!ret) // extra log if we fail for some reason
      CLog::Log(LOGWARNING,"WakeOnAccess failed to bring up [%s] - there may be trouble ahead !", hostName.c_str());

    TouchHostEntry(hostName);

    return ret;
  }
  return true;
}

#define LOCALIZED(id) g_localizeStrings.Get(id)

bool CWakeOnAccess::WakeUpHost(const WakeUpEntry& server)
{
  CStdString heading = StringUtils::Format(LOCALIZED(13027), server.host.c_str());

  ProgressDialogHelper dlg (heading);

  {
    NetworkStartWaiter waitObj (m_netsettle_ms, server.host); // wait until network connected before sending wake-on-lan

    if (dlg.ShowAndWait (waitObj, m_netinit_sec, LOCALIZED(13028)) != ProgressDialogHelper::Success)
    {
      CLog::Log(LOGNOTICE,"WakeOnAccess timeout/cancel while waiting for network");
      return false; // timedout or canceled
    }
  }

  {
    ULONG dst_ip = HostToIP(server.host);

    if (g_application.getNetwork().PingHost(dst_ip, server.ping_port, 500)) // quick ping with short timeout to not block too long
    {
      CLog::Log(LOGNOTICE,"WakeOnAccess success exit, server already running");
      return true;
    }
  }

  if (!g_application.getNetwork().WakeOnLan(server.mac.c_str()))
  {
    CLog::Log(LOGERROR,"WakeOnAccess failed to send. (Is it blocked by firewall?)");

    if (g_application.IsCurrentThread() || !g_application.m_pPlayer->IsPlaying())
      CGUIDialogKaiToast::QueueNotification(CGUIDialogKaiToast::Error, heading, LOCALIZED(13029));
    return false;
  }

  {
    PingResponseWaiter waitObj (dlg.HasDialog(), server); // wait for ping response ..

    ProgressDialogHelper::wait_result 
      result = dlg.ShowAndWait (waitObj, server.wait_online1_sec, LOCALIZED(13030));

    if (result == ProgressDialogHelper::TimedOut)
      result = dlg.ShowAndWait (waitObj, server.wait_online2_sec, LOCALIZED(13031));

    if (result != ProgressDialogHelper::Success)
    {
      CLog::Log(LOGNOTICE,"WakeOnAccess timeout/cancel while waiting for response");
      return false; // timedout or canceled
    }
  }

  // we have ping response ; just add extra wait-for-services before returning if requested

  {
    WaitCondition waitObj ; // wait uninteruptable fixed time for services ..

    dlg.ShowAndWait (waitObj, server.wait_services_sec, LOCALIZED(13032));

    CLog::Log(LOGNOTICE,"WakeOnAccess sequence completed, server started");
  }
  return true;
}

bool CWakeOnAccess::FindOrTouchHostEntry (const CStdString& hostName, WakeUpEntry& result)
{
  CSingleLock lock (m_entrylist_protect);

  bool need_wakeup = false;

  for (EntriesVector::iterator i = m_entries.begin();i != m_entries.end(); ++i)
  {
    WakeUpEntry& server = *i;

    if (hostName.Equals(server.host.c_str()))
    {
      CDateTime now = CDateTime::GetCurrentDateTime();

      if (now > server.nextWake)
      {
        result = server;
        need_wakeup = true;
      }
      else // 'touch' next wakeup time
      {
        server.nextWake = now + server.timeout;
      }

      break;
    }
  }

  return need_wakeup;
}

void CWakeOnAccess::TouchHostEntry (const CStdString& hostName)
{
  CSingleLock lock (m_entrylist_protect);

  for (EntriesVector::iterator i = m_entries.begin();i != m_entries.end(); ++i)
  {
    WakeUpEntry& server = *i;

    if (hostName.Equals(server.host.c_str()))
    {
      server.nextWake = CDateTime::GetCurrentDateTime() + server.timeout;
      return;
    }
  }
}

static void AddHost (const CStdString& host, vector<string>& hosts)
{
  for (vector<string>::const_iterator it = hosts.begin(); it != hosts.end(); ++it)
    if (host.Equals((*it).c_str()))
      return; // allready there ..

  if (!host.empty())
    hosts.push_back(host);
}

static void AddHostFromDatabase(const DatabaseSettings& setting, vector<string>& hosts)
{
  if (setting.type.Equals("mysql"))
    AddHost(setting.host, hosts);
}

void CWakeOnAccess::QueueMACDiscoveryForHost(const CStdString& host)
{
  if (IsEnabled())
  {
    if (URIUtils::IsHostOnLAN(host, true))
      CJobManager::GetInstance().AddJob(new CMACDiscoveryJob(host), this);
    else
      CLog::Log(LOGNOTICE, "%s - skip Mac discovery for non-local host '%s'", __FUNCTION__, host.c_str());
  }
}

static void AddHostsFromMediaSource(const CMediaSource& source, std::vector<std::string>& hosts)
{
  for (vector<string>::const_iterator it = source.vecPaths.begin() ; it != source.vecPaths.end(); it++)
  {
    CURL url(*it);

    AddHost (GetAndStoreUrlHost(url), hosts);
  }
}

static void AddHostsFromVecSource(const VECSOURCES& sources, vector<string>& hosts)
{
  for (VECSOURCES::const_iterator it = sources.begin(); it != sources.end(); it++)
    AddHostsFromMediaSource(*it, hosts);
}

static void AddHostsFromVecSource(const VECSOURCES* sources, vector<string>& hosts)
{
  if (sources)
    AddHostsFromVecSource(*sources, hosts);
}

void CWakeOnAccess::QueueMACDiscoveryForAllRemotes()
{
  vector<string> hosts;

#ifdef HAS_UPNP
  upnp_server_list_t old_list;
  upnp_servers.MoveTo(old_list);
#endif

  // add media sources
  CMediaSourceSettings& ms = CMediaSourceSettings::Get();

  AddHostsFromVecSource(ms.GetSources("video"), hosts);
  AddHostsFromVecSource(ms.GetSources("music"), hosts);
  AddHostsFromVecSource(ms.GetSources("files"), hosts);
  AddHostsFromVecSource(ms.GetSources("pictures"), hosts);
  AddHostsFromVecSource(ms.GetSources("programs"), hosts);

#ifdef HAS_UPNP
  upnp_servers.LearnFrom(old_list);
#endif

  // add mysql servers
  AddHostFromDatabase(g_advancedSettings.m_databaseVideo, hosts);
  AddHostFromDatabase(g_advancedSettings.m_databaseMusic, hosts);
  AddHostFromDatabase(g_advancedSettings.m_databaseEpg, hosts);
  AddHostFromDatabase(g_advancedSettings.m_databaseTV, hosts);

  // add from path substitutions ..
  for (CAdvancedSettings::StringMapping::iterator i = g_advancedSettings.m_pathSubstitutions.begin(); i != g_advancedSettings.m_pathSubstitutions.end(); ++i)
  {
    CURL url(i->second);

    AddHost (url.GetHostName(), hosts);
  }

  for (vector<string>::const_iterator it = hosts.begin(); it != hosts.end(); it++)
    QueueMACDiscoveryForHost(*it);
}

void CWakeOnAccess::SaveMACDiscoveryResult(const CStdString& host, const CStdString& mac)
{
  CLog::Log(LOGNOTICE, "%s - Mac discovered for host '%s' -> '%s'", __FUNCTION__, host.c_str(), mac.c_str());

  CStdString heading = LOCALIZED(13033);

  for (EntriesVector::iterator i = m_entries.begin(); i != m_entries.end(); ++i)
  {
    if (host.Equals(i->host.c_str()))
    {
      CLog::Log(LOGDEBUG, "%s - Update existing entry for host '%s'", __FUNCTION__, host.c_str());
      if (!mac.Equals(i->mac.c_str()))
      {
        if (IsEnabled()) // show notification only if we have general feature enabled
        {
          CStdString message = StringUtils::Format(LOCALIZED(13034), host.c_str());
          CGUIDialogKaiToast::QueueNotification(CGUIDialogKaiToast::Info, heading, message, 4000, true, 3000);
        }

        i->mac = mac;
        SaveToXML();
      }
#ifdef HAS_UPNP
      else if (upnp_servers.m_is_dirty)
      {
        SaveToXML();
      }
#endif

      return;
    }
  }

  // not found entry to update - create using default values
  WakeUpEntry entry (true);
  entry.host = host;
  entry.mac  = mac;
  m_entries.push_back(entry);

  CLog::Log(LOGDEBUG, "%s - Create new entry for host '%s'", __FUNCTION__, host.c_str());
  if (IsEnabled()) // show notification only if we have general feature enabled
  {
    CStdString message = StringUtils::Format(LOCALIZED(13035), host.c_str());
    CGUIDialogKaiToast::QueueNotification(CGUIDialogKaiToast::Info, heading, message, 4000, true, 3000);
  }

  SaveToXML();
}

void CWakeOnAccess::OnJobComplete(unsigned int jobID, bool success, CJob *job)
{
  CMACDiscoveryJob* discoverJob = (CMACDiscoveryJob*)job;

  const CStdString& host = discoverJob->GetHost();
  const CStdString& mac = discoverJob->GetMAC();

  if (success)
  {
    CSingleLock lock (m_entrylist_protect);

    SaveMACDiscoveryResult(host, mac);
  }
  else
  {
    CLog::Log(LOGERROR, "%s - Mac discovery failed for host '%s'", __FUNCTION__, host.c_str());

    if (IsEnabled())
    {
      CStdString heading = LOCALIZED(13033);
      CStdString message = StringUtils::Format(LOCALIZED(13036), host.c_str());
      CGUIDialogKaiToast::QueueNotification(CGUIDialogKaiToast::Error, heading, message, 4000, true, 3000);
    }
  }
}

CStdString CWakeOnAccess::GetSettingFile()
{
  return CSpecialProtocol::TranslatePath("special://masterprofile/wakeonlan.xml");
}

void CWakeOnAccess::OnSettingsLoaded()
{
  CSingleLock lock (m_entrylist_protect);

  LoadFromXML();
}

void CWakeOnAccess::OnSettingsSaved()
{
  bool enabled = CSettings::Get().GetBool("powermanagement.wakeonaccess");

  if (enabled != IsEnabled())
  {
    SetEnabled(enabled);

    if (enabled)
      QueueMACDiscoveryForAllRemotes();
  }
}

void CWakeOnAccess::SetEnabled(bool enabled) 
{
  m_enabled = enabled;

  CLog::Log(LOGNOTICE,"WakeOnAccess - Enabled:%s", m_enabled ? "TRUE" : "FALSE");
}

void CWakeOnAccess::LoadFromXML()
{
  bool enabled = CSettings::Get().GetBool("powermanagement.wakeonaccess");

  CXBMCTinyXML xmlDoc;
  if (!xmlDoc.LoadFile(GetSettingFile()))
  {
    if (enabled)
      CLog::Log(LOGNOTICE, "%s - unable to load:%s", __FUNCTION__, GetSettingFile().c_str());
    return;
  }

  TiXmlElement* pRootElement = xmlDoc.RootElement();
  if (strcmpi(pRootElement->Value(), "onaccesswakeup"))
  {
    CLog::Log(LOGERROR, "%s - XML file %s doesnt contain <onaccesswakeup>", __FUNCTION__, GetSettingFile().c_str());
    return;
  }

  m_entries.clear();

  CLog::Log(LOGNOTICE,"WakeOnAccess - Load settings :");

  SetEnabled(enabled);

  int tmp;
  if (XMLUtils::GetInt(pRootElement, "netinittimeout", tmp, 0, 5 * 60))
    m_netinit_sec = tmp;
  CLog::Log(LOGNOTICE,"  -Network init timeout : [%d] sec", m_netinit_sec);
  
  if (XMLUtils::GetInt(pRootElement, "netsettletime", tmp, 0, 5 * 1000))
    m_netsettle_ms = tmp;
  CLog::Log(LOGNOTICE,"  -Network settle time  : [%d] ms", m_netsettle_ms);

  const TiXmlNode* pWakeUp = pRootElement->FirstChildElement("wakeup");
  while (pWakeUp)
  {
    WakeUpEntry entry;

    CStdString strtmp;
    if (XMLUtils::GetString(pWakeUp, "host", strtmp))
      entry.host = strtmp;

    if (XMLUtils::GetString(pWakeUp, "mac", strtmp))
      entry.mac = strtmp;

    if (entry.host.empty())
      CLog::Log(LOGERROR, "%s - Missing <host> tag or it's empty", __FUNCTION__);
    else if (entry.mac.empty())
       CLog::Log(LOGERROR, "%s - Missing <mac> tag or it's empty", __FUNCTION__);
    else
    {
      if (XMLUtils::GetInt(pWakeUp, "pingport", tmp, 0, USHRT_MAX))
        entry.ping_port = (unsigned short) tmp;

      if (XMLUtils::GetInt(pWakeUp, "pingmode", tmp, 0, USHRT_MAX))
        entry.ping_mode = (unsigned short) tmp;

      if (XMLUtils::GetInt(pWakeUp, "timeout", tmp, 10, 12 * 60 * 60))
        entry.timeout.SetDateTimeSpan (0, 0, 0, tmp);

      if (XMLUtils::GetInt(pWakeUp, "waitonline", tmp, 0, 10 * 60)) // max 10 minutes
        entry.wait_online1_sec = tmp;

      if (XMLUtils::GetInt(pWakeUp, "waitonline2", tmp, 0, 10 * 60)) // max 10 minutes
        entry.wait_online2_sec = tmp;

      if (XMLUtils::GetInt(pWakeUp, "waitservices", tmp, 0, 5 * 60)) // max 5 minutes
        entry.wait_services_sec = tmp;

      CLog::Log(LOGNOTICE,"  Registering wakeup entry:");
      CLog::Log(LOGNOTICE,"    HostName        : %s", entry.host.c_str());
      CLog::Log(LOGNOTICE,"    MacAddress      : %s", entry.mac.c_str());
      CLog::Log(LOGNOTICE,"    PingPort        : %d", entry.ping_port);
      CLog::Log(LOGNOTICE,"    PingMode        : %d", entry.ping_mode);
      CLog::Log(LOGNOTICE,"    Timeout         : %d (sec)", GetTotalSeconds(entry.timeout));
      CLog::Log(LOGNOTICE,"    WaitForOnline   : %d (sec)", entry.wait_online1_sec);
      CLog::Log(LOGNOTICE,"    WaitForOnlineEx : %d (sec)", entry.wait_online2_sec);
      CLog::Log(LOGNOTICE,"    WaitForServices : %d (sec)", entry.wait_services_sec);

      m_entries.push_back(entry);
    }

    pWakeUp = pWakeUp->NextSiblingElement("wakeup"); // get next one
  }

#ifdef HAS_UPNP
  {
    upnp_servers.Clear();

    const TiXmlNode* pUPnPNode = pRootElement->FirstChildElement("upnp_map");
    while (pUPnPNode)
    {
      UPnpServer server;

      XMLUtils::GetString(pUPnPNode, "uuid", server.uuid);
      XMLUtils::GetString(pUPnPNode, "host", server.host);

      if (server.host.empty() || server.uuid.empty())
      {
        CLog::Log(LOGERROR, "%s - Missing or empty <upnp_map> entry", __FUNCTION__);
      }
      else
      {
        CLog::Log(LOGNOTICE,"  Registering upnp_map entry [%s] -> [%s]", server.uuid.c_str(), server.host.c_str());

        upnp_servers.StoreServer(server);
      }

      pUPnPNode = pUPnPNode->NextSiblingElement("upnp_map"); // get next one
    }
  }
#endif
}

void CWakeOnAccess::SaveToXML()
{
  CXBMCTinyXML xmlDoc;
  TiXmlElement xmlRootElement("onaccesswakeup");
  TiXmlNode *pRoot = xmlDoc.InsertEndChild(xmlRootElement);
  if (!pRoot) return;

  XMLUtils::SetInt(pRoot, "netinittimeout", m_netinit_sec);
  XMLUtils::SetInt(pRoot, "netsettletime", m_netsettle_ms);

  for (EntriesVector::const_iterator i = m_entries.begin(); i != m_entries.end(); ++i)
  {
    TiXmlElement xmlSetting("wakeup");
    TiXmlNode* pWakeUpNode = pRoot->InsertEndChild(xmlSetting);
    if (pWakeUpNode)
    {
      XMLUtils::SetString(pWakeUpNode, "host", i->host);
      XMLUtils::SetString(pWakeUpNode, "mac", i->mac);
      XMLUtils::SetInt(pWakeUpNode, "pingport", i->ping_port);
      XMLUtils::SetInt(pWakeUpNode, "pingmode", i->ping_mode);
      XMLUtils::SetInt(pWakeUpNode, "timeout", GetTotalSeconds(i->timeout));
      XMLUtils::SetInt(pWakeUpNode, "waitonline", i->wait_online1_sec);
      XMLUtils::SetInt(pWakeUpNode, "waitonline2", i->wait_online2_sec);
      XMLUtils::SetInt(pWakeUpNode, "waitservices", i->wait_services_sec);
    }
  }

#ifdef HAS_UPNP
  {
    CSingleLock lock (upnp_servers.m_crit_section);

    for (UPnPServerList::const_iterator i = upnp_servers.begin(); i != upnp_servers.end(); ++i)
    {
      TiXmlElement xmlSetting("upnp_map");
      TiXmlNode* pUPnPNode = pRoot->InsertEndChild(xmlSetting);
      if (pUPnPNode)
      {
        XMLUtils::SetString(pUPnPNode, "uuid", i->uuid);
        XMLUtils::SetString(pUPnPNode, "host", i->host);
      }
    }

    upnp_servers.m_is_dirty = false;
  }
#endif

  xmlDoc.SaveFile(GetSettingFile());
}
