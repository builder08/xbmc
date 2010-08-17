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

#include "utils/TimeUtils.h"
#include "utils/SingleLock.h"
#include "utils/log.h"
#include "GUISettings.h"

#include "libavutil/avutil.h" /* DECLARE_ALIGNED */

#include "AE.h"
#include "AEUtil.h"
#include "AudioRenderers/ALSADirectSound.h"

#ifdef __SSE__
#include <xmmintrin.h>
#endif

using namespace std;

CAE::CAE():
  m_state        (AE_STATE_INVALID),
  m_sink         (NULL ),
  m_passthrough  (false),
  m_buffer       (NULL ),
  m_audioCallback(NULL )
{
}

CAE::~CAE()
{
  /* free the streams */
  while(!m_streams.empty())
  {
    CAEStream *s = m_streams.front();
    /* note: the stream will call RemoveStream via it's dtor */
    delete s;
  }
}

bool CAE::OpenSink()
{
  unsigned int sampleRate = 44100;
  if (!m_streams.empty())
    sampleRate = m_streams.front()->GetSampleRate();

  CSingleLock sinkLock(m_critSectionSink);
  /* if the sink is open and the sampleRate & data format has not changed, dont re-open */
  if (m_sink && sampleRate == m_format.m_sampleRate && (m_passthrough && m_format.m_dataFormat == AE_FMT_IEC958))
  {
    return true;
  }

  /* close the old sink if it is open */
  if (m_sink)
  {
    m_sink->Stop();
    m_sink->Deinitialize();
    delete m_sink;
    m_sink = NULL;

    _aligned_free(m_buffer);
    m_buffer = NULL;
  }

  CLog::Log(LOGDEBUG, "CAE::OpenSink - %uHz\n", sampleRate);
  m_sink = new CALSADirectSound();
  if (!m_sink->Initialize(NULL, "default", m_chLayout, sampleRate, 16, false, false, m_passthrough))
  {
    delete m_sink;
    m_sink = NULL;
    return false;
  }

  m_format        = m_sink->GetAudioFormat();
  m_frameSize     = sizeof(float) * m_channelCount;
  m_convertFn     = CAEConvert::FrFloat(m_format.m_dataFormat);
  m_buffer        = (uint8_t*)_aligned_malloc(m_format.m_frameSize * 2, 16);
  m_bufferSize    = 0;
  m_visBufferSize = 0;

  m_remap.Initialize(m_chLayout, m_format.m_channelLayout, true);

  /* re-init sounds */
  map<const CStdString, CAESound*>::iterator sitt;
  for(sitt = m_sounds.begin(); sitt != m_sounds.end(); ++sitt)
    sitt->second->Initialize();

  /* re-init streams */
  list<CAEStream*>::iterator itt;
  for(itt = m_streams.begin(); itt != m_streams.end(); ++itt)
    (*itt)->Initialize();

  /* re-init the callback */
  CSingleLock acLock(m_critSectionAC);
  if (m_audioCallback)
  {
    m_audioCallback->OnDeinitialize();
    m_audioCallback->OnInitialize(m_channelCount, m_format.m_sampleRate, 32);
  }

  return true;
}

