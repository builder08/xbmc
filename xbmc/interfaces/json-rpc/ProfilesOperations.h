#pragma once
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

#ifndef JSON_RPC_UTILS_STDSTRING_H_INCLUDED
#define JSON_RPC_UTILS_STDSTRING_H_INCLUDED
#include "utils/StdString.h"
#endif

#ifndef JSON_RPC_JSONRPC_H_INCLUDED
#define JSON_RPC_JSONRPC_H_INCLUDED
#include "JSONRPC.h"
#endif

#ifndef JSON_RPC_FILEITEMHANDLER_H_INCLUDED
#define JSON_RPC_FILEITEMHANDLER_H_INCLUDED
#include "FileItemHandler.h"
#endif


namespace JSONRPC
{
  class CProfilesOperations : CFileItemHandler
  {
  public:
    static JSONRPC_STATUS GetProfiles(const CStdString &method, ITransportLayer *transport, IClient *client, const CVariant &parameterObject, CVariant &result);
    static JSONRPC_STATUS GetCurrentProfile(const CStdString &method, ITransportLayer *transport, IClient *client, const CVariant &parameterObject, CVariant &result);
    static JSONRPC_STATUS LoadProfile(const CStdString &method, ITransportLayer *transport, IClient *client, const CVariant &parameterObject, CVariant &result);
  };
}
