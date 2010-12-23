/*
 *      Copyright (C) 2005-2009 Team XBMC
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
#include "Service.h"
#include "AddonManager.h"
#include "log.h"
#ifdef HAS_PYTHON
#include "lib/libPython/XBPython.h"
#endif

using namespace std;

namespace ADDON
{

CService::CService(const cp_extension_t *ext)
  : CAddon(ext), m_type(UNKNOWN)
{
  BuildServiceType();
}


CService::CService(const AddonProps &props)
  : CAddon(props), m_type(UNKNOWN)
{
  BuildServiceType();
}

bool CService::Start()
{
  bool ret = true;
  switch (m_type)
  {
#ifdef HAS_PYTHON
  case PYTHON:
    ret = (g_pythonParser.evalFile(LibPath()) != -1);
    break;
#endif

  case UNKNOWN:
  default:
    ret = false;
    break;
  }

  return ret;
}

bool CService::Stop()
{
  bool ret = true;

  switch (m_type)
  {
#ifdef HAS_PYTHON
  case PYTHON:
    ret = g_pythonParser.StopScript(LibPath());
    break;
#endif

  case UNKNOWN:
  default:
    ret = false;
    break;
  }

  return ret;
}

void CService::BuildServiceType()
{
  CStdString str = LibPath();
  CStdString ext;

  size_t p = str.find_last_of('.');
  if (p != string::npos)
    ext = str.substr(p + 1);

#ifdef HAS_PYTHON
  CStdString pythonExt = ADDON_PYTHON_EXT;
  pythonExt.erase(0, 2);
  if ( ext.Equals(pythonExt) )
    m_type = PYTHON;
  else
#endif
  {
    m_type = UNKNOWN;
    CLog::Log(LOGERROR, "ADDON: extension '%s' is not currently supported for service addon", ext.c_str());
  }
}

}
