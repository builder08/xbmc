#pragma once
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

#ifndef REGEXP_H
#define REGEXP_H

#include <string>
#include <vector>

namespace PCRE {
#ifdef TARGET_WINDOWS
#define PCRE_STATIC
#include "lib/win32/pcre/pcre.h"
#else
#include <pcre.h>
#endif
}

// maximum of 20 backreferences
// OVEVCOUNT must be a multiple of 3
const int OVECCOUNT=(20+1)*3;

class CRegExp
{
public:
  CRegExp(bool caseless = false, bool utf8 = false);
  CRegExp(const CRegExp& re);
  ~CRegExp();

  CRegExp* RegComp(const char *re);
  CRegExp* RegComp(const std::string& re) { return RegComp(re.c_str()); }
  int RegFind(const char *str, int startoffset = 0);
  int RegFind(const std::string& str, int startoffset = 0) { return RegFind(str.c_str(), startoffset); }
  std::string GetReplaceString( const char* sReplaceExp );
  int GetFindLen()
  {
    if (!m_re || !m_bMatched)
      return 0;

    return (m_iOvector[1] - m_iOvector[0]);
  };
  int GetSubCount() { return m_iMatchCount - 1; } // PCRE returns the number of sub-patterns + 1
  int GetSubStart(int iSub) { return m_iOvector[iSub*2]; } // normalized to match old engine
  int GetSubLength(int iSub) { return (m_iOvector[(iSub*2)+1] - m_iOvector[(iSub*2)]); } // correct spelling
  int GetCaptureTotal();
  std::string GetMatch(int iSub = 0);
  const std::string& GetPattern() { return m_pattern; }
  bool GetNamedSubPattern(const char* strName, std::string& strMatch);
  int GetNamedSubPatternNumber(const char* strName);
  void DumpOvector(int iLog);
  const CRegExp& operator= (const CRegExp& re);
  static bool IsUtf8Supported(void);
  static bool AreUnicodePropertiesSupported(void);

private:
  void Cleanup() { if (m_re) { PCRE::pcre_free(m_re); m_re = NULL; } }

private:
  PCRE::pcre* m_re;
  int         m_iOvector[OVECCOUNT];
  int         m_iMatchCount;
  int         m_iOptions;
  bool        m_bMatched;
  std::string m_subject;
  std::string m_pattern;
  static int  m_Utf8Supported;     
  static int  m_UcpSupported;
};

typedef std::vector<CRegExp> VECCREGEXP;

#endif