bool CAE::Initialize()
{
  m_volume = g_settings.m_fVolumeLevel;

  /* open the renderer */
  enum AEStdChLayout chLayout = AE_CH_LAYOUT_2_0;
  switch(g_guiSettings.GetInt("audiooutput.channellayout")) {
    default:
    case 0: chLayout = AE_CH_LAYOUT_2_0; break;
    case 1: chLayout = AE_CH_LAYOUT_2_1; break;
    case 2: chLayout = AE_CH_LAYOUT_3_0; break;
    case 3: chLayout = AE_CH_LAYOUT_3_1; break;
    case 4: chLayout = AE_CH_LAYOUT_4_0; break;
    case 5: chLayout = AE_CH_LAYOUT_4_1; break;
    case 6: chLayout = AE_CH_LAYOUT_5_0; break;
    case 7: chLayout = AE_CH_LAYOUT_5_1; break;
    case 8: chLayout = AE_CH_LAYOUT_7_0; break;
    case 9: chLayout = AE_CH_LAYOUT_7_1; break;
  }
  m_chLayout     = CAEUtil::GetStdChLayout(chLayout);
  m_channelCount = CAEUtil::GetChLayoutCount(m_chLayout);
  CLog::Log(LOGDEBUG, "CAE::Initialize: Configured speaker layout: %s", CAEUtil::GetStdChLayoutName(chLayout));

  /* pretend that we have a 44.1khz float sink for sounds */
  m_format.m_sampleRate    = 44100;
  m_format.m_dataFormat    = AE_FMT_FLOAT;
  m_format.m_channelCount  = m_channelCount;
  m_format.m_channelLayout = m_chLayout;

  if (!m_streams.empty())
  {
    if (!OpenSink()) return false;
    m_state = AE_STATE_READY;
    return true;
  }

  /* re-init sounds */
  map<const CStdString, CAESound*>::iterator sitt;
  for(sitt = m_sounds.begin(); sitt != m_sounds.end(); ++sitt)
    sitt->second->Initialize();

  m_state = AE_STATE_READY;
  return true;
}

void CAE::DeInitialize()
{
  CSingleLock sinkLock(m_critSectionSink);

  m_state = AE_STATE_SHUTDOWN;
  if (m_sink)
  {
    m_sink->Stop();
    m_sink->Deinitialize();
    delete m_sink;
    m_sink = NULL;
  }

  _aligned_free(m_buffer);
  m_buffer = NULL;

  m_state = AE_STATE_INVALID;
}

enum AEState CAE::GetState()
{
  CSingleLock lock(m_critSection);
  return m_state;
}

CAEStream *CAE::GetStream(enum AEDataFormat dataFormat, unsigned int sampleRate, unsigned int channelCount, AEChLayout channelLayout, bool freeOnDrain/* = false */, bool ownsPostProc/* = false */)
{
  CLog::Log(LOGINFO, "CAE::GetStream - %d, %u, %u, %s",
    CAEUtil::DataFormatToBits(dataFormat),
    sampleRate,
    channelCount,
    CAEUtil::GetChLayoutStr(channelLayout).c_str()
  );

  CSingleLock lock(m_critSection);
  if (dataFormat == AE_FMT_IEC958 && m_passthrough)
    return NULL;

  CAEStream *stream = new CAEStream(dataFormat, sampleRate, channelCount, channelLayout, freeOnDrain, ownsPostProc);
  m_streams.push_back(stream);

  /* if the stream is IEC958 turn on passthrough */
  if (dataFormat == AE_FMT_IEC958)
  {
    m_passthrough = true;
    OpenSink();
  }
  else
    /* if we are the only stream, re-open the sink */
    if (m_streams.size() == 1)
      OpenSink();

  return stream;
}

CAESound *CAE::GetSound(CStdString file)
{
  CSingleLock lock(m_critSection);
  CAESound *sound;

  /* see if we have a valid sound */
  if ((sound = m_sounds[file]))
  {
    /* increment the reference count */
    ++sound->m_refcount;
    return sound;
  }

  sound = new CAESound(file);
  if (!sound->Initialize())
  {
    delete sound;
    return NULL;
  }

  m_sounds[file] = sound;
  sound->m_refcount = 1;

  return sound;
}

void CAE::PlaySound(CAESound *sound)
{
   SoundState ss = {
      owner: sound,
      frame: 0
   };
   CSingleLock lock(m_critSection);
   OpenSink();
   m_playing_sounds.push_back(ss);
}

