/*
 *      Copyright (C) 2005-present Team Kodi
 *      This file is part of Kodi - https://kodi.tv
 *
 *  Kodi is free software: you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  Kodi is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Kodi. If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "filesystem/Directory.h"
#include "filesystem/IDirectory.h"
#include "filesystem/SpecialProtocol.h"
#include "FileItem.h"
#include "utils/URIUtils.h"
#include "test/TestUtils.h"

#include "gtest/gtest.h"

TEST(TestDirectory, General)
{
  std::string tmppath1, tmppath2, tmppath3;
  CFileItemList items;
  CFileItemPtr itemptr;
  tmppath1 = CSpecialProtocol::TranslatePath("special://temp/");
  tmppath1 = URIUtils::AddFileToFolder(tmppath1, "TestDirectory");
  tmppath2 = tmppath1;
  tmppath2 = URIUtils::AddFileToFolder(tmppath2, "subdir");
  EXPECT_TRUE(XFILE::CDirectory::Create(tmppath1));
  EXPECT_TRUE(XFILE::CDirectory::Exists(tmppath1));
  EXPECT_FALSE(XFILE::CDirectory::Exists(tmppath2));
  EXPECT_TRUE(XFILE::CDirectory::Create(tmppath2));
  EXPECT_TRUE(XFILE::CDirectory::Exists(tmppath2));
  EXPECT_TRUE(XFILE::CDirectory::GetDirectory(tmppath1, items, "", XFILE::DIR_FLAG_DEFAULTS));
  XFILE::CDirectory::FilterFileDirectories(items, "");
  tmppath3 = tmppath2;
  URIUtils::AddSlashAtEnd(tmppath3);
  itemptr = items[0];
  EXPECT_STREQ(tmppath3.c_str(), itemptr->GetPath().c_str());
  EXPECT_TRUE(XFILE::CDirectory::Remove(tmppath2));
  EXPECT_FALSE(XFILE::CDirectory::Exists(tmppath2));
  EXPECT_TRUE(XFILE::CDirectory::Exists(tmppath1));
  EXPECT_TRUE(XFILE::CDirectory::Remove(tmppath1));
  EXPECT_FALSE(XFILE::CDirectory::Exists(tmppath1));
}

TEST(TestDirectory, CreateRecursive)
{
  auto path1 = URIUtils::AddFileToFolder(
    CSpecialProtocol::TranslatePath("special://temp/"),
    "level1");
  auto path2 = URIUtils::AddFileToFolder(path1,
    "level2",
    "level3");

  EXPECT_TRUE(XFILE::CDirectory::Create(path2));
  EXPECT_TRUE(XFILE::CDirectory::RemoveRecursive(path1));
}
