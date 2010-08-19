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

#if (defined USE_EXTERNAL_FFMPEG)
  #include <libavutil/avutil.h>
#else
  #include "cores/dvdplayer/Codecs/ffmpeg/libavutil/avutil.h"
#endif

#include "AE.h"
#include "AEUtil.h"
#include "AudioRenderers/ALSADirectSound.h"

#ifdef __SSE__
#include <xmmintrin.h>
#endif

using namespace std;

CAE::CAE():
  m_running      (false),
  m_reOpened     (false),
  m_sink         (NULL ),
  m_passthrough  (false),
  m_buffer       (NULL ),
  m_audioCallback(NULL )
{
}

CAE::~CAE()
{
  Deinitialize();

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
  /* lock the sink so the thread gets held up */
  CSingleLock sinkLock(m_critSectionSink);

  /* load the configuration */
  enum AEStdChLayout stdChLayout = AE_CH_LAYOUT_2_0;
  switch(g_guiSettings.GetInt("audiooutput.channellayout")) {
    default:
    case 0: stdChLayout = AE_CH_LAYOUT_2_0; break;
    case 1: stdChLayout = AE_CH_LAYOUT_2_1; break;
    case 2: stdChLayout = AE_CH_LAYOUT_3_0; break;
    case 3: stdChLayout = AE_CH_LAYOUT_3_1; break;
    case 4: stdChLayout = AE_CH_LAYOUT_4_0; break;
    case 5: stdChLayout = AE_CH_LAYOUT_4_1; break;
    case 6: stdChLayout = AE_CH_LAYOUT_5_0; break;
    case 7: stdChLayout = AE_CH_LAYOUT_5_1; break;
    case 8: stdChLayout = AE_CH_LAYOUT_7_0; break;
    case 9: stdChLayout = AE_CH_LAYOUT_7_1; break;
  }
  /* choose a sample rate based on the oldest stream, or if non, 44100hz */
  unsigned int sampleRate = 44100;
  if (!m_streams.empty())
    sampleRate = m_streams.front()->GetSampleRate();

  /* if the sink is already open and it is compatible we dont need to do anything */
  if (m_sink && sampleRate == m_format.m_sampleRate && stdChLayout == m_stdChLayout)
    return true;

  /* let the thread know we have re-opened the sink */
  m_reOpened = true;

  /* we are going to open, so close the old sink if it was open */
  bool reopen = false;
  if (m_sink)
  {
    m_sink->Stop();
    m_sink->Deinitialize();
    delete m_sink;
    m_sink = NULL;
    _aligned_free(m_buffer);
    m_buffer = NULL;
    reopen = true;
  }

  /* set the local members */
  m_stdChLayout  = stdChLayout;
  m_chLayout     = CAEUtil::GetStdChLayout(m_stdChLayout);
  m_channelCount = CAEUtil::GetChLayoutCount(m_chLayout);
  CLog::Log(LOGDEBUG, "CAE::Initialize: Using speaker layout: %s", CAEUtil::GetStdChLayoutName(m_stdChLayout));

  /* create the new sink */
  m_sink = new CALSADirectSound();
  if (!m_sink->Initialize(NULL, "default", m_chLayout, sampleRate, 32, false, false, m_passthrough))
  {
    delete m_sink;
    m_sink = NULL;
    return false;
  }

  /* get the sink's audio format details */
  m_format        = m_sink->GetAudioFormat();
  m_frameSize     = sizeof(float) * m_channelCount;
  m_convertFn     = CAEConvert::FrFloat(m_format.m_dataFormat);
  m_buffer        = (uint8_t*)_aligned_malloc(m_format.m_frameSize * 2, 16);
  m_bufferSize    = 0;
  m_visBufferSize = 0;

  /* initialize the final stage remapper */
  m_remap.Initialize(m_chLayout, m_format.m_channelLayout, true);

  /* if we did not re-open, we are finished */
  if (!reopen)
    return true;

  /* re-init sounds */
  m_playing_sounds.clear();
  map<const CStdString, CAESound*>::iterator sitt;
  for(sitt = m_sounds.begin(); sitt != m_sounds.end(); ++sitt)
    sitt->second->Initialize();

  /* re-init streams */
  list<CAEStream*>::iterator itt;
  for(itt = m_streams.begin(); itt != m_streams.end(); ++itt)
    (*itt)->Initialize();

  /* re-init the callback */
  if (m_audioCallback)
  {
    m_audioCallback->OnDeinitialize();
    m_audioCallback->OnInitialize(m_channelCount, m_format.m_sampleRate, 32);
  }

  return true;
}

