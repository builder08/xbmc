/*
 *      Copyright (C) 2005-2008 Team XBMC
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

#include "DVDAudioCodecFFmpeg.h"
#ifdef _LINUX
#include "XMemUtils.h"
#endif
#include "../../DVDStreamInfo.h"
#include "utils/log.h"
#include "GUISettings.h"

CDVDAudioCodecFFmpeg::CDVDAudioCodecFFmpeg() : CDVDAudioCodec()
{
  m_iBufferSize1 = 0;
  m_pBuffer1     = (float*)_aligned_malloc(AVCODEC_MAX_AUDIO_FRAME_SIZE + FF_INPUT_BUFFER_PADDING_SIZE, 16);
  memset(m_pBuffer1, 0, AVCODEC_MAX_AUDIO_FRAME_SIZE + FF_INPUT_BUFFER_PADDING_SIZE);

  m_iBufferSize2 = 0;
  m_pBuffer2     = (float*)_aligned_malloc(AVCODEC_MAX_AUDIO_FRAME_SIZE + FF_INPUT_BUFFER_PADDING_SIZE, 16);
  memset(m_pBuffer2, 0, AVCODEC_MAX_AUDIO_FRAME_SIZE + FF_INPUT_BUFFER_PADDING_SIZE);

  m_iBuffered = 0;
  m_pCodecContext = NULL;
  m_pConvert = NULL;
  m_bOpenedCodec = false;
}

CDVDAudioCodecFFmpeg::~CDVDAudioCodecFFmpeg()
{
  _aligned_free(m_pBuffer1);
  _aligned_free(m_pBuffer2);
  Dispose();
}

bool CDVDAudioCodecFFmpeg::Open(CDVDStreamInfo &hints, CDVDCodecOptions &options)
{
  AVCodec* pCodec;
  m_bOpenedCodec = false;

  if (!m_dllAvUtil.Load() || !m_dllAvCodec.Load()) return false;

  m_pCodecContext = m_dllAvCodec.avcodec_alloc_context();
  m_dllAvCodec.avcodec_get_context_defaults(m_pCodecContext);

  pCodec = m_dllAvCodec.avcodec_find_decoder(hints.codec);
  if (!pCodec)
  {
    CLog::Log(LOGDEBUG,"CDVDAudioCodecFFmpeg::Open() Unable to find codec %d", hints.codec);
    return false;
  }

  m_pCodecContext->debug_mv = 0;
  m_pCodecContext->debug = 0;
  m_pCodecContext->workaround_bugs = 1;

  if (pCodec->capabilities & CODEC_CAP_TRUNCATED)
    m_pCodecContext->flags |= CODEC_FLAG_TRUNCATED;

  m_pCodecContext->channels = hints.channels;
  m_pCodecContext->sample_rate = hints.samplerate;
  m_pCodecContext->block_align = hints.blockalign;
  
  // TODO: Who should be setting this? It indicated the size of the data, not necessarily the valid bits
  m_pCodecContext->bits_per_coded_sample = hints.bitspersample;
  if(m_pCodecContext->bits_per_coded_sample == 0)
    m_pCodecContext->bits_per_coded_sample = 16; // TODO: Set this properly

/* for now, only set the requested layout for non-apple architecture */
#ifdef __APPLE__
  /* if we need to downmix, do it in ffmpeg as codecs are smarter then we can ever be */
  /* wmapro does not support this */
  if(hints.codec != CODEC_ID_WMAPRO && g_guiSettings.GetBool("audiooutput.downmixmultichannel"))
  {
    m_pCodecContext->request_channel_layout = CH_LAYOUT_STEREO;
    // below is required or center channel is missing with VC1 content under OSX.
    m_pCodecContext->request_channels = 2;
  }
