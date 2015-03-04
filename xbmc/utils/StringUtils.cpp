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
//-----------------------------------------------------------------------
//
//  File:      StringUtils.cpp
//
//  Purpose:   ATL split string utility
//  Author:    Paul J. Weiss
//
//  Modified to use J O'Leary's std::string class by kraqh3d
//
//------------------------------------------------------------------------


#include "StringUtils.h"
#include "utils/fstrcmp.h"
#include "Util.h"
#include "LangInfo.h"
#include <locale>
#include <functional>

#include <assert.h>
#include <math.h>
#include <sstream>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <memory.h>
#include <algorithm>
#include "utils/RegExp.h" // don't move or std functions end up in PCRE namespace

#define FORMAT_BLOCK_SIZE 512 // # of bytes for initial allocation for printf

using namespace std;

const char* ADDON_GUID_RE = "^(\\{){0,1}[0-9a-fA-F]{8}\\-[0-9a-fA-F]{4}\\-[0-9a-fA-F]{4}\\-[0-9a-fA-F]{4}\\-[0-9a-fA-F]{12}(\\}){0,1}$";

/* empty string for use in returns by ref */
const std::string StringUtils::Empty = "";
std::string StringUtils::m_lastUUID = "";

string StringUtils::Format(const char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  string str = FormatV(fmt, args);
  va_end(args);

  return str;
}

string StringUtils::FormatV(const char *fmt, va_list args)
{
  if (!fmt || !fmt[0])
    return "";

  int size = FORMAT_BLOCK_SIZE;
  va_list argCopy;

  while (1) 
  {
    char *cstr = reinterpret_cast<char*>(malloc(sizeof(char) * size));
    if (!cstr)
      return "";

    va_copy(argCopy, args);
    int nActual = vsnprintf(cstr, size, fmt, argCopy);
    va_end(argCopy);

    if (nActual > -1 && nActual < size) // We got a valid result
    {
      std::string str(cstr, nActual);
      free(cstr);
      return str;
    }
    free(cstr);
#ifndef TARGET_WINDOWS
    if (nActual > -1)                   // Exactly what we will need (glibc 2.1)
      size = nActual + 1;
    else                                // Let's try to double the size (glibc 2.0)
      size *= 2;
#else  // TARGET_WINDOWS
    va_copy(argCopy, args);
    size = _vscprintf(fmt, argCopy);
    va_end(argCopy);
    if (size < 0)
      return "";
    else
      size++; // increment for null-termination
#endif // TARGET_WINDOWS
  }

  return ""; // unreachable
}

wstring StringUtils::Format(const wchar_t *fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  wstring str = FormatV(fmt, args);
  va_end(args);
  
  return str;
}

wstring StringUtils::FormatV(const wchar_t *fmt, va_list args)
{
  if (!fmt || !fmt[0])
    return L"";

  int size = FORMAT_BLOCK_SIZE;
  va_list argCopy;
  
  while (1)
  {
    wchar_t *cstr = reinterpret_cast<wchar_t*>(malloc(sizeof(wchar_t) * size));
    if (!cstr)
      return L"";

    va_copy(argCopy, args);
    int nActual = vswprintf(cstr, size, fmt, argCopy);
    va_end(argCopy);
    
    if (nActual > -1 && nActual < size) // We got a valid result
    {
      std::wstring str(cstr, nActual);
      free(cstr);
      return str;
    }
    free(cstr);

#ifndef TARGET_WINDOWS
    if (nActual > -1)                   // Exactly what we will need (glibc 2.1)
      size = nActual + 1;
    else                                // Let's try to double the size (glibc 2.0)
      size *= 2;
#else  // TARGET_WINDOWS
    va_copy(argCopy, args);
    size = _vscwprintf(fmt, argCopy);
    va_end(argCopy);
    if (size < 0)
      return L"";
    else
      size++; // increment for null-termination
#endif // TARGET_WINDOWS
  }
  
  return L"";
}

int compareWchar (const void* a, const void* b)
{
  if (*(wchar_t*)a <  *(wchar_t*)b)
    return -1;
  else if (*(wchar_t*)a >  *(wchar_t*)b)
    return 1;
  return 0;
}

void StringUtils::ToUpper(string &str)
{
  for (auto & c: str) c = std::toupper(c, g_langInfo.GetLocale());
}

