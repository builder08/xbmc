/*
 *      Copyright (C) 2005-2008 Team XBMC
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

#include "system.h"
#include "log.h"
#ifndef _LINUX
#include <share.h>
#include "CharsetConverter.h"
#endif
#include "threads/CriticalSection.h"
#include "threads/SingleLock.h"
#include "settings/AdvancedSettings.h"
#include "threads/Thread.h"

FILE*       CLog::m_file            = NULL;
int         CLog::m_repeatCount     = 0;
int         CLog::m_repeatLogLevel  = -1;
CStdString* CLog::m_repeatLine      = NULL;

static CCriticalSection critSec;

static char levelNames[][8] =
{"DEBUG", "INFO", "NOTICE", "WARNING", "ERROR", "SEVERE", "FATAL", "NONE"};

#ifdef _WIN32
#define LINE_ENDING "\r\n"
#else
#define LINE_ENDING "\n"
#endif


CLog::CLog()
{}

CLog::~CLog()
{}

void CLog::Close()
{
  CSingleLock waitLock(critSec);
  if (m_file)
  {
    fclose(m_file);
    m_file = NULL;
  }
  delete m_repeatLine;
  m_repeatLine = NULL;
}


void CLog::Log(int loglevel, const char *format, ... )
{
  if (g_advancedSettings.m_logLevel > LOG_LEVEL_NORMAL ||
     (g_advancedSettings.m_logLevel > LOG_LEVEL_NONE && loglevel >= LOGNOTICE))
  {
    CSingleLock waitLock(critSec);
    if (!m_file)
      return;

    SYSTEMTIME time;
    GetLocalTime(&time);

    MEMORYSTATUS stat;
    GlobalMemoryStatus(&stat);

    CStdString strPrefix, strData;

    strPrefix.Format("%02.2d:%02.2d:%02.2d T:%"PRIu64" M:%9"PRIu64" %7s: ", time.wHour, time.wMinute, time.wSecond, (uint64_t)CThread::GetCurrentThreadId(), (uint64_t)stat.dwAvailPhys, levelNames[loglevel]);

    strData.reserve(16384);
    va_list va;
    va_start(va, format);
    strData.FormatV(format,va);
    va_end(va);

    if (m_repeatLogLevel == loglevel && *m_repeatLine == strData)
    {
      m_repeatCount++;
      return;
    }
    else if (m_repeatCount)
    {
      CStdString strPrefix2, strData2;
      strPrefix2.Format("%02.2d:%02.2d:%02.2d T:%"PRIu64" M:%9"PRIu64" %7s: ", time.wHour, time.wMinute, time.wSecond, (uint64_t)CThread::GetCurrentThreadId(), (uint64_t)stat.dwAvailPhys, levelNames[m_repeatLogLevel]);

      strData2.Format("Previous line repeats %d times." LINE_ENDING, m_repeatCount);
      fwrite(strPrefix2.c_str(),strPrefix2.size(),1,m_file);
      fwrite(strData2.c_str(),strData2.size(),1,m_file);
#if (defined(_DEBUG) || defined(PROFILE))
      OutputDebugString(strData2.c_str());
#endif
      m_repeatCount = 0;
    }
    
    *m_repeatLine     = strData;
    m_repeatLogLevel  = loglevel;

    unsigned int length = 0;
    while ( length != strData.length() )
    {
      length = strData.length();
      strData.TrimRight(" ");
      strData.TrimRight('\n');
      strData.TrimRight("\r");
    }

    if (!length)
      return;

#if (defined(_DEBUG) || defined(PROFILE))
    OutputDebugString(strData.c_str());
    OutputDebugString("\n");
#endif

    /* fixup newline alignment, number of spaces should equal prefix length */
    strData.Replace("\n", LINE_ENDING"                                            ");
    strData += LINE_ENDING;

    fwrite(strPrefix.c_str(),strPrefix.size(),1,m_file);
    fwrite(strData.c_str(),strData.size(),1,m_file);
    fflush(m_file);
  }
#ifndef _LINUX
#if defined(_DEBUG) || defined(PROFILE)
  else
  {
    // In debug mode dump everything to devstudio regardless of level
    CSingleLock waitLock(critSec);
    CStdString strData;
    strData.reserve(16384);

    va_list va;
    va_start(va, format);
    strData.FormatV(format, va);
    va_end(va);

    OutputDebugString(strData.c_str());
    if( strData.Right(1) != "\n" )
      OutputDebugString("\n");

  }
#endif
#endif
}

bool CLog::Init(const char* path)
{
  CSingleLock waitLock(critSec);
  if (!m_file)
  {
    // g_settings.m_logFolder is initialized in the CSettings constructor
    // and changed in CApplication::Create()
#ifdef _WIN32
    CStdStringW pathW;
    g_charsetConverter.utf8ToW(path, pathW, false);
    CStdStringW strLogFile, strLogFileOld;

    strLogFile.Format(L"%sxbmc.log", pathW);
    strLogFileOld.Format(L"%sxbmc.old.log", pathW);

    struct __stat64 info;
    if (_wstat64(strLogFileOld.c_str(),&info) == 0 &&
        !::DeleteFileW(strLogFileOld.c_str()))
      return false;
    if (_wstat64(strLogFile.c_str(),&info) == 0 &&
        !::MoveFileW(strLogFile.c_str(),strLogFileOld.c_str()))
      return false;

    m_file = _wfsopen(strLogFile.c_str(),L"wb", _SH_DENYWR);
#else
    CStdString strLogFile, strLogFileOld;

    strLogFile.Format("%sxbmc.log", path);
    strLogFileOld.Format("%sxbmc.old.log", path);

    struct stat64 info;
    if (stat64(strLogFileOld.c_str(),&info) == 0 &&
        remove(strLogFileOld.c_str()) != 0)
      return false;
    if (stat64(strLogFile.c_str(),&info) == 0 &&
        rename(strLogFile.c_str(),strLogFileOld.c_str()) != 0)
      return false;

    m_file = fopen(strLogFile.c_str(),"wb");
#endif
  }

  if (m_file)
  {
    unsigned char BOM[3] = {0xEF, 0xBB, 0xBF};
    fwrite(BOM, sizeof(BOM), 1, m_file);
  }

  if (!m_repeatLine)
    m_repeatLine = new CStdString;

  return m_file != NULL;
}

void CLog::MemDump(char *pData, int length)
{
  Log(LOGDEBUG, "MEM_DUMP: Dumping from %p", pData);
  for (int i = 0; i < length; i+=16)
  {
    CStdString strLine;
    strLine.Format("MEM_DUMP: %04x ", i);
    char *alpha = pData;
    for (int k=0; k < 4 && i + 4*k < length; k++)
    {
      for (int j=0; j < 4 && i + 4*k + j < length; j++)
      {
        CStdString strFormat;
        strFormat.Format(" %02x", *pData++);
        strLine += strFormat;
      }
      strLine += " ";
    }
    // pad with spaces
    while (strLine.size() < 13*4 + 16)
      strLine += " ";
    for (int j=0; j < 16 && i + j < length; j++)
    {
      if (*alpha > 31)
        strLine += *alpha;
      else
        strLine += '.';
      alpha++;
    }
    Log(LOGDEBUG, "%s", strLine.c_str());
  }
}

