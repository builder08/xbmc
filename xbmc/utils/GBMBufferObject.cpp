/*
 *  Copyright (c) 2020 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#include "GBMBufferObject.h"

#include "ServiceBroker.h"
#include "windowing/gbm/WinSystemGbmEGLContext.h"

#include <gbm.h>

using namespace KODI::WINDOWING::GBM;

CGBMBufferObject::CGBMBufferObject(int format) :
  m_format(format)
{
  m_device = static_cast<CWinSystemGbmEGLContext*>(CServiceBroker::GetWinSystem())->GetGBMDevice();
}

CGBMBufferObject::~CGBMBufferObject()
{
  ReleaseMemory();
  DestroyBufferObject();
}

bool CGBMBufferObject::CreateBufferObject(int width, int height)
{
  m_width = width;
  m_height = height;

  m_bo = gbm_bo_create(m_device, m_width, m_height, m_format, GBM_BO_USE_LINEAR);

  if (!m_bo)
    return false;

  m_fd = gbm_bo_get_fd(m_bo);

  return true;
}

void CGBMBufferObject::DestroyBufferObject()
{
  if (m_bo)
    gbm_bo_destroy(m_bo);
}

uint8_t* CGBMBufferObject::GetMemory()
{
  if (m_bo)
  {
    m_map = static_cast<uint8_t*>(gbm_bo_map(m_bo, 0, 0, m_width, m_height, GBM_BO_TRANSFER_WRITE, &m_stride, &m_map_data));
    if (m_map)
      return m_map;
  }

  return nullptr;
}

void CGBMBufferObject::ReleaseMemory()
{
  if (m_bo && m_map)
  {
    gbm_bo_unmap(m_bo, m_map_data);
    m_map_data = nullptr;
    m_map = nullptr;
  }
}

int CGBMBufferObject::GetFd()
{
  return m_fd;
}

int CGBMBufferObject::GetStride()
{
  return m_stride;
}

uint64_t CGBMBufferObject::GetModifier()
{
#if defined(HAS_GBM_MODIFIERS)
  return gbm_bo_get_modifier(m_bo);
#else
  return 0;
#endif
}
