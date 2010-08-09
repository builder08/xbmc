/*
 *      Copyright (C) 2005-2010 Team XBMC
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
 *  along with XBMC; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#ifndef AESTREAM_H
#define AESTREAM_H

#include <samplerate.h>
#include <list>

#include "utils/CriticalSection.h"

#include "AE.h"
#include "AEAudioFormat.h"
#include "AEConvert.h"
#include "AERemap.h"
#include "AEPostProc.h"

using namespace std;

class IAEPostProc;
class CAEStream
{
public:
  CAEStream(enum AEDataFormat format, unsigned int sampleRate, unsigned int channelCount, AEChLayout channelLayout);
  ~CAEStream();
  void Initialize();

  unsigned int GetFrameSize() {return m_format.m_frameSize;}
  unsigned int AddData(void *data, unsigned int size);
  float* GetFrame();
  float GetDelay();

  bool IsPaused() {return m_paused; }
  void Pause   () {m_paused = true; }
  void Resume  () {m_paused = false;}
  void Drain   ();
  void Flush   ();

  float GetVolume()             { return m_volume;   }
  void  SetVolume(float volume) { m_volume = std::max(0.0f, std::min(1.0f, volume)); }
  void  SetDynamicRangeCompression(int drc);

  void AppendPostProc(IAEPostProc *pp);

  unsigned int GetFrameSamples() { return m_format.m_frameSamples; }
  unsigned int GetChannelCount() { return m_format.m_channelCount; }
  unsigned int GetSampleRate()   { return m_format.m_sampleRate;   }
private:
  CCriticalSection  m_critSection;
  enum AEDataFormat m_initDataFormat;
  unsigned int      m_initSampleRate;
  unsigned int      m_initChannelCount;
  AEChLayout        m_initChannelLayout;
  
  typedef struct
  {
    unsigned int samples;
    float        *data;
  } PPacket;

  AEAudioFormat m_format;

  bool               m_resample;      /* true if the audio needs to be resampled  */
  bool               m_convert;       /* true if the bitspersample needs converting */
  float             *m_convertBuffer; /* buffer for converted data */
  bool               m_valid;         /* true if the stream is valid */
  CAERemap           m_remap;         /* the remapper */
  float              m_volume;        /* the volume level */
  list<IAEPostProc*> m_postProc;      /* post processing objects */

  CAEConvert::AEConvertToFn m_convertFn;

  uint8_t      *m_frameBuffer;
  unsigned int  m_frameBufferSize;
  unsigned int  m_bytesPerFrame;
  unsigned int  m_aeChannelCount;
  unsigned int  m_aePacketSamples;
  SRC_STATE    *m_ssrc;
  SRC_DATA      m_ssrcData;
  unsigned int  m_framesBuffered;
  list<PPacket> m_outBuffer;
  unsigned int  ProcessFrameBuffer();
  PPacket       m_newPacket;
  PPacket       m_packet;
  float        *m_packetPos;
  bool          m_paused;
};

#endif
