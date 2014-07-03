#pragma once
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

#include "utils/StdString.h"
#include "utils/UrlOptions.h"

#ifdef TARGET_WINDOWS
#undef SetPort // WIN32INCLUDES this is defined as SetPortA in WinSpool.h which is being included _somewhere_
#ifdef GetUserName
#undef GetUserName
#endif // GetUserName
#endif

class CURL
{
public:
  explicit CURL(const CStdString& strURL);
  CURL();
  virtual ~CURL(void);

  // explicit equals operator for std::string comparison
  bool operator==(const std::string &url) const { return Get() == url; }

  void Reset();
  void Parse(const CStdString& strURL);
  void SetFileName(const CStdString& strFileName);
  void SetHostName(const CStdString& strHostName);
  void SetUserName(const CStdString& strUserName);
  void SetPassword(const CStdString& strPassword);
  void SetProtocol(const CStdString& strProtocol);
  void SetOptions(const CStdString& strOptions);
  void SetProtocolOptions(const CStdString& strOptions);
  void SetPort(int port);

  bool HasPort() const;

  int GetPort() const;
  const CStdString& GetHostName() const;
  const CStdString& GetDomain() const;
  const CStdString& GetUserName() const;
  const CStdString& GetPassWord() const;
  const CStdString& GetFileName() const;
  const CStdString& GetProtocol() const;
  const std::string GetBaseProtocol(void) const;
  const CStdString& GetFileType() const;
  const CStdString& GetShareName() const;
  const CStdString& GetOptions() const;
  const CStdString& GetProtocolOptions() const;
  const CStdString GetFileNameWithoutPath() const; /* return the filename excluding path */

  inline char GetDirectorySeparatorPrimary(void) const
  {
#ifdef TARGET_WINDOWS
    if (m_strProtocol.empty())
      return '\\'; // win32 local filesystem
#endif
    return '/';
  }
  inline char GetDirectorySeparatorAdditional(void) const
  {
#ifdef TARGET_WINDOWS
    if (m_strProtocol.empty())
      return '/';
#endif
    return 0; // no additional separator
  }


  CStdString Get() const;
  std::string GetWithoutUserDetails(bool redact = false) const;
  CStdString GetWithoutFilename() const;
  std::string GetRedacted() const;
  static std::string GetRedacted(const std::string& path);
  bool IsLocal() const;
  bool IsLocalHost() const;
  static bool IsFileOnly(const std::string& url); ///< return true if there are no directories in the url.
  static bool IsFullPath(const std::string& url); ///< return true if the url includes the full path
  static std::string Decode(const std::string& strURLData);
  static std::string Encode(const std::string& strURLData);
  static std::string BaseProtocol(const std::string& prot);

  void GetOptions(std::map<CStdString, CStdString> &options) const;
  bool HasOption(const std::string& key) const;
  bool GetOption(const std::string& key, std::string& value) const;
  std::string GetOption(const std::string& key) const;
  void SetOption(const std::string& key, const std::string& value);
  void RemoveOption(const std::string& key);

  void GetProtocolOptions(std::map<CStdString, CStdString> &options) const;
  bool HasProtocolOption(const std::string& key) const;
  bool GetProtocolOption(const std::string& key, std::string& value) const;
  std::string GetProtocolOption(const std::string& key) const;
  void SetProtocolOption(const std::string& key, const std::string& value);
  void RemoveProtocolOption(const std::string& key);

protected:
  int m_iPort;
  CStdString m_strHostName;
  CStdString m_strShareName;
  CStdString m_strDomain;
  CStdString m_strUserName;
  CStdString m_strPassword;
  CStdString m_strFileName;
  CStdString m_strProtocol;
  CStdString m_strFileType;
  CStdString m_strOptions;
  CStdString m_strProtocolOptions;
  CUrlOptions m_options;
  CUrlOptions m_protocolOptions;
};