void StringUtils::ToUpper(wstring &str)
{
  for (auto & c: str) c = std::toupper(c, g_langInfo.GetLocale());
}

void StringUtils::ToLower(string &str)
{
  for (auto & c: str) c = std::tolower(c, g_langInfo.GetLocale());
}

void StringUtils::ToLower(wstring &str)
{
  for (auto & c: str) c = std::tolower(c, g_langInfo.GetLocale());
}

bool StringUtils::EqualsNoCase(const std::string &str1, const std::string &str2)
{
  return EqualsNoCase(str1.c_str(), str2.c_str());
}

bool StringUtils::EqualsNoCase(const std::string &str1, const char *s2)
{
  return EqualsNoCase(str1.c_str(), s2);
}

bool StringUtils::EqualsNoCase(const char *s1, const char *s2)
{
  char c2; // we need only one char outside the loop
  do
  {
    const char c1 = *s1++; // const local variable should help compiler to optimize
    c2 = *s2++;
    if (c1 != c2 && ::tolower(c1) != ::tolower(c2)) // This includes the possibility that one of the characters is the null-terminator, which implies a string mismatch.
      return false;
  } while (c2 != '\0'); // At this point, we know c1 == c2, so there's no need to test them both.
  return true;
}

int StringUtils::CompareNoCase(const std::string &str1, const std::string &str2)
{
  return CompareNoCase(str1.c_str(), str2.c_str());
}

int StringUtils::CompareNoCase(const char *s1, const char *s2)
{
  char c2; // we need only one char outside the loop
  do
  {
    const char c1 = *s1++; // const local variable should help compiler to optimize
    c2 = *s2++;
    if (c1 != c2 && ::tolower(c1) != ::tolower(c2)) // This includes the possibility that one of the characters is the null-terminator, which implies a string mismatch.
      return ::tolower(c1) - ::tolower(c2);
  } while (c2 != '\0'); // At this point, we know c1 == c2, so there's no need to test them both.
  return 0;
}

string StringUtils::Left(const string &str, size_t count)
{
  count = max((size_t)0, min(count, str.size()));
  return str.substr(0, count);
}

string StringUtils::Mid(const string &str, size_t first, size_t count /* = string::npos */)
{
  if (first + count > str.size())
    count = str.size() - first;
  
  if (first > str.size())
    return string();
  
  assert(first + count <= str.size());
  
  return str.substr(first, count);
}

string StringUtils::Right(const string &str, size_t count)
{
  count = max((size_t)0, min(count, str.size()));
  return str.substr(str.size() - count);
}

std::string& StringUtils::Trim(std::string &str)
{
  TrimLeft(str);
  return TrimRight(str);
}

std::string& StringUtils::Trim(std::string &str, const char* const chars)
{
  TrimLeft(str, chars);
  return TrimRight(str, chars);
}

// hack to check only first byte of UTF-8 character
// without this hack "TrimX" functions failed on Win32 and OS X with UTF-8 strings
static int isspace_c(char c)
{
  return (c & 0x80) == 0 && ::isspace(c);
}

std::string& StringUtils::TrimLeft(std::string &str)
{
  str.erase(str.begin(), ::find_if(str.begin(), str.end(), ::not1(::ptr_fun(isspace_c))));
  return str;
}

std::string& StringUtils::TrimLeft(std::string &str, const char* const chars)
{
  size_t nidx = str.find_first_not_of(chars);
  str.erase(0, nidx);
  return str;
}

std::string& StringUtils::TrimRight(std::string &str)
{
  str.erase(::find_if(str.rbegin(), str.rend(), ::not1(::ptr_fun(isspace_c))).base(), str.end());
  return str;
}

std::string& StringUtils::TrimRight(std::string &str, const char* const chars)
{
  size_t nidx = str.find_last_not_of(chars);
  str.erase(str.npos == nidx ? 0 : ++nidx);
  return str;
}

std::string& StringUtils::RemoveDuplicatedSpacesAndTabs(std::string& str)
{
  std::string::iterator it = str.begin();
  bool onSpace = false;
  while(it != str.end())
  {
    if (*it == '\t')
      *it = ' ';

    if (*it == ' ')
    {
      if (onSpace)
      {
        it = str.erase(it);
        continue;
      }
      else
        onSpace = true;
    }
    else
      onSpace = false;

    ++it;
  }
  return str;
}

