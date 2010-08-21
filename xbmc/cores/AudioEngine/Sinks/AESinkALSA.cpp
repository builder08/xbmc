/*
 *      Copyright (C) 2005-2010 Team XBMC
 *      http://www.xbmc.org
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

#include "AESinkALSA.h"
#include <stdint.h>
#include <limits.h>

#include "AEUtil.h"
#include "StdString.h"
#include "utils/log.h"
#include "utils/SingleLock.h"

CAESinkALSA::CAESinkALSA() :
  m_pcm       (NULL ),
  m_running   (false),
  m_bufferWait(1)
{
  /* ensure that ALSA has been initialized */
  if(!snd_config)
    snd_config_update();

  m_buffer[0] = NULL;
  m_buffer[1] = NULL;
}

CAESinkALSA::~CAESinkALSA()
{
  Deinitialize();
}

bool CAESinkALSA::Initialize(AEAudioFormat format)
{
  static enum AEChannel ALSAChannelMap[9] =
    {AE_CH_FL, AE_CH_FR, AE_CH_BL, AE_CH_BR, AE_CH_FC, AE_CH_LFE, AE_CH_SL, AE_CH_SR, AE_CH_NULL};

  int i, c;

  format.m_channelCount = 0;
  for(c = 0; c < 8; ++c)
    for(i = 0; format.m_channelLayout[i] != AE_CH_NULL; ++i)
      if (format.m_channelLayout[i] == ALSAChannelMap[c])
      {
        format.m_channelCount = c + 1;
        break;
      }

  if (format.m_channelCount == 0)
  {
    CLog::Log(LOGERROR, "CAESinkALSA::Initialize - Unable to open the requested channel layout");
    return false;
  }

  format.m_channelLayout      = ALSAChannelMap;
  CStdString       pcm_device = "default"; //FIXME: load from settings
  snd_config_t     *config;

  format.m_frameSamples = 512;
  format.m_frames       = 16;

  if (pcm_device == "default")
    switch(m_format.m_channelCount)
    {
      case 8: pcm_device = "plug:surround71"; break;
      case 6: pcm_device = "plug:surround51"; break;
      case 5: pcm_device = "plug:surround50"; break;
      case 4: pcm_device = "plug:surround40"; break;
    }

  /* get the sound config */
  snd_config_copy(&config, snd_config);
  int error;

  error = snd_pcm_open_lconf(&m_pcm, pcm_device.c_str(), SND_PCM_STREAM_PLAYBACK, 0, config);
  if (error < 0)
  {
    CLog::Log(LOGERROR, "CAESinkALSA::Initialize - snd_pcm_open_lconf(%d)", error);
    snd_config_delete(config);
    return false;
  }

  /* free the sound config */
  snd_config_delete(config);

  if (!InitializeHW(format)) return false;
  if (!InitializeSW(format)) return false;

  m_buffer        = new uint8_t[format.m_frameSize];
  m_bufferSamples = 0;

  snd_pcm_prepare(m_pcm);
  snd_pcm_start(m_pcm);

  m_format  = format;
  m_running = true;
  return true;
}

snd_pcm_format_t CAESinkALSA::AEFormatToALSAFormat(const enum AEDataFormat format)
{
  switch(format)
  {
    case AE_FMT_S8    : return SND_PCM_FORMAT_S8;
    case AE_FMT_U8    : return SND_PCM_FORMAT_U8;
    case AE_FMT_S16LE : return SND_PCM_FORMAT_S16_LE;
    case AE_FMT_S16BE : return SND_PCM_FORMAT_S16_BE;
    case AE_FMT_FLOAT : return SND_PCM_FORMAT_FLOAT;
    case AE_FMT_IEC958: return SND_PCM_FORMAT_S16_BE;

    default:
      return SND_PCM_FORMAT_UNKNOWN;
  }
}