#endif

  if( hints.extradata && hints.extrasize > 0 )
  {
    m_pCodecContext->extradata_size = hints.extrasize;
    m_pCodecContext->extradata = (uint8_t*)m_dllAvUtil.av_mallocz(hints.extrasize + FF_INPUT_BUFFER_PADDING_SIZE);
    memcpy(m_pCodecContext->extradata, hints.extradata, hints.extrasize);
  }

  // set acceleration
  m_pCodecContext->dsp_mask = FF_MM_FORCE | FF_MM_MMX | FF_MM_MMXEXT | FF_MM_SSE;

  if (m_dllAvCodec.avcodec_open(m_pCodecContext, pCodec) < 0)
  {
    CLog::Log(LOGDEBUG,"CDVDAudioCodecFFmpeg::Open() Unable to open codec");
    Dispose();
    return false;
  }

  m_iMapChannels = -1;
  m_bOpenedCodec = true;
  
  return true;
}

void CDVDAudioCodecFFmpeg::Dispose()
{
  if (m_pConvert)
  {
    m_dllAvCodec.av_audio_convert_free(m_pConvert);
    m_pConvert = NULL;
  }

  if (m_pCodecContext)
  {
    if (m_bOpenedCodec) m_dllAvCodec.avcodec_close(m_pCodecContext);
    m_bOpenedCodec = false;
    m_dllAvUtil.av_free(m_pCodecContext);
    m_pCodecContext = NULL;
  }

  m_dllAvCodec.Unload();
  m_dllAvUtil.Unload();

  m_iBufferSize1 = 0;
  m_iBufferSize2 = 0;
  m_iBuffered = 0;
}

int CDVDAudioCodecFFmpeg::Decode(BYTE* pData, int iSize)
{
  int iBytesUsed;
  if (!m_pCodecContext) return -1;

  m_iBufferSize1 = AVCODEC_MAX_AUDIO_FRAME_SIZE ;
  m_iBufferSize2 = 0;

  iBytesUsed = m_dllAvCodec.avcodec_decode_audio2( m_pCodecContext
                                                 , (int16_t*)m_pBuffer1
                                                 , &m_iBufferSize1
                                                 , pData
                                                 , iSize);

#if (LIBAVCODEC_VERSION_INT > AV_VERSION_INT(52, 48, 0))
  #warning "Make sure upstream version still needs this workaround (ffmpeg issue #1709)"
#endif
  /* upstream ac3dec is bugged, returns the packet size, not a negative value on error */
  if (m_pCodecContext->codec_id == CODEC_ID_AC3 && iBytesUsed > iSize)
  {
    m_iBufferSize1 = 0;
    return iSize;
  }

  if(m_iBufferSize1 == 0 && iBytesUsed >= 0)
    m_iBuffered += iBytesUsed;
  else
    m_iBuffered = 0;

  // If sample format providd by ffmpeg does not match our desired format, convert it
  if(m_pCodecContext->sample_fmt != SAMPLE_FMT_FLT && m_iBufferSize1 > 0)
  {
    // Create a new converter, if necessary
    if(!m_pConvert)
      m_pConvert = m_dllAvCodec.av_audio_convert_alloc(SAMPLE_FMT_FLT, 1, m_pCodecContext->sample_fmt, 1, NULL, 0);

    if(!m_pConvert)
    {
      CLog::Log(LOGERROR, "CDVDAudioCodecFFmpeg::Decode - Unable to create converter for %d to SAMPLE_FMT_FLT", m_pCodecContext->sample_fmt);
      m_iBufferSize1 = 0;
      m_iBufferSize2 = 0;
      return iBytesUsed;
    }

    const void *ibuf[MAX_DECODE_CHANS] = { m_pBuffer1 };
    void       *obuf[MAX_DECODE_CHANS] = { m_pBuffer2 };
    int         istr[MAX_DECODE_CHANS] = { m_dllAvCodec.av_get_bits_per_sample_format(m_pCodecContext->sample_fmt)/8 }; // Bytes per sample, in
    int         ostr[MAX_DECODE_CHANS] = { sizeof(float) }; // Bytes per sample, out
    int         len     = m_iBufferSize1 / istr[0]; // Number of samples
    if(m_dllAvCodec.av_audio_convert(m_pConvert, obuf, ostr, ibuf, istr, len) < 0)
    {
      CLog::Log(LOGERROR, "CDVDAudioCodecFFmpeg::Decode - Unable to convert %d to SAMPLE_FMT_FLT", (int)m_pCodecContext->sample_fmt);
      m_iBufferSize1 = 0;
      m_iBufferSize2 = 0;
      return iBytesUsed;
    }

    m_iBufferSize1 = 0;
    m_iBufferSize2 = len * ostr[0];
  }

  return iBytesUsed;
}