int StringUtils::Replace(string &str, char oldChar, char newChar)
{
  int replacedChars = 0;
  for (string::iterator it = str.begin(); it != str.end(); ++it)
  {
    if (*it == oldChar)
    {
      *it = newChar;
      replacedChars++;
    }
  }
  
  return replacedChars;
}

int StringUtils::Replace(std::string &str, const std::string &oldStr, const std::string &newStr)
{
  if (oldStr.empty())
    return 0;

  int replacedChars = 0;
  size_t index = 0;
  
  while (index < str.size() && (index = str.find(oldStr, index)) != string::npos)
  {
    str.replace(index, oldStr.size(), newStr);
    index += newStr.size();
    replacedChars++;
  }
  
  return replacedChars;
}

int StringUtils::Replace(std::wstring &str, const std::wstring &oldStr, const std::wstring &newStr)
{
  if (oldStr.empty())
    return 0;

  int replacedChars = 0;
  size_t index = 0;

  while (index < str.size() && (index = str.find(oldStr, index)) != string::npos)
  {
    str.replace(index, oldStr.size(), newStr);
    index += newStr.size();
    replacedChars++;
  }

  return replacedChars;
}

bool StringUtils::StartsWith(const std::string &str1, const std::string &str2)
{
  return str1.compare(0, str2.size(), str2) == 0;
}

bool StringUtils::StartsWith(const std::string &str1, const char *s2)
{
  return StartsWith(str1.c_str(), s2);
}

bool StringUtils::StartsWith(const char *s1, const char *s2)
{
  while (*s2 != '\0')
  {
    if (*s1 != *s2)
      return false;
    s1++;
    s2++;
  }
  return true;
}

bool StringUtils::StartsWithNoCase(const std::string &str1, const std::string &str2)
{
  return StartsWithNoCase(str1.c_str(), str2.c_str());
}

bool StringUtils::StartsWithNoCase(const std::string &str1, const char *s2)
{
  return StartsWithNoCase(str1.c_str(), s2);
}

bool StringUtils::StartsWithNoCase(const char *s1, const char *s2)
{
  while (*s2 != '\0')
  {
    if (::tolower(*s1) != ::tolower(*s2))
      return false;
    s1++;
    s2++;
  }
  return true;
}

bool StringUtils::EndsWith(const std::string &str1, const std::string &str2)
{
  if (str1.size() < str2.size())
    return false;
  return str1.compare(str1.size() - str2.size(), str2.size(), str2) == 0;
}

bool StringUtils::EndsWith(const std::string &str1, const char *s2)
{
  size_t len2 = strlen(s2);
  if (str1.size() < len2)
    return false;
  return str1.compare(str1.size() - len2, len2, s2) == 0;
}

bool StringUtils::EndsWithNoCase(const std::string &str1, const std::string &str2)
{
  if (str1.size() < str2.size())
    return false;
  const char *s1 = str1.c_str() + str1.size() - str2.size();
  const char *s2 = str2.c_str();
  while (*s2 != '\0')
  {
    if (::tolower(*s1) != ::tolower(*s2))
      return false;
    s1++;
    s2++;
  }
  return true;
}

bool StringUtils::EndsWithNoCase(const std::string &str1, const char *s2)
{
  size_t len2 = strlen(s2);
  if (str1.size() < len2)
    return false;
  const char *s1 = str1.c_str() + str1.size() - len2;
  while (*s2 != '\0')
  {
    if (::tolower(*s1) != ::tolower(*s2))
      return false;
    s1++;
    s2++;
  }
  return true;
}

std::string StringUtils::Join(const vector<string> &strings, const std::string& delimiter)
{
  std::string result;
  for(vector<string>::const_iterator it = strings.begin(); it != strings.end(); ++it )
    result += (*it) + delimiter;
  
  if (!result.empty())
    result.erase(result.size() - delimiter.size());
  return result;
}

