#pragma once

/*
 *      Copyright (C) 2005-present Team Kodi
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
