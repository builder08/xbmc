/*
 *      Copyright (C) 2010-2013 Team XBMC
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

#ifndef SINKS_SYSTEM_H_INCLUDED
#define SINKS_SYSTEM_H_INCLUDED
#include "system.h"
#endif


#include <stdint.h>
#include <limits.h>

#ifndef SINKS_CORES_AUDIOENGINE_SINKS_AESINKPROFILER_H_INCLUDED
#define SINKS_CORES_AUDIOENGINE_SINKS_AESINKPROFILER_H_INCLUDED
#include "cores/AudioEngine/Sinks/AESinkProfiler.h"
#endif

#ifndef SINKS_CORES_AUDIOENGINE_UTILS_AEUTIL_H_INCLUDED
#define SINKS_CORES_AUDIOENGINE_UTILS_AEUTIL_H_INCLUDED
#include "cores/AudioEngine/Utils/AEUtil.h"
#endif

#ifndef SINKS_UTILS_STDSTRING_H_INCLUDED
#define SINKS_UTILS_STDSTRING_H_INCLUDED
#include "utils/StdString.h"
#endif

#ifndef SINKS_UTILS_LOG_H_INCLUDED
#define SINKS_UTILS_LOG_H_INCLUDED
#include "utils/log.h"
#endif

#ifndef SINKS_UTILS_TIMEUTILS_H_INCLUDED
#define SINKS_UTILS_TIMEUTILS_H_INCLUDED
#include "utils/TimeUtils.h"
#endif


CAESinkProfiler::CAESinkProfiler()
{
}

CAESinkProfiler::~CAESinkProfiler()
{
}

bool CAESinkProfiler::Initialize(AEAudioFormat &format, std::string &device)
{
  if (AE_IS_RAW(format.m_dataFormat))
    return false;

  format.m_sampleRate    = 192000;
  format.m_channelLayout = AE_CH_LAYOUT_7_1;
  format.m_dataFormat    = AE_FMT_S32LE;
  format.m_frames        = 30720;
  format.m_frameSamples  = format.m_channelLayout.Count();
  format.m_frameSize     = format.m_frameSamples * sizeof(float);
  return true;
}

void CAESinkProfiler::Deinitialize()
{
}

double CAESinkProfiler::GetDelay()
{
  return 0.0f;
}

unsigned int CAESinkProfiler::AddPackets(uint8_t *data, unsigned int frames, bool hasAudio, bool blocking)
{
  int64_t ts = CurrentHostCounter();
  CLog::Log(LOGDEBUG, "CAESinkProfiler::AddPackets - latency %f ms", (float)(ts - m_ts) / 1000000.0f);
  m_ts = ts;
  return frames;
}

void CAESinkProfiler::Drain()
{
}

void CAESinkProfiler::EnumerateDevices (AEDeviceList &devices, bool passthrough)
{
  devices.push_back(AEDevice("Profiler", "Profiler"));
}
