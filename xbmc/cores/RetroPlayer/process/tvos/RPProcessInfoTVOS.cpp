/*
 *  Copyright (C) 2017-2018 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#include "RPProcessInfoTVOS.h"

using namespace KODI;
using namespace RETRO;

CRPProcessInfoTVOS::CRPProcessInfoTVOS() :
  CRPProcessInfo("tvOS")
{
}

CRPProcessInfo* CRPProcessInfoTVOS::Create()
{
  return new CRPProcessInfoTVOS();
}

void CRPProcessInfoTVOS::Register()
{
  CRPProcessInfo::RegisterProcessControl(CRPProcessInfoTVOS::Create);
}
