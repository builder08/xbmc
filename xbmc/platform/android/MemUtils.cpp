/*
 *  Copyright (c) 2020 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#include "utils/MemUtils.h"

#include "platform/android/activity/XBMCApp.h"

#include <stdlib.h>

namespace KODI
{
namespace MEMORY
{

void* AlignedMalloc(size_t s, size_t alignTo)
{
  void* p;
  posix_memalign(&p, alignTo, s);

  return p;
}

void AlignedFree(void* p)
{
  free(p);
}

void GetMemoryStatus(MemoryStatus* buffer)
{
  if (!buffer)
    return;

  long availMem, totalMem;

  if (CXBMCApp::get()->GetMemoryInfo(availMem, totalMem))
  {
    *buffer = {};
    buffer->totalPhys = totalMem;
    buffer->availPhys = availMem;
  }
}

}
}
