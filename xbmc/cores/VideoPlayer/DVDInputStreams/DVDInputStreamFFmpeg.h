/*
 *      Copyright (C) 2005-2013 Team XBMC
 *      http://kodi.tv
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

#pragma once

#include "DVDInputStream.h"

class CDVDInputStreamFFmpeg
  : public CDVDInputStream
{
public:
  explicit CDVDInputStreamFFmpeg(const CFileItem& fileitem);
  ~CDVDInputStreamFFmpeg() override;
  bool Open() override;
  void Close() override;
  int Read(uint8_t* buf, int buf_size) override;
  int64_t Seek(int64_t offset, int whence) override;
  bool Pause(double dTime) override { return false; };
  bool IsEOF() override;
  int64_t GetLength() override;
  std::string GetFileName() override;

  void  Abort() override { m_aborted = true;  }
  bool Aborted() { return m_aborted;  }

  const CFileItem& GetItem() const { return m_item; }

  bool CanSeek() override { return m_can_seek; }
  bool CanPause() override { return m_can_pause; }
  void SetRealtime(bool realtime) override;

  std::string GetProxyType() const;
  std::string GetProxyHost() const;
  uint16_t GetProxyPort() const;
  std::string GetProxyUser() const;
  std::string GetProxyPassword() const;

protected:
  bool m_can_pause;
  bool m_can_seek;
  bool m_aborted;
};
