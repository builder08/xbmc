#pragma once
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

#ifndef SINKS_THREADS_THREAD_H_INCLUDED
#define SINKS_THREADS_THREAD_H_INCLUDED
#include "threads/Thread.h"
#endif

#ifndef SINKS_CORES_AUDIOENGINE_INTERFACES_AESINK_H_INCLUDED
#define SINKS_CORES_AUDIOENGINE_INTERFACES_AESINK_H_INCLUDED
#include "cores/AudioEngine/Interfaces/AESink.h"
#endif


class CAESinkNULL : public CThread, public IAESink
{
public:
  virtual const char *GetName() { return "NULL"; }

  CAESinkNULL();
  virtual ~CAESinkNULL();

  virtual bool Initialize(AEAudioFormat &format, std::string &device);
  virtual void Deinitialize();

  virtual double       GetDelay        ();
  virtual double       GetCacheTotal   ();
  virtual unsigned int AddPackets      (uint8_t *data, unsigned int frames, bool hasAudio, bool blocking = false);
  virtual void         Drain           ();

  static void          EnumerateDevices(AEDeviceList &devices, bool passthrough);
private:
  virtual void         Process();

  CEvent               m_wake;
  CEvent               m_inited;
  volatile bool        m_draining;
  AEAudioFormat        m_format;
  unsigned int         m_sink_frameSize;
  unsigned int         m_sinkbuffer_size;  ///< total size of the buffer
  unsigned int         m_sinkbuffer_level; ///< current level in the buffer
  double               m_sinkbuffer_sec_per_byte;
};