int CDVDAudioCodecFFmpeg::GetData(float** dst)
{
  if(m_iBufferSize1)
  {
    *dst = m_pBuffer1;
    return m_iBufferSize1;
  }
  else if(m_iBufferSize2)
  {
    *dst = m_pBuffer2;
    return m_iBufferSize2;
  }
  
  return 0;
}

void CDVDAudioCodecFFmpeg::Reset()
{
  if (m_pCodecContext) m_dllAvCodec.avcodec_flush_buffers(m_pCodecContext);
  m_iBufferSize1 = 0;
  m_iBufferSize2 = 0;
  m_iBuffered = 0;
}

int CDVDAudioCodecFFmpeg::GetChannels()
{
  if (m_pCodecContext) return m_pCodecContext->channels;
  return 0;
}

int CDVDAudioCodecFFmpeg::GetSampleRate()
{
  if (m_pCodecContext) return m_pCodecContext->sample_rate;
  return 0;
}

void CDVDAudioCodecFFmpeg::BuildChannelMap()
{
  int index = 0;
  if (m_pCodecContext->channel_layout & CH_FRONT_LEFT           ) m_channelMap[index++] = PCM_FRONT_LEFT           ;
  if (m_pCodecContext->channel_layout & CH_FRONT_RIGHT          ) m_channelMap[index++] = PCM_FRONT_RIGHT          ;
  if (m_pCodecContext->channel_layout & CH_FRONT_CENTER         ) m_channelMap[index++] = PCM_FRONT_CENTER         ;
  if (m_pCodecContext->channel_layout & CH_LOW_FREQUENCY        ) m_channelMap[index++] = PCM_LOW_FREQUENCY        ;
  if (m_pCodecContext->channel_layout & CH_BACK_LEFT            ) m_channelMap[index++] = PCM_BACK_LEFT            ;
  if (m_pCodecContext->channel_layout & CH_BACK_RIGHT           ) m_channelMap[index++] = PCM_BACK_RIGHT           ;
  if (m_pCodecContext->channel_layout & CH_FRONT_LEFT_OF_CENTER ) m_channelMap[index++] = PCM_FRONT_LEFT_OF_CENTER ;
  if (m_pCodecContext->channel_layout & CH_FRONT_RIGHT_OF_CENTER) m_channelMap[index++] = PCM_FRONT_RIGHT_OF_CENTER;
  if (m_pCodecContext->channel_layout & CH_BACK_CENTER          ) m_channelMap[index++] = PCM_BACK_CENTER          ;
  if (m_pCodecContext->channel_layout & CH_SIDE_LEFT            ) m_channelMap[index++] = PCM_SIDE_LEFT            ;
  if (m_pCodecContext->channel_layout & CH_SIDE_RIGHT           ) m_channelMap[index++] = PCM_SIDE_RIGHT           ;
  if (m_pCodecContext->channel_layout & CH_TOP_CENTER           ) m_channelMap[index++] = PCM_TOP_CENTER           ;
  if (m_pCodecContext->channel_layout & CH_TOP_FRONT_LEFT       ) m_channelMap[index++] = PCM_TOP_FRONT_LEFT       ;
  if (m_pCodecContext->channel_layout & CH_TOP_FRONT_CENTER     ) m_channelMap[index++] = PCM_TOP_FRONT_CENTER     ;
  if (m_pCodecContext->channel_layout & CH_TOP_FRONT_RIGHT      ) m_channelMap[index++] = PCM_TOP_FRONT_RIGHT      ;
  if (m_pCodecContext->channel_layout & CH_TOP_BACK_LEFT        ) m_channelMap[index++] = PCM_TOP_BACK_LEFT        ;
  if (m_pCodecContext->channel_layout & CH_TOP_BACK_CENTER      ) m_channelMap[index++] = PCM_TOP_BACK_CENTER      ;
  if (m_pCodecContext->channel_layout & CH_TOP_BACK_RIGHT       ) m_channelMap[index++] = PCM_TOP_BACK_RIGHT       ;

  /* if there is less channels in the map then advertised, we need to fix it */
  if (index < m_pCodecContext->channels)
  {
    CLog::Log(LOGINFO, "CDVDAudioCodecFFmpeg::GetChannelMap - FFmpeg did not report the channel layout properly, trying to guess");

    index = 0;
    switch(m_pCodecContext->codec_id)
    {
      case CODEC_ID_FLAC:
        switch(m_pCodecContext->channels)
        {
          case 1:
            m_channelMap[index++] = PCM_FRONT_CENTER;
            break;

          case 2:
            m_channelMap[index++] = PCM_FRONT_LEFT;
            m_channelMap[index++] = PCM_FRONT_RIGHT;
            break;

          case 3:
            m_channelMap[index++] = PCM_FRONT_LEFT;
            m_channelMap[index++] = PCM_FRONT_RIGHT;
            m_channelMap[index++] = PCM_FRONT_CENTER;
            break;

          case 4:
            m_channelMap[index++] = PCM_FRONT_LEFT;
            m_channelMap[index++] = PCM_FRONT_RIGHT;
            m_channelMap[index++] = PCM_BACK_LEFT;
            m_channelMap[index++] = PCM_BACK_RIGHT;
            break;

          case 5:
            m_channelMap[index++] = PCM_FRONT_LEFT;
            m_channelMap[index++] = PCM_FRONT_RIGHT;
            m_channelMap[index++] = PCM_FRONT_CENTER;
            m_channelMap[index++] = PCM_BACK_LEFT;
            m_channelMap[index++] = PCM_BACK_RIGHT;
            break;

          case 6:
            m_channelMap[index++] = PCM_FRONT_LEFT;
            m_channelMap[index++] = PCM_FRONT_RIGHT;
            m_channelMap[index++] = PCM_FRONT_CENTER;
            m_channelMap[index++] = PCM_LOW_FREQUENCY;
            m_channelMap[index++] = PCM_BACK_LEFT;
            m_channelMap[index++] = PCM_BACK_RIGHT;
            break;

          case 7:
            m_channelMap[index++] = PCM_FRONT_LEFT;
            m_channelMap[index++] = PCM_FRONT_RIGHT;
            m_channelMap[index++] = PCM_FRONT_CENTER;
            m_channelMap[index++] = PCM_BACK_LEFT;
            m_channelMap[index++] = PCM_BACK_RIGHT;
            m_channelMap[index++] = PCM_SIDE_LEFT;
            m_channelMap[index++] = PCM_SIDE_RIGHT;
            break;

          case 8:
            m_channelMap[index++] = PCM_FRONT_LEFT;
            m_channelMap[index++] = PCM_FRONT_RIGHT;
            m_channelMap[index++] = PCM_FRONT_CENTER;
            m_channelMap[index++] = PCM_LOW_FREQUENCY;
            m_channelMap[index++] = PCM_BACK_LEFT;
            m_channelMap[index++] = PCM_BACK_RIGHT;
            m_channelMap[index++] = PCM_SIDE_LEFT;
            m_channelMap[index++] = PCM_SIDE_RIGHT;
            break;
        }
        break;

      default:;
    }

    if (index == 0)
      CLog::Log(LOGERROR, "CDVDAudioCodecFFmpeg::GetChannelMap - Unable to guess a channel layout, please report this to XBMC and submit a sample file");
  }

  //terminate the channel map
  m_channelMap[index] = PCM_INVALID;
  m_iMapChannels = GetChannels();
}

enum PCMChannels* CDVDAudioCodecFFmpeg::GetChannelMap()
{
  if (m_iMapChannels != GetChannels())
    BuildChannelMap();
  if (m_channelMap[0] == PCM_INVALID)
    return NULL;
  return m_channelMap;
}
