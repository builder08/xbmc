#pragma once

/*
 *      Copyright (c) 2002 Frodo
 *      Portions Copyright (c) by the authors of ffmpeg and xvid
 *
 *      Copyright (C) 2002-2013 Team XBMC
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

#ifndef NETWORK_UTILS_STDSTRING_H_INCLUDED
#define NETWORK_UTILS_STDSTRING_H_INCLUDED
#include "utils/StdString.h"
#endif

#ifndef NETWORK_THREADS_THREAD_H_INCLUDED
#define NETWORK_THREADS_THREAD_H_INCLUDED
#include "threads/Thread.h"
#endif

#ifndef NETWORK_THREADS_CRITICALSECTION_H_INCLUDED
#define NETWORK_THREADS_CRITICALSECTION_H_INCLUDED
#include "threads/CriticalSection.h"
#endif

#include <sys/socket.h>
#include <netinet/in.h>
#ifndef NETWORK_SYSTEM_H_INCLUDED
#define NETWORK_SYSTEM_H_INCLUDED
#include "system.h"
#endif


class CUdpClient : CThread
{
public:
  CUdpClient();
  virtual ~CUdpClient(void);

protected:

  bool Create();
  void Destroy();

  void OnStartup();
  void Process();

  bool Broadcast(int aPort, CStdString& aMessage);
  bool Send(CStdString aIpAddress, int aPort, CStdString& aMessage);
  bool Send(SOCKADDR_IN aAddress, CStdString& aMessage);
  bool Send(SOCKADDR_IN aAddress, LPBYTE pMessage, DWORD dwSize);

  virtual void OnMessage(SOCKADDR_IN& aRemoteAddress, CStdString& aMessage, LPBYTE pMessage, DWORD dwMessageLength){};

protected:

  struct UdpCommand
  {
    SOCKADDR_IN address;
    CStdString message;
    LPBYTE binary;
    DWORD binarySize;
  };

  bool DispatchNextCommand();

  SOCKET client_socket;

  std::vector<UdpCommand> commands;
  typedef std::vector<UdpCommand> ::iterator COMMANDITERATOR;

  CCriticalSection critical_section;
};