vector<string> StringUtils::Split(const std::string& input, const std::string& delimiter, unsigned int iMaxStrings /* = 0 */)
{
  std::vector<std::string> results;
  if (input.empty())
    return results;
  if (delimiter.empty())
  {
    results.push_back(input);
    return results;
  }

  const size_t delimLen = delimiter.length();
  size_t nextDelim;
  size_t textPos = 0;
  do
  {
    if (--iMaxStrings == 0)
    {
      results.push_back(input.substr(textPos));
      break;
    }
    nextDelim = input.find(delimiter, textPos);
    results.push_back(input.substr(textPos, nextDelim - textPos));
    textPos = nextDelim + delimLen;
  } while (nextDelim != std::string::npos);

  return results;
}

std::vector<std::string> StringUtils::Split(const std::string& input, const char delimiter, size_t iMaxStrings /*= 0*/)
{
  std::vector<std::string> results;
  if (input.empty())
    return results;

  size_t nextDelim;
  size_t textPos = 0;
  do
  {
    if (--iMaxStrings == 0)
    {
      results.push_back(input.substr(textPos));
      break;
    }
    nextDelim = input.find(delimiter, textPos);
    results.push_back(input.substr(textPos, nextDelim - textPos));
    textPos = nextDelim + 1;
  } while (nextDelim != std::string::npos);

  return results;
}


// returns the number of occurrences of strFind in strInput.
int StringUtils::FindNumber(const std::string& strInput, const std::string &strFind)
{
  size_t pos = strInput.find(strFind, 0);
  int numfound = 0;
  while (pos != std::string::npos)
  {
    numfound++;
    pos = strInput.find(strFind, pos + 1);
  }
  return numfound;
}

// Compares separately the numeric and alphabetic parts of a string.
// returns negative if left < right, positive if left > right
// and 0 if they are identical (essentially calculates left - right)
int64_t StringUtils::AlphaNumericCompare(const wchar_t *left, const wchar_t *right)
{
  wchar_t *l = (wchar_t *)left;
  wchar_t *r = (wchar_t *)right;
  wchar_t *ld, *rd;
  wchar_t lc, rc;
  int64_t lnum, rnum;
  const collate<wchar_t>& coll = use_facet< collate<wchar_t> >( g_langInfo.GetLocale() );
  int cmp_res = 0;
  while (*l != 0 && *r != 0)
  {
    // check if we have a numerical value
    if (*l >= L'0' && *l <= L'9' && *r >= L'0' && *r <= L'9')
    {
      ld = l;
      lnum = 0;
      while (*ld >= L'0' && *ld <= L'9' && ld < l + 15)
      { // compare only up to 15 digits
        lnum *= 10;
        lnum += *ld++ - '0';
      }
      rd = r;
      rnum = 0;
      while (*rd >= L'0' && *rd <= L'9' && rd < r + 15)
      { // compare only up to 15 digits
        rnum *= 10;
        rnum += *rd++ - L'0';
      }
      // do we have numbers?
      if (lnum != rnum)
      { // yes - and they're different!
        return lnum - rnum;
      }
      l = ld;
      r = rd;
      continue;
    }
    // do case less comparison
    lc = *l;
    if (lc >= L'A' && lc <= L'Z')
      lc += L'a'-L'A';
    rc = *r;
    if (rc >= L'A' && rc <= L'Z')
      rc += L'a'- L'A';

    // ok, do a normal comparison, taking current locale into account. Add special case stuff (eg '(' characters)) in here later
    if ((cmp_res = coll.compare(&lc, &lc + 1, &rc, &rc + 1)) != 0)
    {
      return cmp_res;
    }
    l++; r++;
  }
  if (*r)
  { // r is longer
    return -1;
  }
  else if (*l)
  { // l is longer
    return 1;
  }
  return 0; // files are the same
}

int StringUtils::DateStringToYYYYMMDD(const std::string &dateString)
{
  vector<string> days = StringUtils::Split(dateString, '-');
  if (days.size() == 1)
    return atoi(days[0].c_str());
  else if (days.size() == 2)
    return atoi(days[0].c_str())*100+atoi(days[1].c_str());
  else if (days.size() == 3)
    return atoi(days[0].c_str())*10000+atoi(days[1].c_str())*100+atoi(days[2].c_str());
  else
    return -1;
}

