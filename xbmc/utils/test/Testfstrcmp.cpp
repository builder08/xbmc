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

#ifndef TEST_UTILS_FSTRCMP_H_INCLUDED
#define TEST_UTILS_FSTRCMP_H_INCLUDED
#include "utils/fstrcmp.h"
#endif

#ifndef TEST_UTILS_STDSTRING_H_INCLUDED
#define TEST_UTILS_STDSTRING_H_INCLUDED
#include "utils/StdString.h"
#endif

#ifndef TEST_UTILS_STRINGUTILS_H_INCLUDED
#define TEST_UTILS_STRINGUTILS_H_INCLUDED
#include "utils/StringUtils.h"
#endif


#ifndef TEST_GTEST_GTEST_H_INCLUDED
#define TEST_GTEST_GTEST_H_INCLUDED
#include "gtest/gtest.h"
#endif


TEST(Testfstrcmp, General)
{
  CStdString refstr, varstr, refresult, varresult;
  refstr = "Testfstrcmp test string";
  varstr = refstr;

  /* NOTE: Third parameter is not used at all in fstrcmp. */
  refresult = "1.000000";
  varresult = StringUtils::Format("%.6f", fstrcmp(refstr.c_str(), varstr.c_str(), 0.0));
  EXPECT_STREQ(refresult.c_str(), varresult.c_str());

  varstr = "Testfstrcmp_test_string";
  refresult = "0.913043";
  varresult = StringUtils::Format("%.6f", fstrcmp(refstr.c_str(), varstr.c_str(), 0.0));
  EXPECT_STREQ(refresult.c_str(), varresult.c_str());

  varstr = "";
  refresult = "0.000000";
  varresult = StringUtils::Format("%.6f", fstrcmp(refstr.c_str(), varstr.c_str(), 0.0));
  EXPECT_STREQ(refresult.c_str(), varresult.c_str());
}
