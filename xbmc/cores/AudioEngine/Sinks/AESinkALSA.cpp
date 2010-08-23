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

static enum AEChannel ALSAChannelMap[9] =
  {AE_CH_FL, AE_CH_FR, AE_CH_BL, AE_CH_BR, AE_CH_FC, AE_CH_LFE, AE_CH_SL, AE_CH_SR, AE_CH_NULL};

CAESinkALSA::CAESinkALSA() :
  m_channelLayout(NULL ),
  m_pcm          (NULL )
{
  /* ensure that ALSA has been initialized */
  if(!snd_config)
    snd_config_update();
}

CAESinkALSA::~CAESinkALSA()
{
  Deinitialize();
}

inline unsigned int CAESinkALSA::GetChannelCount(const AEAudioFormat format)
{
  if (format.m_dataFormat == AE_FMT_IEC958)
    return 2;
  else
  {
    unsigned int out = 0;
    for(unsigned int c = 0; c < 8; ++c)
      for(unsigned int i = 0; format.m_channelLayout[i] != AE_CH_NULL; ++i)
        if (format.m_channelLayout[i] == ALSAChannelMap[c])
        {
          out = c + 1;
          break;
        }

    return out;
  }
}

inline CStdString CAESinkALSA::GetDeviceUse(const AEAudioFormat format, CStdString device)
{
  if (format.m_dataFormat == AE_FMT_IEC958)
  {
    if (device == "default")
      device = "iec958";

    if (device == "iec958")
    {
      device += ":AES0=0x6,AES1=0x82,AES2=0x0";
           if (format.m_sampleRate == 192000) device += ",AES3=0xe";
      else if (format.m_sampleRate == 176400) device += ",AES3=0xc";
      else if (format.m_sampleRate ==  96000) device += ",AES3=0xa";
      else if (format.m_sampleRate ==  88200) device += ",AES3=0x8";
      else if (format.m_sampleRate ==  48000) device += ",AES3=0x2";
      else if (format.m_sampleRate ==  44100) device += ",AES3=0x0";
      else if (format.m_sampleRate ==  32000) device += ",AES3=0x3";
      else device += ",AES3=0x1";
    }
    return device;
  }

  if (device == "default")
    switch(format.m_channelCount)
    {
      case 8: device = "plug:surround71"; break;
      case 6: device = "plug:surround51"; break;
      case 5: device = "plug:surround50"; break;
      case 4: device = "plug:surround40"; break;
    }

  return device;
}

bool CAESinkALSA::Initialize(AEAudioFormat &format, CStdString &device)
{
  format.m_channelCount = GetChannelCount(format);
  if (format.m_channelCount == 0)
  {
    CLog::Log(LOGERROR, "CAESinkALSA::Initialize - Unable to open the requested channel layout");
    return false;
  }

  m_channelLayout = new enum AEChannel[format.m_channelCount + 1];
  memcpy(m_channelLayout, ALSAChannelMap, format.m_channelCount * sizeof(enum AEChannel));
  m_channelLayout[format.m_channelCount] = AE_CH_NULL;

  /* set the channelLayout and the output device */
  format.m_channelLayout = m_channelLayout;
  m_device = device      = GetDeviceUse(format, device);

  /* get the sound config */
  snd_config_t *config;
  snd_config_copy(&config, snd_config);
  int error;

  error = snd_pcm_open_lconf(&m_pcm, device.c_str(), SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK, config);
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

  m_bufferSamples = 0;
  snd_pcm_prepare(m_pcm);

  m_format  = format;
  return true;
}

bool CAESinkALSA::IsCompatible(const AEAudioFormat format, const CStdString device)
{
  AEAudioFormat tmp  = format;
  tmp.m_channelCount = GetChannelCount(format);
  return (
    tmp.m_sampleRate   == m_format.m_sampleRate   &&
    tmp.m_dataFormat   == m_format.m_dataFormat   &&
    tmp.m_channelCount == m_format.m_channelCount &&
    GetDeviceUse(tmp, device) == m_device
  );
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
  unsigned int      frames     = 32;
  snd_pcm_uframes_t frameSize  = ((CAEUtil::DataFormatToBits(format.m_dataFormat) >> 3) * format.m_channelCount) * frames;

  snd_pcm_hw_params_set_rate_near       (m_pcm, hw_params, &sampleRate          , NULL);
  snd_pcm_hw_params_set_channels        (m_pcm, hw_params, format.m_channelCount      );
  snd_pcm_hw_params_set_period_size_near(m_pcm, hw_params, &frameSize           , NULL);
  snd_pcm_hw_params_set_periods_near    (m_pcm, hw_params, &frames              , NULL);

  /* set the parameters */
  if (snd_pcm_hw_params(m_pcm, hw_params) < 0)
  {
    CLog::Log(LOGERROR, "CAESinkALSA::InitializeHW - Failed to set the parameters");
    snd_pcm_hw_params_free(hw_params);
    return false;
  }

  /* set the format paremeters */
  format.m_sampleRate   = sampleRate;
  format.m_frames       = frames;
  format.m_frameSize    = snd_pcm_frames_to_bytes(m_pcm, 1);
  format.m_frameSamples = frames * format.m_channelCount;

  snd_pcm_hw_params_free(hw_params);
  return true;
}

bool CAESinkALSA::InitializeSW(AEAudioFormat &format)
{
  snd_pcm_sw_params_t *sw_params;
  snd_pcm_uframes_t boundary;

  snd_pcm_sw_params_malloc(&sw_params);

  snd_pcm_sw_params_current              (m_pcm, sw_params);
  snd_pcm_sw_params_set_start_threshold  (m_pcm, sw_params, INT_MAX);
  snd_pcm_sw_params_set_silence_threshold(m_pcm, sw_params, 0);
  snd_pcm_sw_params_get_boundary         (sw_params, &boundary);
  snd_pcm_sw_params_set_silence_size     (m_pcm, sw_params, boundary);

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
    m_pcm = NULL;
  }

  delete[] m_channelLayout;
  m_channelLayout = NULL;
}

void CAESinkALSA::Stop()
{
  CSingleLock runLock(m_runLock);
  if (!m_pcm) return;
  snd_pcm_drop(m_pcm);
}

float CAESinkALSA::GetDelay()
{
  if (!m_pcm) return 0;
  snd_pcm_sframes_t frames = 0;
  snd_pcm_delay(m_pcm, &frames);
  float delay = (float)(frames + (m_bufferSamples / m_format.m_channelCount)) / m_format.m_sampleRate;
  return delay;
}

unsigned int CAESinkALSA::AddPackets(uint8_t *data, unsigned int samples)
{
  CSingleLock runLock(m_runLock);
  if(snd_pcm_state(m_pcm) == SND_PCM_STATE_PREPARED)
    snd_pcm_start(m_pcm);

  m_bufferSamples = samples;
  snd_pcm_wait(m_pcm, 10);
  if (snd_pcm_writei(m_pcm, (void*)data, samples) == -EPIPE)
  {
    printf("Underrun\n");
    snd_pcm_prepare(m_pcm);
  }
  m_bufferSamples = 0;

  return samples;
}

