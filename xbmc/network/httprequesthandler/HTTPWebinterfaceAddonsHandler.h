#pragma once
/*
 *      Copyright (C) 2011-present Team Kodi
 *      This file is part of Kodi - https://kodi.tv
 *
 *  Kodi is free software: you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  Kodi is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Kodi. If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include <string>

#include "network/httprequesthandler/IHTTPRequestHandler.h"

class CHTTPWebinterfaceAddonsHandler : public IHTTPRequestHandler
{
public:
  CHTTPWebinterfaceAddonsHandler() = default;
  ~CHTTPWebinterfaceAddonsHandler() override = default;
  
  IHTTPRequestHandler* Create(const HTTPRequest &request) const override { return new CHTTPWebinterfaceAddonsHandler(request); }
  bool CanHandleRequest(const HTTPRequest &request) const override;

  int HandleRequest() override;

  HttpResponseRanges GetResponseData() const override;

  int GetPriority() const override { return 4; }

protected:
  explicit CHTTPWebinterfaceAddonsHandler(const HTTPRequest &request)
    : IHTTPRequestHandler(request)
  { }

private:
  std::string m_responseData;
  CHttpResponseRange m_responseRange;
};
