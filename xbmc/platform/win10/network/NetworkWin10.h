/*
 *  Copyright (C) 2005-2018 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#pragma once

#include "network/Network.h"
#include "threads/CriticalSection.h"
#include "utils/stopwatch.h"

#include <string>
#include <vector>

#include <IPTypes.h>
#include <winrt/Windows.Networking.Connectivity.h>

class CNetworkWin10;

class CNetworkInterfaceWin10 : public CNetworkInterface
{
public:
  CNetworkInterfaceWin10(CNetworkWin10* network,
                         const PIP_ADAPTER_ADDRESSES adapter,
                         ::IUnknown* winRTadapter);
  ~CNetworkInterfaceWin10(void);

  virtual const std::string& GetName(void) const;

  virtual bool IsEnabled(void) const;
  virtual bool IsConnected(void) const;

  virtual std::string GetMacAddress(void) const;
  virtual void GetMacAddressRaw(char rawMac[6]) const;

  virtual bool GetHostMacAddress(unsigned long host, std::string& mac) const;

  virtual std::string GetCurrentIPAddress() const;
  virtual std::string GetCurrentNetmask() const;
  virtual std::string GetCurrentDefaultGateway(void) const;

private:
  CNetworkWin10* m_network;

  std::string m_adaptername;
  PIP_ADAPTER_ADDRESSES m_adapterAddr;
  winrt::Windows::Networking::Connectivity::NetworkAdapter m_winRT = nullptr;
  mutable winrt::Windows::Networking::Connectivity::ConnectionProfile m_profile = nullptr;
};


class CNetworkWin10 : public CNetworkBase
{
public:
    CNetworkWin10();
    virtual ~CNetworkWin10(void);

    std::vector<CNetworkInterface*>& GetInterfaceList(void) override;
    CNetworkInterface* GetDefaultInterface() override;
    std::vector<std::string> GetNameServers(void) override;

    bool PingHost(unsigned long host, unsigned int timeout_ms = 2000) override;

    friend class CNetworkInterfaceWin10;
private:
    int GetSocket() { return m_sock; }
    void queryInterfaceList();
    void CleanInterfaceList();

    std::vector<CNetworkInterface*> m_interfaces;
    int m_sock;
    CStopWatch m_netrefreshTimer;
    CCriticalSection m_critSection;
    PIP_ADAPTER_ADDRESSES m_adapterAddresses;
};

using CNetwork = CNetworkWin10;