bool CAESinkALSA::InitializeHW(AEAudioFormat &format)
{
  snd_pcm_hw_params_t *hw_params;

  snd_pcm_hw_params_malloc(&hw_params);
  snd_pcm_hw_params_any(m_pcm, hw_params);
  snd_pcm_hw_params_set_access(m_pcm, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED);

  snd_pcm_format_t fmt = AEFormatToALSAFormat(format.m_dataFormat);
  if (fmt == SND_PCM_FORMAT_UNKNOWN)
  {
      /* if we dont support the requested format, fallback to float */
      format.m_dataFormat = AE_FMT_FLOAT;
      fmt                 = SND_PCM_FORMAT_FLOAT;
  }

  /* try the data format */
  if (snd_pcm_hw_params_set_format(m_pcm, hw_params, fmt) < 0)
  {
    /* if the chosen format is not supported, try each one in decending order */
    CLog::Log(LOGERROR, "CAESinkALSA::InitializeHW - Your hardware does not support %s, trying other formats", CAEUtil::DataFormatToStr(format.m_dataFormat));
    for(int i = AE_FMT_MAX; i > AE_FMT_INVALID; --i)
    {
      fmt = AEFormatToALSAFormat((enum AEDataFormat)i);
      if (fmt == SND_PCM_FORMAT_UNKNOWN || snd_pcm_hw_params_set_format(m_pcm, hw_params, fmt) < 0)
      {
        fmt = SND_PCM_FORMAT_UNKNOWN;
        continue;
      }

      format.m_dataFormat = (enum AEDataFormat)i;
      CLog::Log(LOGINFO, "CAESinkALSA::InitializeHW - Using data format %s", CAEUtil::DataFormatToStr(format.m_dataFormat));
      break;
    }

    /* if we failed to find a valid output format */
    if (fmt == SND_PCM_FORMAT_UNKNOWN)
    {
      CLog::Log(LOGERROR, "CAESinkALSA::InitializeHW - Unable to find a suitable output format");
      snd_pcm_hw_params_free(hw_params);
      return false;
    }
  }

  unsigned int      sampleRate = format.m_sampleRate;
  snd_pcm_uframes_t periodSize = 512;
  unsigned int      periods    = 64;
  snd_pcm_uframes_t bufferSize;

  snd_pcm_hw_params_set_rate_near       (m_pcm, hw_params, &sampleRate          , NULL);
  snd_pcm_hw_params_set_channels        (m_pcm, hw_params, format.m_channelCount       );
  snd_pcm_hw_params_set_period_size_near(m_pcm, hw_params, &periodSize          , NULL);
  snd_pcm_hw_params_set_periods_near    (m_pcm, hw_params, &periods             , NULL);
  snd_pcm_hw_params_get_buffer_size     (hw_params, &bufferSize);

  /* set the parameters */
  if (snd_pcm_hw_params(m_pcm, hw_params) < 0)
  {
    CLog::Log(LOGERROR, "CAESinkALSA::InitializeHW - Failed to set the parameters");
    snd_pcm_hw_params_free(hw_params);
    return false;
  }

  /* set the format paremeters */
  format.m_sampleRate   = sampleRate;
  format.m_frames       = periods;
  format.m_frameSize    = periodSize;
  format.m_frameSamples = snd_pcm_bytes_to_frames(m_pcm, periodSize);

  snd_pcm_hw_params_free(hw_params);
  return true;
}

bool CAESinkALSA::InitializeSW(AEAudioFormat &format)
{
  snd_pcm_sw_params_t *sw_params;
  snd_pcm_uframes_t boundary;

  snd_pcm_sw_params_malloc(&sw_params);

  snd_pcm_sw_params_current                (m_pcm, sw_params);
  snd_pcm_sw_params_set_start_threshold    (m_pcm, sw_params, INT_MAX);
  snd_pcm_sw_params_set_silence_threshold  (m_pcm, sw_params, 0);
  snd_pcm_sw_params_get_boundary           (sw_params, &boundary);
  snd_pcm_sw_params_set_silence_size       (m_pcm, sw_params, boundary);

  if (snd_pcm_sw_params(m_pcm, sw_params) < 0)
  {
    CLog::Log(LOGERROR, "CAESinkALSA::InitializeSW - Failed to set the parameters");
    snd_pcm_sw_params_free(sw_params);
    return false;
  }

  return true;
}

void CAESinkALSA::Deinitialize()
{
  Stop();

  if (m_pcm)
  {
    snd_pcm_drop (m_pcm);
    snd_pcm_close(m_pcm);
  }

  delete[] m_buffer;
  m_buffer = NULL;
}

void CAESinkALSA::Run()
{
  CLog::Log(LOGDEBUG, "CAESinkALSA::Run - Thread Started");
  CSingleLock runLock(m_runLock);
  while(m_running)
  {
    CSingleLock bufferLock(m_bufferLock);
    if (m_bufferSamples == 0)
    {
      /* wait for a buffer */
      bufferLock.Leave();
      m_bufferWait.Wait();
      continue;
    }

    CLog::Log(LOGDEBUG, "CAESinkALSA::Run - Write");
    snd_pcm_writei(m_pcm, (void*)m_buffer, m_bufferSamples);
    m_bufferSamples = 0;
  }
  CLog::Log(LOGDEBUG, "CAESinkALSA::Run - Thread Stopped");
}

void CAESinkALSA::Stop()
{
  m_running = false;
  m_bufferWait.Post();
  CSingleLock runLock(m_runLock);
}

AEAudioFormat CAESinkALSA::GetAudioFormat()
{
  return m_format;
}

float CAESinkALSA::GetDelay()
{
  if (!m_pcm) return 0;

  snd_pcm_sframes_t frames = 0;
  snd_pcm_delay(m_pcm, &frames);

  return frames / m_format.m_sampleRate;
}

unsigned int CAESinkALSA::AddPackets(uint8_t *data, unsigned int samples)
{
  /* wait for a free buffer */ 
  CSingleLock bufferLock(m_bufferLock);

  /* fill the buffer */
  CLog::Log(LOGDEBUG, "Add %d\n", samples);
  memcpy(m_buffer, data, samples / m_format.m_channelCount);
  m_bufferSamples = samples;

  /* notify the thread */
  m_bufferWait.Post();
  return samples;
}

