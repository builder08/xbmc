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
struct real_pcre_jit_stack; // forward declaration for PCRE without JIT
typedef struct real_pcre_jit_stack pcre_jit_stack;
#ifdef TARGET_WINDOWS
#define PCRE_STATIC
#include "lib/win32/pcre/pcre.h"
#else
#include <pcre.h>
#endif
}

class CRegExp
{
public:
  static const int m_MaxNumOfBackrefrences = 20;
  /**
   * @param caseless (optional) Matching will be case insensitive if set to true
   *                            or case sensitive if set to false
   * @param utf8 (optional) If set to true all string will be processed as UTF-8 strings 
   */
  CRegExp(bool caseless = false, bool utf8 = false);
  /**
   * Create new CRegExp object and compile regexp expression in one step
   * @warning Use only with hardcoded regexp when you're sure that regexp is compiled without errors
   * @param caseless    Matching will be case insensitive if set to true 
   *                    or case sensitive if set to false
   * @param utf8        If set to true all string will be processed as UTF-8 strings
   * @param re          The regular expression
   * @param study (optional) If set to true expression will be additionally studied, useful if
   *                         expression will be used several times
   * @param jitCompile (optional) Heavyweight expression optimization, longer compilation but 
   *                              much faster matching. Ignored if study==false or PCRE lack
   *                              support for JIT compiling
   */
  CRegExp(bool caseless, bool utf8, const char *re, bool study = false, bool jitCompile = true);
  CRegExp(const CRegExp& re);
  ~CRegExp();

  /**
   * Compile (prepare) regular expression
   * @param re          The regular expression
   * @param study (optional) If set to true expression will be additionally studied, useful if
   *                         expression will be used several times
   * @param jitCompile (optional) Heavyweight expression optimization, longer compilation but 
   *                              much faster matching. Ignored if study==false or PCRE lack
   *                              support for JIT compiling
   * @return true on success, false on any error
   */
  bool RegComp(const char *re, bool study = false, bool jitCompile = true);

  /**
   * Compile (prepare) regular expression
   * @param re          The regular expression
   * @param study (optional) If set to true expression will be additionally studied, useful if
   *                         expression will be used several times
   * @param jitCompile (optional) Heavyweight expression optimization, longer compilation but
   *                              much faster matching. Ignored if study==false or PCRE lack
   *                              support for JIT compiling
   * @return true on success, false on any error
   */
  bool RegComp(const std::string& re, bool study = false, bool jitCompile = false)
  { return RegComp(re.c_str(), study, jitCompile); }
  /**
   * Find first match of regular expression in given string
   * @param str         The string to match against regular expression
   * @param startoffset (optional) The string offset to start matching
   * @param maxNumberOfCharsToTest (optional) The maximum number of characters to test (match) in 
   *                                          string. If set to -1 string checked up to the end.
   * @return staring position of match in string, negative value in case of error or no match
   */
  int RegFind(const char* str, unsigned int startoffset = 0, int maxNumberOfCharsToTest = -1);
  /**
   * Find first match of regular expression in given string
   * @param str         The string to match against regular expression
   * @param startoffset (optional) The string offset to start matching
   * @param maxNumberOfCharsToTest (optional) The maximum number of characters to test (match) in
   *                                          string. If set to -1 string checked up to the end.
   * @return staring position of match in string, negative value in case of error or no match
   */
  int RegFind(const std::string& str, unsigned int startoffset = 0, int maxNumberOfCharsToTest = -1)
  { return PrivateRegFind(str.length(), str.c_str(), startoffset, maxNumberOfCharsToTest); }
  std::string GetReplaceString(const std::string& sReplaceExp) const;
  int GetFindLen() const
  {
    if (!m_re || !m_bMatched)
      return 0;

    return (m_iOvector[1] - m_iOvector[0]);
  };
  int GetSubCount() const { return m_iMatchCount - 1; } // PCRE returns the number of sub-patterns + 1
  int GetSubStart(int iSub) const;
  int GetSubStart(const std::string& subName) const;
  int GetSubLength(int iSub) const;
  int GetSubLength(const std::string& subName) const;
  int GetCaptureTotal() const;
  std::string GetMatch(int iSub = 0) const;
  std::string GetMatch(const std::string& subName) const;
  const std::string& GetPattern() const { return m_pattern; }
  bool GetNamedSubPattern(const char* strName, std::string& strMatch) const;
  int GetNamedSubPatternNumber(const char* strName) const;
  void DumpOvector(int iLog);
  const CRegExp& operator= (const CRegExp& re);
  static bool IsUtf8Supported(void);
  static bool AreUnicodePropertiesSupported(void);
  static bool IsJitSupported(void);

private:
  int PrivateRegFind(size_t bufferLen, const char *str, unsigned int startoffset = 0, int maxNumberOfCharsToTest = -1);
  void InitValues(bool caseless = false, bool utf8 = false);

  void Cleanup();
  inline bool IsValidSubNumber(int iSub) const;

  PCRE::pcre* m_re;
  PCRE::pcre_extra* m_sd;
  static const int OVECCOUNT=(m_MaxNumOfBackrefrences + 1) * 3;
  unsigned int m_offset;
  int         m_iOvector[OVECCOUNT];
  int         m_iMatchCount;
  int         m_iOptions;
  bool        m_jitCompiled;
  bool        m_bMatched;
  PCRE::pcre_jit_stack* m_jitStack;
  std::string m_subject;
  std::string m_pattern;
  static int  m_Utf8Supported;     
  static int  m_UcpSupported;
  static int  m_JitSupported;
};

typedef std::vector<CRegExp> VECCREGEXP;

#endif

