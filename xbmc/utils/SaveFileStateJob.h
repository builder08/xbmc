/*
 *      Copyright (C) 2010-2013 Team XBMC
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
#pragma once

#include "Job.h"
#include "FileItem.h"
#include "video/Bookmark.h"
#include "cores/VideoSettings.h"

class CSaveFileStateJob : public CJob
{
  CFileItem m_item;
  CFileItem m_item_discstack;
  CBookmark m_bookmark;
  bool      m_updatePlayCount;

public:
  CSaveFileStateJob(const CFileItem& item,
                    const CFileItem& item_discstack,
                    const CBookmark& bookmark,
                    bool updatePlayCount)
    : m_item(item),
      m_item_discstack(item_discstack),
      m_bookmark(bookmark),
      m_updatePlayCount(updatePlayCount)
  {
  }
  ~CSaveFileStateJob() override = default;
  bool  DoWork() override;
};