bool CAE::Initialize()
{
  /* get the current volume level */
  m_volume = g_settings.m_fVolumeLevel;
  return OpenSink();
}

void CAE::Deinitialize()
{
  Stop();

  if (m_sink)
  {
    m_sink->Stop();
    m_sink->Deinitialize();
    delete m_sink;
    m_sink = NULL;
  }

  _aligned_free(m_buffer);
  m_buffer = NULL;
}

void CAE::Stop()
{
  m_running = false;

  /* wait for the thread to stop */
  CSingleLock lock(m_runLock);
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
  CAEStream *stream = new CAEStream(dataFormat, sampleRate, channelCount, channelLayout, freeOnDrain, ownsPostProc);
  m_streams.push_back(stream);

  /* if we are the only stream, try to re-open the sink to what we want */
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
      owner  : sound,
      samples: sound->GetSamples(),
      frames : sound->GetFrameCount()
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
  CSingleLock lock(m_critSection);
  m_audioCallback = pCallback;
  if (m_audioCallback)
    m_audioCallback->OnInitialize(m_channelCount, m_format.m_sampleRate, 32);
}

void CAE::UnRegisterAudioCallback()
{
  CSingleLock lock(m_critSection);
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

float CAE::GetDelay()
{
  CSingleLock lock(m_critSection);
  if (!m_running) return 0.0f;

  CSingleLock sinkLock(m_critSectionSink);
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
inline void CAE::SSEMulAddArray(float *data, float *add, const float mul, uint32_t count)
{
  const __m128 m = _mm_set_ps1(mul);

  /* work around invalid alignment */
  while((((uintptr_t)data & 0xF) || ((uintptr_t)add & 0xF)) && count > 0)
  {
    data[0] += add[0] * mul;
    ++add;
    ++data;
    --count;
  }

  uint32_t even = count & ~0x3;
  for(uint32_t i = 0; i < even; i+=4, data+=4, add+=4)
  {
    __m128 ad      = _mm_load_ps(add );
    __m128 to      = _mm_load_ps(data);
    *(__m128*)data = _mm_add_ps (to, _mm_mul_ps(ad, m));
  }

  if (even != count)
  {
    uint32_t odd = count - even;
    if (odd == 1)
      data[0] += add[0] * mul;
    else
    {
      __m128 ad;
      __m128 to;
      if (odd == 2)
      {
        ad = _mm_setr_ps(add [0], add [1], 0, 0);
        to = _mm_setr_ps(data[0], data[1], 0, 0);
        __m128 ou = _mm_add_ps(to, _mm_mul_ps(ad, m));
        data[0] = ((float*)&ou)[0];
        data[1] = ((float*)&ou)[1];
      }
      else
      {
        ad = _mm_setr_ps(add [0], add [1], add [2], 0);
        to = _mm_setr_ps(data[0], data[1], data[2], 0);
        __m128 ou = _mm_add_ps(to, _mm_mul_ps(ad, m));
        data[0] = ((float*)&ou)[0];
        data[1] = ((float*)&ou)[1];
        data[2] = ((float*)&ou)[2];
      }
    }
  }
}

inline void CAE::SSEMulArray(float *data, const float mul, uint32_t count)
{
  const __m128 m = _mm_set_ps1(mul);

  /* work around invalid alignment */
  while(((uintptr_t)data & 0xF) && count > 0)
  {
    data[0] *= mul;
    ++data;
    --count;
  }

  uint32_t even = count & ~0x3;
  for(uint32_t i = 0; i < even; i+=4, data+=4)
  {
    __m128 to      = _mm_load_ps(data);
    *(__m128*)data = _mm_mul_ps (to, m);
  }

  if (even != count)
  {
    uint32_t odd = count - even;
    if (odd == 1)
      data[0] *= mul;
    else
    {     
      __m128 to;
      if (odd == 2)
      {
        to = _mm_setr_ps(data[0], data[1], 0, 0);
        __m128 ou = _mm_mul_ps(to, m);
        data[0] = ((float*)&ou)[0];
        data[1] = ((float*)&ou)[1];
      }
      else
      {
        to = _mm_setr_ps(data[0], data[1], data[2], 0);
        __m128 ou = _mm_mul_ps(to, m);
        data[0] = ((float*)&ou)[0];
        data[1] = ((float*)&ou)[1];
        data[2] = ((float*)&ou)[2];
      }
    }
  }
}
#endif

/* ==== MAIN PROCESSING LOOP ==== */
void CAE::Run()
{
  /* we release this when we exit the thread unblocking anyone waiting on "Stop" */
  CSingleLock runLock(m_runLock);
  m_running = true;

  CLog::Log(LOGINFO, "CAE::Run - Thread Started");
  while(m_running)
  {
    list<CAEStream*>::iterator itt;
    list<SoundState>::iterator sitt;
    CAEStream *stream;
    unsigned int div;
    unsigned int i;

/* ============== OUTPUT STAGE ============= */

    /* we need the sink */
    CSingleLock sinkLock(m_critSectionSink);

    /* we dont care if it has been re-opened here */
    m_reOpened = false;

    /*
       take a copy of the channel count so we dont crash during
       the loop if it changes on us
    */
    unsigned int ourChannelCount = m_channelCount;

    /* the size of these is dependant on the sink's channels */
    DECLARE_ALIGNED(16, float, out[m_channelCount         ]);
    DECLARE_ALIGNED(16, float, dst[m_format.m_channelCount]);

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

    memset(out, 0, sizeof(out));
    div = 1;

    sinkLock.Leave();

/* ============== AESound MIXER STAGE ============= */
    CSingleLock lock(m_critSection);

    /* mix in any sounds */
    for(sitt = m_playing_sounds.begin(); sitt != m_playing_sounds.end(); )
    {
      /* no more frames, so remove it from the list */
      if ((*sitt).frames == 0)
      {
        sitt = m_playing_sounds.erase(sitt);
        continue;
      }

      float *frame = (*sitt).samples;
      float volume = (*sitt).owner->GetVolume();
      (*sitt).samples += ourChannelCount;
      --(*sitt).frames;

      #ifdef __SSE__
      if (ourChannelCount > 1)
        CAE::SSEMulAddArray(out, frame, volume, ourChannelCount);
      else
      #endif
      for(i = 0; i < ourChannelCount; ++i)
        out[i] += frame[i] * volume;

      ++div;
      ++sitt;
    }

/* ============== AEStream MIXER STAGE ============= */

    /* mix in any running streams */
    for(itt = m_streams.begin(); itt != m_streams.end();)
    {
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

      float volume = stream->GetVolume();
      #ifdef __SSE__
      if (ourChannelCount > 1)
        CAE::SSEMulAddArray(out, frame, volume, ourChannelCount);
      else
      #endif
      {
        for(i = 0; i < ourChannelCount; ++i)
          out[i] += frame[i] * volume;
      }

      ++div;
      ++itt;
    }
    lock.Leave();

/* ============== NORMALIZATION STAGE ============= */

    if (div > 1)
    {
      #ifdef __SSE__
      if (ourChannelCount > 1)
        CAE::SSEMulArray(out, 1.0f / div, ourChannelCount);
      else
      #endif
      {
        float mul = 1.0f / div;
        for(i = 0; i < ourChannelCount; ++i)
          out[i] *= mul;
      }
    }

/* ============== VIS STAGE =============*/

    /* if we have an audio callback, use it */
    if (m_audioCallback)
    {
      /* add the frame to the visBuffer */
      memcpy(&m_visBuffer[m_visBufferSize], out, sizeof(out));
      m_visBufferSize += ourChannelCount;

      /* if the buffer full, flush it through */
      if (m_visBufferSize >= AUDIO_BUFFER_SIZE)
      {
        m_audioCallback->OnAudioData(m_visBuffer, AUDIO_BUFFER_SIZE);
        m_visBufferSize = 0;
      }
    }

/* ============== VOLUME STAGE ============= */
    /* if muted just zero the data and continue */
    if (g_settings.m_bMute)
    {
      memset(&m_buffer[m_bufferSize], 0, sizeof(out));
      m_bufferSize += sizeof(out);
      continue;
    }

    if (m_volume != 1.0f)
    {
      #ifdef __SSE__
      if (ourChannelCount > 1)
        CAE::SSEMulArray(out, m_volume, ourChannelCount);
      else
      #endif
      {
        for(i = 0; i < ourChannelCount; ++i)
          out[i] *= m_volume;
      }
    }

/* ============== REMAP AND BUFFER STAGE ============= */
    sinkLock.Enter();
    /* if the config has changed while in the loop, we need to assume the data is stuffed */
    if (m_reOpened)
    {
      m_reOpened = false;
      continue;
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
}

