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

#ifndef TEST_FILESYSTEM_NFSFILE_H_INCLUDED
#define TEST_FILESYSTEM_NFSFILE_H_INCLUDED
#include "filesystem/NFSFile.h"
#endif

#ifndef TEST_TEST_TESTUTILS_H_INCLUDED
#define TEST_TEST_TESTUTILS_H_INCLUDED
#include "test/TestUtils.h"
#endif


#include <errno.h>
#include <string>
#ifndef TEST_URL_H_INCLUDED
#define TEST_URL_H_INCLUDED
#include "URL.h"
#endif


#ifndef TEST_GTEST_GTEST_H_INCLUDED
#define TEST_GTEST_GTEST_H_INCLUDED
#include "gtest/gtest.h"
#endif


using ::testing::Test;
using ::testing::WithParamInterface;
using ::testing::ValuesIn;

struct SplitPath
{
  std::string url;
  CStdString exportPath;
  CStdString relativePath;
  bool expectedResultExport;
  bool expectedResultPath;
} g_TestData[] = { 
                   {"nfs://192.168.0.1:2049/srv/test/tvmedia/foo.txt", "/srv/test", "//tvmedia/foo.txt", true, true},
                   {"nfs://192.168.0.1/srv/test/tv/media/foo.txt", "/srv/test/tv", "//media/foo.txt", true, true},
                   {"nfs://192.168.0.1:2049/srv/test/tvmedia", "/srv/test", "//tvmedia", true, true},
                   {"nfs://192.168.0.1:2049/srv/test/tvmedia/", "/srv/test", "//tvmedia/", true, true},
                   {"nfs://192.168.0.1:2049/srv/test/tv/media", "/srv/test/tv", "//media", true, true},
                   {"nfs://192.168.0.1:2049/srv/test/tv/media/", "/srv/test/tv", "//media/", true, true},
                   {"nfs://192.168.0.1:2049/srv/test/tv", "/srv/test/tv", "//", true, true},
                   {"nfs://192.168.0.1:2049/srv/test/", "/srv/test", "//", true, true},
                   {"nfs://192.168.0.1:2049/", "/", "//", true, true},
                   {"nfs://192.168.0.1:2049/notexported/foo.txt", "/", "//notexported/foo.txt", true, true},

                   {"nfs://192.168.0.1:2049/notexported/foo.txt", "/notexported", "//foo.txt", false, false},
                 };

class TestNfs : public Test,
                public WithParamInterface<SplitPath>
{
};

class ExportList
{
  public: 
    std::list<std::string> data;

    ExportList()
    {
      data.push_back("/srv/test");
      data.push_back("/srv/test/tv");
      data.push_back("/");
      data.sort();
      data.reverse();
    }
};

static ExportList exportList;

TEST_P(TestNfs, splitUrlIntoExportAndPath)
{
  CURL url(GetParam().url);
  CStdString exportPath;
  CStdString relativePath;
  gNfsConnection.splitUrlIntoExportAndPath(url, exportPath, relativePath, exportList.data);

  if (GetParam().expectedResultExport)    
    EXPECT_STREQ(GetParam().exportPath, exportPath);
  else
    EXPECT_STRNE(GetParam().exportPath, exportPath);

  if (GetParam().expectedResultPath)
    EXPECT_STREQ(GetParam().relativePath, relativePath);
  else
    EXPECT_STRNE(GetParam().relativePath, relativePath);
}

INSTANTIATE_TEST_CASE_P(NfsFile, TestNfs, ValuesIn(g_TestData));