void CAE::FreeSound(CAESound *sound)
{
  CSingleLock lock(m_critSection);
  /* decrement the sound's ref count */
  --sound->m_refcount;
  ASSERT(sound->m_refcount >= 0);

  /* if other processes are using the sound, dont remove it */
  if (sound->m_refcount > 0)
    return;

  /* set the timeout to 30 seconds */
  sound->m_ts = CTimeUtils::GetTimeMS() + 30000;

  /* stop the sound playing */
  list<SoundState>::iterator itt;
  for(itt = m_playing_sounds.begin(); itt != m_playing_sounds.end(); )
  {
    if ((*itt).owner == sound) itt = m_playing_sounds.erase(itt);
    else ++itt;
  }
}

void CAE::GarbageCollect()
{
  CSingleLock lock(m_critSection);

  unsigned int ts = CTimeUtils::GetTimeMS();
  map<const CStdString, CAESound*>::iterator itt;
  list<map<const CStdString, CAESound*>::iterator> remove;

  for(itt = m_sounds.begin(); itt != m_sounds.end(); ++itt)
  {
    CAESound *sound = itt->second;
    /* free any sounds that are no longer used and are > 30 seconds old */
    if (sound->m_refcount == 0 && ts > sound->m_ts)
    {
      delete sound;
      remove.push_back(itt);
      continue;
    }
  }

  /* erase the entries from the map */
  while(!remove.empty())
  {
    m_sounds.erase(remove.front());
    remove.pop_front();
  }
}

void CAE::RegisterAudioCallback(IAudioCallback* pCallback)
{
  CSingleLock acLock(m_critSectionAC);
  m_audioCallback = pCallback;
  if (m_audioCallback)
    m_audioCallback->OnInitialize(m_channelCount, m_format.m_sampleRate, 32);
}

void CAE::UnRegisterAudioCallback()
{
  CSingleLock acLock(m_critSectionAC);
  m_audioCallback = NULL;
  m_visBufferSize = 0;
}

void CAE::StopSound(CAESound *sound)
{
  CSingleLock lock(m_critSection);
  list<SoundState>::iterator itt;
  for(itt = m_playing_sounds.begin(); itt != m_playing_sounds.end(); )
  {
    if ((*itt).owner == sound) itt = m_playing_sounds.erase(itt);
    else ++itt;
  }
}

bool CAE::IsPlaying(CAESound *sound)
{
  CSingleLock lock(m_critSection);
  list<SoundState>::iterator itt;
  for(itt = m_playing_sounds.begin(); itt != m_playing_sounds.end(); ++itt)
    if ((*itt).owner == sound) return true;
  return false;
}

void CAE::RemoveStream(CAEStream *stream)
{
  CSingleLock lock(m_critSection);
  m_streams.remove(stream);
  if (stream->GetDataFormat() == AE_FMT_IEC958)
  {
    m_passthrough = false;
    OpenSink();
  }
}

