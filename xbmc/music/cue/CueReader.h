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
 *  along with XBMC; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#pragma once
#include "utils/StdString.h"

/*! Base class for reading CUE-data from different sources.
 */
class CueReader
{
public:
  virtual bool isValid() const = 0;
  inline bool skipChar(char ch) const
  {
    return (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n');
  }
  virtual bool ReadNextLine(CStdString &line) = 0;
  virtual ~CueReader() {};
};