long StringUtils::TimeStringToSeconds(const std::string &timeString)
{
  std::string strCopy(timeString);
  StringUtils::Trim(strCopy);
  if(StringUtils::EndsWithNoCase(strCopy, " min"))
  {
    // this is imdb format of "XXX min"
    return 60 * atoi(strCopy.c_str());
  }
  else
  {
    vector<string> secs = StringUtils::Split(strCopy, ':');
    int timeInSecs = 0;
    for (unsigned int i = 0; i < 3 && i < secs.size(); i++)
    {
      timeInSecs *= 60;
      timeInSecs += atoi(secs[i].c_str());
    }
    return timeInSecs;
  }
}

std::string StringUtils::SecondsToTimeString(long lSeconds, TIME_FORMAT format)
{
  int hh = lSeconds / 3600;
  lSeconds = lSeconds % 3600;
  int mm = lSeconds / 60;
  int ss = lSeconds % 60;

  if (format == TIME_FORMAT_GUESS)
    format = (hh >= 1) ? TIME_FORMAT_HH_MM_SS : TIME_FORMAT_MM_SS;
  std::string strHMS;
  if (format & TIME_FORMAT_HH)
    strHMS += StringUtils::Format("%02.2i", hh);
  else if (format & TIME_FORMAT_H)
    strHMS += StringUtils::Format("%i", hh);
  if (format & TIME_FORMAT_MM)
    strHMS += StringUtils::Format(strHMS.empty() ? "%02.2i" : ":%02.2i", mm);
  if (format & TIME_FORMAT_SS)
    strHMS += StringUtils::Format(strHMS.empty() ? "%02.2i" : ":%02.2i", ss);
  return strHMS;
}

bool StringUtils::IsNaturalNumber(const std::string& str)
{
  size_t i = 0, n = 0;
  // allow whitespace,digits,whitespace
  while (i < str.size() && isspace((unsigned char) str[i]))
    i++;
  while (i < str.size() && isdigit((unsigned char) str[i]))
  {
    i++; n++;
  }
  while (i < str.size() && isspace((unsigned char) str[i]))
    i++;
  return i == str.size() && n > 0;
}

bool StringUtils::IsInteger(const std::string& str)
{
  size_t i = 0, n = 0;
  // allow whitespace,-,digits,whitespace
  while (i < str.size() && isspace((unsigned char) str[i]))
    i++;
  if (i < str.size() && str[i] == '-')
    i++;
  while (i < str.size() && isdigit((unsigned char) str[i]))
  {
    i++; n++;
  }
  while (i < str.size() && isspace((unsigned char) str[i]))
    i++;
  return i == str.size() && n > 0;
}

int StringUtils::asciidigitvalue(char chr)
{
  if (!isasciidigit(chr))
    return -1;

  return chr - '0';
}

int StringUtils::asciixdigitvalue(char chr)
{
  int v = asciidigitvalue(chr);
  if (v >= 0)
    return v;
  if (chr >= 'a' && chr <= 'f')
    return chr - 'a' + 10;
  if (chr >= 'A' && chr <= 'F')
    return chr - 'A' + 10;

  return -1;
}


void StringUtils::RemoveCRLF(std::string& strLine)
{
  StringUtils::TrimRight(strLine, "\n\r");
}

std::string StringUtils::SizeToString(int64_t size)
{
  std::string strLabel;
  const char prefixes[] = {' ','k', 'M', 'G', 'T', 'P', 'E', 'Z', 'Y'};
  unsigned int i = 0;
  double s = (double)size;
  while (i < ARRAY_SIZE(prefixes) && s >= 1000.0)
  {
    s /= 1024.0;
    i++;
  }

  if (!i)
    strLabel = StringUtils::Format("%.0lf %cB ", s, prefixes[i]);
  else if (s >= 100.0)
    strLabel = StringUtils::Format("%.1lf %cB", s, prefixes[i]);
  else
    strLabel = StringUtils::Format("%.2lf %cB", s, prefixes[i]);

  return strLabel;
}

