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

#ifndef DVDINPUTSTREAMS_DVDINPUTSTREAM_H_INCLUDED
#define DVDINPUTSTREAMS_DVDINPUTSTREAM_H_INCLUDED
#include "DVDInputStream.h"
#endif

#ifndef DVDINPUTSTREAMS_URL_H_INCLUDED
#define DVDINPUTSTREAMS_URL_H_INCLUDED
#include "URL.h"
#endif


CDVDInputStream::CDVDInputStream(DVDStreamType streamType)
{
  m_streamType = streamType;
}

CDVDInputStream::~CDVDInputStream()
{
}

bool CDVDInputStream::Open(const char* strFile, const std::string &content)
{
  CURL url(strFile);

  m_url = url;
  // get rid of any protocol options which might have sneaked in here
  // but keep them in m_url.
  url.SetProtocolOptions("");
  m_strFileName = url.Get();

  m_content = content;
  return true;
}

void CDVDInputStream::Close()
{
  m_strFileName = "";
  m_item.Reset();
}

void CDVDInputStream::SetFileItem(const CFileItem& item)
{
  m_item = item;
}
