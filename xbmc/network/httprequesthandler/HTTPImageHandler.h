#pragma once
/*
 *      Copyright (C) 2012-2013 Team XBMC
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

#ifndef HTTPREQUESTHANDLER_IHTTPREQUESTHANDLER_H_INCLUDED
#define HTTPREQUESTHANDLER_IHTTPREQUESTHANDLER_H_INCLUDED
#include "IHTTPRequestHandler.h"
#endif


#ifndef HTTPREQUESTHANDLER_UTILS_STDSTRING_H_INCLUDED
#define HTTPREQUESTHANDLER_UTILS_STDSTRING_H_INCLUDED
#include "utils/StdString.h"
#endif


class CHTTPImageHandler : public IHTTPRequestHandler
{
public:
  CHTTPImageHandler() { };

  virtual IHTTPRequestHandler* GetInstance() { return new CHTTPImageHandler(); }
  virtual bool CheckHTTPRequest(const HTTPRequest &request);
  virtual int HandleHTTPRequest(const HTTPRequest &request);

  virtual std::string GetHTTPResponseFile() const { return m_path; }

  virtual int GetPriority() const { return 2; }

private:
  CStdString m_path;
};