void CAE::Run()
{
  CSingleLock lock    (m_critSection);

  /* so we can lock the audio sink */
  CSingleLock sinkLock(m_critSectionSink);
  sinkLock.Leave();

  if (!AE.Initialize())
  {
    CLog::Log(LOGERROR, "CAE::Run - Failed to initialize");
    return;
  }

  m_state = AE_STATE_RUN;
  lock.Leave();

  CLog::Log(LOGINFO, "CAE::Run - Thread Started");
  while(GetState() == AE_STATE_RUN)
  {
    /* if a sink has not been opened yet */
    sinkLock.Enter();
    if (m_sink == NULL)
    {
      sinkLock.Leave();
      usleep(1000);
      continue;
    }

    list<CAEStream*>::iterator itt;
    list<SoundState>::iterator sitt;
    CAEStream *stream;

    DECLARE_ALIGNED(16, float, out[m_channelCount         ]);
    DECLARE_ALIGNED(16, float, dst[m_format.m_channelCount]);

    unsigned int div;
    unsigned int i;

    /* this normally only loops once */
    while(m_bufferSize >= m_format.m_frameSize)
    {
        /* this call must block! */
        int wrote = m_sink->AddPackets(m_buffer, m_bufferSize);
        if (!wrote) continue;

	int left = m_bufferSize - wrote;
        memmove(&m_buffer[0], &m_buffer[wrote], left);
        m_bufferSize -= wrote;
    }

    /* we are finished with the sink */
    sinkLock.Leave();

    memset(out, 0, sizeof(out));
    div = 1;

    lock.Enter();
    /* mix in any sounds */
    for(sitt = m_playing_sounds.begin(); sitt != m_playing_sounds.end(); )
    {
      if (m_state != AE_STATE_RUN) break;

      float *frame = (*sitt).owner->GetFrame((*sitt).frame++);
      /* if no more frames, remove it from the list */
      if (frame == NULL)
      {
        sitt = m_playing_sounds.erase(sitt);
        continue;
      }

      /* we still need to take frames when muted or in passthrough */
      if (!m_passthrough && !g_settings.m_bMute)
      {
        /* mix the frame into the output */
        float volume = (*sitt).owner->GetVolume();
        #ifdef __SSE__
        if (m_channelCount > 1)
          CAE::SSEMixSamples(out, frame, m_channelCount, volume);
        else
        #endif
        {
          for(i = 0; i < m_channelCount; ++i)
            out[i] += frame[i] * volume;
        }
        ++div;
      }

      ++sitt;
    }

    /* mix in any running streams */
    bool done = false;
    for(itt = m_streams.begin(); itt != m_streams.end();)
    {
      if (m_state != AE_STATE_RUN) break;
      stream = *itt;

      /* delete streams that are flagged for deletion */
      if (stream->m_delete)
      {
        itt = m_streams.erase(itt);
        delete stream;
        continue;
      }

      /* dont process streams that are paused */
      if (stream->IsPaused()) {
        ++itt;
        continue;
      }

      float *frame = stream->GetFrame();
      if (!frame)
      {
        /* if the stream is drained and is set to free on drain */
        if (stream->IsDraining() && stream->IsFreeOnDrain())
        {
          itt = m_streams.erase(itt);
          delete stream;
          continue;
        }

        ++itt;
        continue;
      }

      if (m_passthrough)
      {
        if (stream->GetDataFormat() == AE_FMT_IEC958 && !done)
        {
          memcpy(out, frame, sizeof(float) * m_channelCount);
          done = true;
        }
      }
      else if (!g_settings.m_bMute)
      {
        float volume = stream->GetVolume();
        #if __SSE__
        if (m_channelCount > 1)
          CAE::SSEMixSamples(out, frame, m_channelCount, volume);
        else
        #endif
        {
          for(i = 0; i < m_channelCount; ++i)
            out[i] += frame[i] * volume;
        }
        ++div;
      }

      ++itt;
    }
    lock.Leave();

    /* if muted just zero the data and continue */
    if (g_settings.m_bMute) {
      memset(&m_buffer[m_bufferSize], 0, sizeof(out));
      m_bufferSize += sizeof(out);
      continue;
    }

    if (!m_passthrough)
    {
      #ifdef __SSE__
      if (m_channelCount > 1)
        CAE::SSEDeAmpSamples(out, m_channelCount, m_volume);
      else
      #endif
      {
        for(i = 0; i < m_channelCount; ++i)
          out[i] *= m_volume;
      }

      if (div > 1)
      {
	#ifdef __SSE__
        if (m_channelCount > 1)
          CAE::SSENormalizeSamples(out, m_channelCount, div);
        else
	#endif
        {
          float mul = 1.0f / div;
          for(i = 0; i < m_channelCount; ++i)
            out[i] *= mul;
	}
      }

      /* if we have an audio callback, use it */
      CSingleLock acLock(m_critSectionAC);
      if (m_audioCallback)
      {
        /* add the frame to the visBuffer */
        memcpy(&m_visBuffer[m_visBufferSize], out, sizeof(out));
        m_visBufferSize += m_channelCount;

        /* if the buffer full, flush it through */
        if (m_visBufferSize >= AUDIO_BUFFER_SIZE)
        {
          m_audioCallback->OnAudioData(m_visBuffer, AUDIO_BUFFER_SIZE);
          m_visBufferSize = 0;
        }
      }
    }

    /* remap the frame before we buffer it */
    m_remap.Remap(out, dst, 1);

    /* do we need to convert */
    if (m_convertFn)
      m_bufferSize += m_convertFn(dst, m_format.m_channelCount, &m_buffer[m_bufferSize]);
    else
    {
      memcpy(&m_buffer[m_bufferSize], dst, sizeof(dst));
      m_bufferSize += sizeof(dst);
    }
  }

  CLog::Log(LOGINFO, "CAE::Run - Thread Terminating");
  lock.Enter();
  DeInitialize();
  m_state = AE_STATE_INVALID;
  lock.Leave();
}

