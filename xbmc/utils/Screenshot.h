#pragma once
/*
 *      Copyright (C) 2005-2012 Team XBMC
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
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include "utils/StdString.h"

class CScreenshotSurface
{

public:
  int            m_width;
  int            m_height;
  int            m_stride;
  unsigned char* m_buffer;

  CScreenshotSurface(void);
  bool capture( void );
};

class CScreenShot
{

public:
  static void TakeScreenshot();
  static void TakeScreenshot(const CStdString &filename, bool sync);
};
