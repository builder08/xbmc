/*
 *  Copyright (C) 2022 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#include "DiscsUtils.h"

extern "C"
{
#include "dvdnav/dvdnav.h"
}
#ifdef HAVE_LIBBLURAY
//! @todo it's wrong to include vfs scoped files in a utils class, refactor
// to use libbluray directly.
#include "filesystem/BlurayDirectory.h"
#endif


bool UTILS::DISCS::GetDiscInfo(UTILS::DISCS::DiscInfo& info, const std::string& mediaPath)
{
  // try to probe as a DVD
  info = ProbeDVDDiscInfo(mediaPath);
  if (!info.empty())
    return true;

  // try to probe as Blu-ray
  info = ProbeBlurayDiscInfo(mediaPath);
  if (!info.empty())
    return true;

  return false;
}

UTILS::DISCS::DiscInfo UTILS::DISCS::ProbeDVDDiscInfo(const std::string& mediaPath)
{
  DiscInfo info;

  struct dvdnav_s* dvdnav;
  if (dvdnav_open(&dvdnav, mediaPath.c_str()) == DVDNAV_STATUS_OK)
  {
    info.type = DiscType::DVD;

    // get DVD title
    const char* dvdTitle = nullptr;
    if (dvdnav_get_title_string(dvdnav, &dvdTitle) == DVDNAV_STATUS_OK)
      info.name = std::string{dvdTitle};

    // get DVD serial number
    const char* dvdSerialNumber = nullptr;
    if (dvdnav_get_serial_string(dvdnav, &dvdSerialNumber) == DVDNAV_STATUS_OK)
      info.serial = std::string{dvdSerialNumber};
  }

  return info;
}

UTILS::DISCS::DiscInfo UTILS::DISCS::ProbeBlurayDiscInfo(const std::string& mediaPath)
{
  DiscInfo info;
#ifdef HAVE_LIBBLURAY
  XFILE::CBlurayDirectory bdDir;
  if (!bdDir.InitializeBluray(mediaPath))
    return info;

  info.type = DiscType::BLURAY;
  info.name = bdDir.GetBlurayTitle();
  info.serial = bdDir.GetBlurayID();
#endif
  return info;
}