void CAE::Stop()
{
  CSingleLock lock(m_critSection);
  if (m_state == AE_STATE_READY) return;
  m_state = AE_STATE_STOP;
}

float CAE::GetDelay()
{
  CSingleLock lock    (m_critSection    );
  CSingleLock sinkLock(m_critSectionSink);

  if (m_state == AE_STATE_INVALID) return 0.0f;
  return m_sink->GetDelay() + m_bufferSize / m_frameSize / m_format.m_sampleRate;
}

float CAE::GetVolume()
{
  CSingleLock lock(m_critSection);
  return m_volume;
}

void CAE::SetVolume(float volume)
{
  CSingleLock lock(m_critSection);
  g_settings.m_fVolumeLevel = volume;
  m_volume = volume;
}

#ifdef __SSE__
inline void CAE::SSENormalizeSamples(float *samples, uint32_t count, const uint32_t div)
{
  const __m128 mul = _mm_set_ps1(1.0f / div);
  uint32_t even = (count / 4) * 4;
  for(uint32_t i = 0; i < even; i+=4, samples+=4)
    *((__m128*)samples) = _mm_mul_ps(_mm_load_ps(samples), mul);

  if (even != count)
  {
    uint32_t odd = count - even;
    __m128 in;
    memcpy(&in, samples, sizeof(float) * odd);
    __m128 out = _mm_mul_ps(in, mul);
    memcpy(samples, &out, sizeof(float) * odd);
  }
}

inline void CAE::SSEMixSamples(float *dest, float *src, uint32_t count, const float volume)
{
  const __m128 vol = _mm_set_ps1(volume);
  uint32_t even = (count / 4) * 4;
  for(uint32_t i = 0; i < even; i+=4, dest+=4, src+=4)
    *((__m128*)dest) = _mm_add_ps(_mm_load_ps(dest), _mm_mul_ps(_mm_load_ps(src), vol));

  if (even != count)
  {
    uint32_t odd = count - even;
    __m128 in, ou;
    memcpy(&in, src , sizeof(float) * odd);
    memcpy(&ou, dest, sizeof(float) * odd);
    __m128 out = _mm_add_ps(in, _mm_mul_ps(ou, vol));
    memcpy(dest, &out, sizeof(float) * odd);
  }
}

inline void CAE::SSEDeAmpSamples(float *samples, uint32_t count, const float volume)
{
  const __m128 vol = _mm_set_ps1(volume);
  uint32_t even = (count / 4) * 4;
  for(uint32_t i = 0; i < even; i+=4, samples+=4)
    *((__m128*)samples) = _mm_mul_ps(_mm_load_ps(samples), vol);

  if (even != count)
  {
    uint32_t odd = count - even;
    __m128 in;
    memcpy(&in, samples, sizeof(float) * odd);
    __m128 out = _mm_mul_ps(in, vol);
    memcpy(samples, &out, sizeof(float) * odd);
  }
}
#endif

