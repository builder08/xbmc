#pragma once
/*
 *      Copyright (C) 2005-2010 Team XBMC
 *      http://www.xbmc.org
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
 *  along with XBMC; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#include "StdString.h"
#include "JSONRPC.h"
#include "LibraryBase.h"

namespace JSONRPC
{
  class CMusicLibrary : public CLibraryBase
  {
  public:
    static JSON_STATUS GetArtists(const CStdString &method, ITransportLayer *transport, const Json::Value& parameterObject, Json::Value &result);
    static JSON_STATUS GetAlbums(const CStdString &method, ITransportLayer *transport, const Json::Value& parameterObject, Json::Value &result);
    static JSON_STATUS GetSongs(const CStdString &method, ITransportLayer *transport, const Json::Value& parameterObject, Json::Value &result);
    static JSON_STATUS GetSongInfo(const CStdString &method, ITransportLayer *transport, const Json::Value& parameterObject, Json::Value &result);
  };
}
