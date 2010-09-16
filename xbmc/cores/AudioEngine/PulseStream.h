#pragma once
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

#include "AEStream.h"
#include <pulse/pulseaudio.h>

class CPulseStream : public IAEStream
{
public:
  /* this should NEVER be called directly, use AE.GetStream */
  CPulseStream(pa_context *context, pa_threaded_mainloop *mainLoop, enum AEDataFormat format, unsigned int sampleRate, unsigned int channelCount, AEChLayout channelLayout, unsigned int options);
  virtual ~CPulseStream();

  virtual void Destroy();
  virtual void SetDataCallback (AECBFunc *cbFunc, void *arg); /* called when the buffer < 50% full */
  virtual void SetDrainCallback(AECBFunc *cbFunc, void *arg); /* called when the buffer has been drained */

  virtual unsigned int AddData(void *data, unsigned int size);
  virtual float GetDelay();

  virtual bool IsPaused     ();
  virtual bool IsDraining   ();
  virtual bool IsFreeOnDrain();
  virtual bool IsDestroyed  ();

  virtual void Pause   ();
  virtual void Resume  ();
  virtual void Drain   ();
  virtual void Flush   ();

  virtual float GetVolume    ();
  virtual float GetReplayGain();
  virtual void  SetVolume    (float volume);
  virtual void  SetReplayGain(float factor);

  virtual void AppendPostProc (IAEPostProc *pp);
  virtual void PrependPostProc(IAEPostProc *pp);
  virtual void RemovePostProc (IAEPostProc *pp);

  virtual unsigned int      GetFrameSize   ();
  virtual unsigned int      GetChannelCount();
  virtual unsigned int      GetSampleRate  ();
  virtual enum AEDataFormat GetDataFormat  ();
  virtual bool              IsRaw          ();

  /* for dynamic sample rate changes (smoothvideo) */
  virtual double GetResampleRatio();
  virtual void   SetResampleRatio(double ratio);

  /* vizualization callback register function */
  virtual void RegisterAudioCallback(IAudioCallback* pCallback);
  virtual void UnRegisterAudioCallback();
private:
  static void StreamRequestCallback(pa_stream *s, size_t length, void *userdata);
  static void StreamLatencyUpdateCallback(pa_stream *s, void *userdata);
  static void StreamStateCallback(pa_stream *s, void *userdata);

  static inline bool WaitForOperation(pa_operation *op, pa_threaded_mainloop *mainloop, const char *LogEntry);
  bool Cork(bool cork);

  bool m_Initialized;
  bool m_Paused;

  pa_stream *m_Stream;
  pa_sample_spec m_SampleSpec;
  pa_cvolume m_Volume;

  pa_context *m_Context;
  pa_threaded_mainloop *m_MainLoop;

  IAudioCallback* m_AudioCallback;

  AECBFunc *m_AudioDataCallback;
  AECBFunc *m_AudioDrainCallback;
  void *m_AudioDataArg;
  void *m_AudioDrainArg;

  enum AEDataFormat m_format;
  unsigned int m_sampleRate;
  unsigned int m_channelCount;
  AEChLayout m_channelLayout;
  unsigned int m_options;
  unsigned int m_frameSize;
  unsigned int m_frameSamples;
};

