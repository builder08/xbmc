/*
 *      Copyright (C) 2016-present Team Kodi
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

#include "addons/AddonBuilder.h"
#include "addons/LanguageResource.h"
#include "gtest/gtest.h"

using namespace ADDON;


class TestAddonBuilder : public ::testing::Test
{
protected:
  CAddonBuilder builder;

  void SetUp() override
  {
    builder.SetId("foo.bar");
    builder.SetVersion(AddonVersion("1.2.3"));
  }
};

TEST_F(TestAddonBuilder, ShouldFailWhenIdIsNotSet)
{
  CAddonBuilder builder;
  builder.SetId("");
  EXPECT_EQ(nullptr, builder.Build());
}

TEST_F(TestAddonBuilder, ShouldBuildDependencyAddons)
{
  std::vector<DependencyInfo> deps;
  deps.emplace_back("a", AddonVersion("1.0.0"), false);
  builder.SetDependencies(deps);
  builder.SetType(ADDON_UNKNOWN);
  builder.SetExtPoint(nullptr);
  auto addon = builder.Build();
  EXPECT_EQ(deps, addon->GetDependencies());
}

TEST_F(TestAddonBuilder, ShouldReturnDerivedType)
{
  builder.SetType(ADDON_RESOURCE_LANGUAGE);
  auto addon = std::dynamic_pointer_cast<CLanguageResource>(builder.Build());
  EXPECT_NE(nullptr, addon);
}
