#include "TCPServer.h"
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include "JSONRPC.h"
#include "../libjsoncpp/json.h"
#include "AnnouncementManager.h"
#include "log.h"

using namespace JSONRPC;
using namespace ANNOUNCEMENT;
using namespace std;
using namespace Json;

#define RECEIVEBUFFER 1024

CTCPServer *CTCPServer::ServerInstance = NULL;

void CTCPServer::StartServer(int port, bool nonlocal)
{
  StopServer(true);

  ServerInstance = new CTCPServer(port, nonlocal);
  ServerInstance->Create();
}

void CTCPServer::StopServer(bool bWait)
{
  if (ServerInstance)
  {
    ServerInstance->StopThread(bWait);
    if (bWait)
    {
      delete ServerInstance;
      ServerInstance = NULL;
    }
  }
}

CTCPServer::CTCPServer(int port, bool nonlocal)
{
  m_port = port;
  m_nonlocal = nonlocal;
  m_ServerSocket = -1;
}

void CTCPServer::Process()
{
  m_bStop = !Initialize();

  while (!m_bStop)
  {
    int             max_fd = 0;
    fd_set          rfds;
    struct timeval  to     = {5, 0};
    FD_ZERO(&rfds);

    FD_SET(m_ServerSocket, &rfds);
    max_fd = m_ServerSocket;

    for (unsigned int i = 0; i < m_connections.size(); i++)
    {
      FD_SET(m_connections[i].m_socket, &rfds);
      if (m_connections[i].m_socket > max_fd)
        max_fd = m_connections[i].m_socket;
    }

    int res = select(max_fd+1, &rfds, NULL, NULL, &to);
    if (res < 0)
    {
      CLog::Log(LOGERROR, "JSONRPC Server: Select failed");
      m_bStop = false;
    }
    else if (res > 0)
    {
      for (int i = m_connections.size() - 1; i >= 0; i--)
      {
        int socket = m_connections[i].m_socket;
        if (FD_ISSET(socket, &rfds))
        {
          char buffer[RECEIVEBUFFER] = {};
          int  nread = 0;
          nread = recv(socket, &buffer, RECEIVEBUFFER, 0);
          if (nread > 0)
          {
            printf("recieved %d bytes from client %d (%*s)\n", nread, socket, nread, buffer);
            m_connections[i].PushBuffer(this, buffer, nread);
          }
          if (nread <= 0)
          {
            CLog::Log(LOGINFO, "JSONRPC Server: Disconnection detected");
            m_connections[i].Disconnect();
            m_connections.erase(m_connections.begin() + i);
          }
        }
      }

      if (FD_ISSET(m_ServerSocket, &rfds))
      {
        CLog::Log(LOGDEBUG, "JSONRPC Server: New connection detected");
        CTCPClient newconnection;
        newconnection.m_socket = accept(m_ServerSocket, &newconnection.m_cliaddr, &newconnection.m_addrlen);

        if (newconnection.m_socket < 0)
          CLog::Log(LOGERROR, "JSONRPC Server: Accept of new connection failed");
        else
        {
          CLog::Log(LOGINFO, "JSONRPC Server: New connection added");
          m_connections.push_back(newconnection);
        }
      }
    }
  }

  Deinitialize();
}

bool CTCPServer::CanAnnounce()
{
  return true;
}

void CTCPServer::Announce(EAnnouncementFlag flag, const char *sender, const char *message, const char *data)
{
  Value root;
  root["jsonrpc"] = "2.0";
  root["method"]  = "Announcement";
  root["params"]["sender"] = sender;
  root["params"]["message"] = message;
  if (data)
    root["params"]["data"] = data;

  StyledWriter writer;
  string str = writer.write(root);

  printf("Announce %s\n", str.c_str());

  for (unsigned int i = 0; i < m_connections.size(); i++)
  {
    unsigned int sent = 0;
    do
    {
      sent += send(m_connections[i].m_socket, str.c_str(), str.size() - sent, sent);
    } while (sent < str.size());
  }
}

bool CTCPServer::Initialize()
{
  Deinitialize();

  struct sockaddr_in myaddr;

  myaddr.sin_family = AF_INET;
  myaddr.sin_port = htons(m_port);
  bool m_bindlocally = true;

  if (m_nonlocal)
    myaddr.sin_addr.s_addr = INADDR_ANY;
  else
    inet_pton(AF_INET, "127.0.0.1", &myaddr.sin_addr.s_addr);

  m_ServerSocket = socket(PF_INET, SOCK_STREAM, 0);

  if (m_ServerSocket < 0)
  {
    CLog::Log(LOGERROR, "JSONRPC Server: Failed to create serversocket");
    return false;
  }

  if (bind(m_ServerSocket, (struct sockaddr*)&myaddr, sizeof myaddr) < 0)
  {
    CLog::Log(LOGERROR, "JSONRPC Server: Failed to bind serversocket");
    close(m_ServerSocket);
    return false;
  }
  
  if (listen(m_ServerSocket, 10) < 0)
  {
    CLog::Log(LOGERROR, "JSONRPC Server: Failed to set listen");
    close(m_ServerSocket);
    return false;
  }

  CAnnouncementManager::AddAnnouncer(this);

  CLog::Log(LOGINFO, "JSONRPC Server: Successfully initialized");
  return true;
}

void CTCPServer::Deinitialize()
{
  for (unsigned int i = 0; i < m_connections.size(); i++)
    m_connections[i].Disconnect();

  m_connections.clear();

  if (m_ServerSocket > 0)
  {
    shutdown(m_ServerSocket, SHUT_RDWR);
    close(m_ServerSocket);
    m_ServerSocket = -1;
  }

  CAnnouncementManager::RemoveAnnouncer(this);
}

CTCPServer::CTCPClient::CTCPClient()
{
  m_announcementflags = 0;
  m_socket = -1;
  m_beginBrackets = 0;
  m_endBrackets = 0;

  m_addrlen = sizeof(struct sockaddr);
}

int CTCPServer::CTCPClient::GetPermissionFlags()
{
  return OPERATION_PERMISSION_ALL;
}

int CTCPServer::CTCPClient::GetAnnouncementFlags()
{
  return m_announcementflags;
}

bool CTCPServer::CTCPClient::SetAnnouncementFlags(int flags)
{
  m_announcementflags = flags;
  return true;
}

void CTCPServer::CTCPClient::PushBuffer(CTCPServer *host, const char *buffer, int length)
{
  for (int i = 0; i < length; i++)
  {
    char c = buffer[i];
    m_buffer.push_back(c);
    if (c == '{')
      m_beginBrackets++;
    else if (c == '}')
      m_endBrackets++;
    if (m_beginBrackets > 0 && m_endBrackets > 0 && m_beginBrackets == m_endBrackets)
    {
      string line = CJSONRPC::MethodCall(m_buffer, host, this);
      send(m_socket, line.c_str(), line.size(), 0);
      m_beginBrackets = m_endBrackets = 0;
      m_buffer.clear();
    }
  }
}

void CTCPServer::CTCPClient::Disconnect()
{
  if (m_socket > 0)
  {
    shutdown(m_socket, SHUT_RDWR);
    close(m_socket);
    m_socket = -1;
  }
}