// return -1 if not, else return the utf8 char length.
int IsUTF8Letter(const unsigned char *str)
{
  // reference:
  // unicode -> utf8 table: http://www.utf8-chartable.de/
  // latin characters in unicode: http://en.wikipedia.org/wiki/Latin_characters_in_Unicode
  unsigned char ch = str[0];
  if (!ch)
    return -1;
  if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z'))
    return 1;
  if (!(ch & 0x80))
    return -1;
  unsigned char ch2 = str[1];
  if (!ch2)
    return -1;
  // check latin 1 letter table: http://en.wikipedia.org/wiki/C1_Controls_and_Latin-1_Supplement
  if (ch == 0xC3 && ch2 >= 0x80 && ch2 <= 0xBF && ch2 != 0x97 && ch2 != 0xB7)
    return 2;
  // check latin extended A table: http://en.wikipedia.org/wiki/Latin_Extended-A
  if (ch >= 0xC4 && ch <= 0xC7 && ch2 >= 0x80 && ch2 <= 0xBF)
    return 2;
  // check latin extended B table: http://en.wikipedia.org/wiki/Latin_Extended-B
  // and International Phonetic Alphabet: http://en.wikipedia.org/wiki/IPA_Extensions_(Unicode_block)
  if (((ch == 0xC8 || ch == 0xC9) && ch2 >= 0x80 && ch2 <= 0xBF)
      || (ch == 0xCA && ch2 >= 0x80 && ch2 <= 0xAF))
    return 2;
  return -1;
}

size_t StringUtils::FindWords(const char *str, const char *wordLowerCase)
{
  // NOTE: This assumes word is lowercase!
  unsigned char *s = (unsigned char *)str;
  do
  {
    // start with a compare
    unsigned char *c = s;
    unsigned char *w = (unsigned char *)wordLowerCase;
    bool same = true;
    while (same && *c && *w)
    {
      unsigned char lc = *c++;
      if (lc >= 'A' && lc <= 'Z')
        lc += 'a'-'A';

      if (lc != *w++) // different
        same = false;
    }
    if (same && *w == 0)  // only the same if word has been exhausted
      return (const char *)s - str;

    // otherwise, skip current word (composed by latin letters) or number
    int l;
    if (*s >= '0' && *s <= '9')
    {
      ++s;
      while (*s >= '0' && *s <= '9') ++s;
    }
    else if ((l = IsUTF8Letter(s)) > 0)
    {
      s += l;
      while ((l = IsUTF8Letter(s)) > 0) s += l;
    }
    else
      ++s;
    while (*s && *s == ' ') s++;

    // and repeat until we're done
  } while (*s);

  return std::string::npos;
}

// assumes it is called from after the first open bracket is found
int StringUtils::FindEndBracket(const std::string &str, char opener, char closer, int startPos)
{
  int blocks = 1;
  for (unsigned int i = startPos; i < str.size(); i++)
  {
    if (str[i] == opener)
      blocks++;
    else if (str[i] == closer)
    {
      blocks--;
      if (!blocks)
        return i;
    }
  }

  return (int)std::string::npos;
}

void StringUtils::WordToDigits(std::string &word)
{
  static const char word_to_letter[] = "22233344455566677778889999";
  StringUtils::ToLower(word);
  for (unsigned int i = 0; i < word.size(); ++i)
  { // NB: This assumes ascii, which probably needs extending at some  point.
    char letter = word[i];
    if ((letter >= 'a' && letter <= 'z')) // assume contiguous letter range
    {
      word[i] = word_to_letter[letter-'a'];
    }
    else if (letter < '0' || letter > '9') // We want to keep 0-9!
    {
      word[i] = ' ';  // replace everything else with a space
    }
  }
}

std::string StringUtils::CreateUUID()
{
  /* This function generate a DCE 1.1, ISO/IEC 11578:1996 and IETF RFC-4122
  * Version 4 conform local unique UUID based upon random number generation.
  */
  char UuidStrTmp[40];
  char *pUuidStr = UuidStrTmp;
  int i;

  static bool m_uuidInitialized = false;
  if (!m_uuidInitialized)
  {
    /* use current time as the seed for rand()*/
    srand(time(NULL));
    m_uuidInitialized = true;
  }

  /*Data1 - 8 characters.*/
  for(i = 0; i < 8; i++, pUuidStr++)
    ((*pUuidStr = (rand() % 16)) < 10) ? *pUuidStr += 48 : *pUuidStr += 55;

  /*Data2 - 4 characters.*/
  *pUuidStr++ = '-';
  for(i = 0; i < 4; i++, pUuidStr++)
    ((*pUuidStr = (rand() % 16)) < 10) ? *pUuidStr += 48 : *pUuidStr += 55;

  /*Data3 - 4 characters.*/
  *pUuidStr++ = '-';
  for(i = 0; i < 4; i++, pUuidStr++)
    ((*pUuidStr = (rand() % 16)) < 10) ? *pUuidStr += 48 : *pUuidStr += 55;

  /*Data4 - 4 characters.*/
  *pUuidStr++ = '-';
  for(i = 0; i < 4; i++, pUuidStr++)
    ((*pUuidStr = (rand() % 16)) < 10) ? *pUuidStr += 48 : *pUuidStr += 55;

  /*Data5 - 12 characters.*/
  *pUuidStr++ = '-';
  for(i = 0; i < 12; i++, pUuidStr++)
    ((*pUuidStr = (rand() % 16)) < 10) ? *pUuidStr += 48 : *pUuidStr += 55;

  *pUuidStr = '\0';

  m_lastUUID = UuidStrTmp;
  return UuidStrTmp;
}

bool StringUtils::ValidateUUID(const std::string &uuid)
{
  CRegExp guidRE;
  guidRE.RegComp(ADDON_GUID_RE);
  return (guidRE.RegFind(uuid.c_str()) == 0);
}

double StringUtils::CompareFuzzy(const std::string &left, const std::string &right)
{
  return (0.5 + fstrcmp(left.c_str(), right.c_str(), 0.0) * (left.length() + right.length())) / 2.0;
}

int StringUtils::FindBestMatch(const std::string &str, const vector<string> &strings, double &matchscore)
{
  int best = -1;
  matchscore = 0;

  int i = 0;
  for (vector<string>::const_iterator it = strings.begin(); it != strings.end(); ++it, i++)
  {
    int maxlength = max(str.length(), it->length());
    double score = StringUtils::CompareFuzzy(str, *it) / maxlength;
    if (score > matchscore)
    {
      matchscore = score;
      best = i;
    }
  }
  return best;
}

bool StringUtils::ContainsKeyword(const std::string &str, const vector<string> &keywords)
{
  for (vector<string>::const_iterator it = keywords.begin(); it != keywords.end(); ++it)
  {
    if (str.find(*it) != str.npos)
      return true;
  }
  return false;
}

size_t StringUtils::utf8_strlen(const char *s)
{
  size_t length = 0;
  while (*s)
  {
    if ((*s++ & 0xC0) != 0x80)
      length++;
  }
  return length;
}

std::string StringUtils::Paramify(const std::string &param)
{
  std::string result = param;
  // escape backspaces
  StringUtils::Replace(result, "\\", "\\\\");
  // escape double quotes
  StringUtils::Replace(result, "\"", "\\\"");

  // add double quotes around the whole string
  return "\"" + result + "\"";
}

std::vector<std::string> StringUtils::Tokenize(const std::string &input, const std::string &delimiters)
{
  std::vector<std::string> tokens;
  Tokenize(input, tokens, delimiters);
  return tokens;
}

void StringUtils::Tokenize(const std::string& input, std::vector<std::string>& tokens, const std::string& delimiters)
{
  tokens.clear();
  // Skip delimiters at beginning.
  std::string::size_type dataPos = input.find_first_not_of(delimiters);
  while (dataPos != std::string::npos)
  {
    // Find next delimiter
    const std::string::size_type nextDelimPos = input.find_first_of(delimiters, dataPos);
    // Found a token, add it to the vector.
    tokens.push_back(input.substr(dataPos, nextDelimPos - dataPos));
    // Skip delimiters.  Note the "not_of"
    dataPos = input.find_first_not_of(delimiters, nextDelimPos);
  }
}

std::vector<std::string> StringUtils::Tokenize(const std::string &input, const char delimiter)
{
  std::vector<std::string> tokens;
  Tokenize(input, tokens, delimiter);
  return tokens;
}

void StringUtils::Tokenize(const std::string& input, std::vector<std::string>& tokens, const char delimiter)
{
  tokens.clear();
  // Skip delimiters at beginning.
  std::string::size_type dataPos = input.find_first_not_of(delimiter);
  while (dataPos != std::string::npos)
  {
    // Find next delimiter
    const std::string::size_type nextDelimPos = input.find(delimiter, dataPos);
    // Found a token, add it to the vector.
    tokens.push_back(input.substr(dataPos, nextDelimPos - dataPos));
    // Skip delimiters.  Note the "not_of"
    dataPos = input.find_first_not_of(delimiter, nextDelimPos);
  }
}
