/*
 *      Copyright (C) 2010 Alwin Esch (Team XBMC)
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

#include <stdint.h>
#include <limits.h>
#include <libavcodec/avcodec.h> // For codec id's
#include "VNSIDemux.h"
#include "responsepacket.h"
#include "requestpacket.h"
#include "vdrcommand.h"

cVNSIDemux::cVNSIDemux()
  : m_startup(false)
  , m_channel(0)
  , m_StatusCount(0)
{
  m_Streams.iStreamCount = 0;
}

cVNSIDemux::~cVNSIDemux()
{
  Close();
}

bool cVNSIDemux::Open(const PVR_CHANNEL &channelinfo)
{
  m_channel = channelinfo.iChannelNumber;

  if(!m_session.Open(g_szHostname, g_iPort, "XBMC Live stream receiver"))
    return false;

  cRequestPacket vrp;
  if (!vrp.init(VDR_CHANNELSTREAM_OPEN) ||
      !vrp.add_U32(m_channel) ||
      !m_session.ReadSuccess(&vrp))
  {
    XBMC->Log(LOG_ERROR, "cVNSIDemux::Open - Can't open channel %i - %s", m_channel, channelinfo.strChannelName);
    return false;
  }

  m_StatusCount = 0;
  m_startup = true;

  while (m_Streams.iStreamCount == 0 && m_StatusCount == 0)
  {
    DemuxPacket* pkg = Read();
    if(!pkg)
    {
      Close();
      return false;
    }
    PVR->FreeDemuxPacket(pkg);
  }

  return true;
}

void cVNSIDemux::Close()
{
  cRequestPacket vrp;
  m_session.Close();
}

bool cVNSIDemux::GetStreamProperties(PVR_STREAM_PROPERTIES* props)
{
  props->iStreamCount = m_Streams.iStreamCount;
  for (unsigned int i = 0; i < m_Streams.iStreamCount; i++)
  {
    props->stream[i].iStreamIndex           = m_Streams.stream[i].iStreamIndex;
    props->stream[i].iPhysicalId       = m_Streams.stream[i].iPhysicalId;
    props->stream[i].iCodecType   = m_Streams.stream[i].iCodecType;
    props->stream[i].iCodecId     = m_Streams.stream[i].iCodecId;
    props->stream[i].iHeight       = m_Streams.stream[i].iHeight;
    props->stream[i].iWidth        = m_Streams.stream[i].iWidth;
    props->stream[i].strLanguage[0]  = m_Streams.stream[i].strLanguage[0];
    props->stream[i].strLanguage[1]  = m_Streams.stream[i].strLanguage[1];
    props->stream[i].strLanguage[2]  = m_Streams.stream[i].strLanguage[2];
    props->stream[i].strLanguage[3]  = m_Streams.stream[i].strLanguage[3];
    props->stream[i].iIdentifier   = m_Streams.stream[i].iIdentifier;
  }
  return (props->iStreamCount > 0);
}

void cVNSIDemux::Abort()
{
  m_Streams.iStreamCount = 0;
  m_session.Abort();
}

DemuxPacket* cVNSIDemux::Read()
{
  cResponsePacket *resp = m_session.ReadMessage();

  if(resp == NULL)
  {
    return NULL;
  }

  if (resp->getChannelID() != CHANNEL_STREAM)
  {
    delete resp;
    return NULL;
  }

  if (resp->getOpCodeID() == VDR_STREAM_CHANGE)
  {
    StreamChange(resp);
    if (!m_startup)
    {
      DemuxPacket* pkt  = PVR->AllocateDemuxPacket(0);
      pkt->iStreamId    = DMX_SPECIALID_STREAMCHANGE;
      delete resp;
      return pkt;
    }
    else
      m_startup = false;
  }
  else if (resp->getOpCodeID() == VDR_STREAM_STATUS)
  {
    StreamStatus(resp);
  }
  else if (resp->getOpCodeID() == VDR_STREAM_SIGNALINFO)
  {
    StreamSignalInfo(resp);
  }
  else if (resp->getOpCodeID() == VDR_STREAM_CONTENTINFO)
  {
    StreamContentInfo(resp);
    DemuxPacket* pkt = PVR->AllocateDemuxPacket(sizeof(PVR_STREAM_PROPERTIES));
    memcpy(pkt->pData, &m_Streams, sizeof(PVR_STREAM_PROPERTIES));
    pkt->iStreamId  = DMX_SPECIALID_STREAMINFO;
    pkt->iSize      = sizeof(PVR_STREAM_PROPERTIES);
    delete resp;
    return pkt;
  }
  else if (resp->getOpCodeID() == VDR_STREAM_MUXPKT)
  {
    DemuxPacket* p = (DemuxPacket*)resp->getUserData();

    p->iSize      = resp->getUserDataLength();
    p->duration   = (double)resp->getDuration() * DVD_TIME_BASE / 1000000;
    p->dts        = (double)resp->getDTS() * DVD_TIME_BASE / 1000000;
    p->pts        = (double)resp->getPTS() * DVD_TIME_BASE / 1000000;
    p->iStreamId  = -1;
    for(unsigned int i = 0; i < m_Streams.iStreamCount; i++)
    {
      if(m_Streams.stream[i].iPhysicalId == (unsigned int)resp->getStreamID())
      {
            p->iStreamId = i;
            break;
      }
    }
    delete resp;
    return p;
  }

  delete resp;
  return PVR->AllocateDemuxPacket(0);
}

bool cVNSIDemux::SwitchChannel(const PVR_CHANNEL &channelinfo)
{
  XBMC->Log(LOG_DEBUG, "changing to channel %d", channelinfo.iChannelNumber);

  cRequestPacket vrp;
  if (!vrp.init(VDR_CHANNELSTREAM_OPEN) || !vrp.add_U32(channelinfo.iChannelNumber) || !m_session.ReadSuccess(&vrp))
  {
    XBMC->Log(LOG_ERROR, "cVNSIDemux::SetChannel - failed to set channel");
  }
  else
  {
    m_channel           = channelinfo.iChannelNumber;
    m_Streams.iStreamCount  = 0;
    m_startup           = true;
    while (m_Streams.iStreamCount == 0 && m_StatusCount == 0)
    {
      DemuxPacket* pkg = Read();
      if(!pkg)
        return false;
      PVR->FreeDemuxPacket(pkg);
    }
    return true;
  }
  return false;
}

bool cVNSIDemux::GetSignalStatus(PVR_SIGNAL_STATUS &qualityinfo)
{
  if (m_Quality.fe_name.empty())
    return false;

  strncpy(qualityinfo.strAdapterName, m_Quality.fe_name.c_str(), sizeof(qualityinfo.strAdapterName));
  strncpy(qualityinfo.strAdapterStatus, m_Quality.fe_status.c_str(), sizeof(qualityinfo.strAdapterStatus));
  qualityinfo.iSignal = (uint16_t)m_Quality.fe_signal;
  qualityinfo.iSNR = (uint16_t)m_Quality.fe_snr;
  qualityinfo.iBER = (uint32_t)m_Quality.fe_ber;
  qualityinfo.iUNC = (uint32_t)m_Quality.fe_unc;
  qualityinfo.dVideoBitrate = 0;
  qualityinfo.dAudioBitrate = 0;
  qualityinfo.dDolbyBitrate = 0;

  return true;
}

void cVNSIDemux::StreamChange(cResponsePacket *resp)
{
  m_Streams.iStreamCount = 0;

  while (!resp->end())
  {
    uint32_t    index = resp->extract_U32();
    const char* type  = resp->extract_String();

    m_Streams.stream[m_Streams.iStreamCount].iFPSScale         = 0;
    m_Streams.stream[m_Streams.iStreamCount].iFPSRate          = 0;
    m_Streams.stream[m_Streams.iStreamCount].iHeight           = 0;
    m_Streams.stream[m_Streams.iStreamCount].iWidth            = 0;
    m_Streams.stream[m_Streams.iStreamCount].fAspect           = 0.0;

    m_Streams.stream[m_Streams.iStreamCount].iChannels         = 0;
    m_Streams.stream[m_Streams.iStreamCount].iSampleRate       = 0;
    m_Streams.stream[m_Streams.iStreamCount].iBlockAlign       = 0;
    m_Streams.stream[m_Streams.iStreamCount].iBitRate          = 0;
    m_Streams.stream[m_Streams.iStreamCount].iBitsPerSample  = 0;

    if(!strcmp(type, "AC3"))
    {
      const char *language = resp->extract_String();

      m_Streams.stream[m_Streams.iStreamCount].iStreamIndex         = m_Streams.iStreamCount;
      m_Streams.stream[m_Streams.iStreamCount].iPhysicalId     = index;
      m_Streams.stream[m_Streams.iStreamCount].iCodecType = CODEC_TYPE_AUDIO;
      m_Streams.stream[m_Streams.iStreamCount].iCodecId   = CODEC_ID_AC3;
      m_Streams.stream[m_Streams.iStreamCount].strLanguage[0]= language[0];
      m_Streams.stream[m_Streams.iStreamCount].strLanguage[1]= language[1];
      m_Streams.stream[m_Streams.iStreamCount].strLanguage[2]= language[2];
      m_Streams.stream[m_Streams.iStreamCount].strLanguage[3]= 0;
      m_Streams.stream[m_Streams.iStreamCount].iIdentifier = -1;
      m_Streams.iStreamCount++;
    }
    else if(!strcmp(type, "MPEG2AUDIO"))
    {
      const char *language = resp->extract_String();

      m_Streams.stream[m_Streams.iStreamCount].iStreamIndex         = m_Streams.iStreamCount;
      m_Streams.stream[m_Streams.iStreamCount].iPhysicalId     = index;
      m_Streams.stream[m_Streams.iStreamCount].iCodecType = CODEC_TYPE_AUDIO;
      m_Streams.stream[m_Streams.iStreamCount].iCodecId   = CODEC_ID_MP2;
      m_Streams.stream[m_Streams.iStreamCount].strLanguage[0]= language[0];
      m_Streams.stream[m_Streams.iStreamCount].strLanguage[1]= language[1];
      m_Streams.stream[m_Streams.iStreamCount].strLanguage[2]= language[2];
      m_Streams.stream[m_Streams.iStreamCount].strLanguage[3]= 0;
      m_Streams.stream[m_Streams.iStreamCount].iIdentifier = -1;
      m_Streams.iStreamCount++;
    }
    else if(!strcmp(type, "AAC"))
    {
      const char *language = resp->extract_String();

      m_Streams.stream[m_Streams.iStreamCount].iStreamIndex         = m_Streams.iStreamCount;
      m_Streams.stream[m_Streams.iStreamCount].iPhysicalId     = index;
      m_Streams.stream[m_Streams.iStreamCount].iCodecType = CODEC_TYPE_AUDIO;
      m_Streams.stream[m_Streams.iStreamCount].iCodecId   = CODEC_ID_AAC;
      m_Streams.stream[m_Streams.iStreamCount].strLanguage[0]= language[0];
      m_Streams.stream[m_Streams.iStreamCount].strLanguage[1]= language[1];
      m_Streams.stream[m_Streams.iStreamCount].strLanguage[2]= language[2];
      m_Streams.stream[m_Streams.iStreamCount].strLanguage[3]= 0;
      m_Streams.stream[m_Streams.iStreamCount].iIdentifier = -1;
      m_Streams.iStreamCount++;
    }
    else if(!strcmp(type, "DTS"))
    {
      const char *language = resp->extract_String();

      m_Streams.stream[m_Streams.iStreamCount].iStreamIndex         = m_Streams.iStreamCount;
      m_Streams.stream[m_Streams.iStreamCount].iPhysicalId     = index;
      m_Streams.stream[m_Streams.iStreamCount].iCodecType = CODEC_TYPE_AUDIO;
      m_Streams.stream[m_Streams.iStreamCount].iCodecId   = CODEC_ID_DTS;
      m_Streams.stream[m_Streams.iStreamCount].strLanguage[0]= language[0];
      m_Streams.stream[m_Streams.iStreamCount].strLanguage[1]= language[1];
      m_Streams.stream[m_Streams.iStreamCount].strLanguage[2]= language[2];
      m_Streams.stream[m_Streams.iStreamCount].strLanguage[3]= 0;
      m_Streams.stream[m_Streams.iStreamCount].iIdentifier = -1;
      m_Streams.iStreamCount++;
    }
    else if(!strcmp(type, "EAC3"))
    {
      const char *language = resp->extract_String();

      m_Streams.stream[m_Streams.iStreamCount].iStreamIndex         = m_Streams.iStreamCount;
      m_Streams.stream[m_Streams.iStreamCount].iPhysicalId     = index;
      m_Streams.stream[m_Streams.iStreamCount].iCodecType = CODEC_TYPE_AUDIO;
      m_Streams.stream[m_Streams.iStreamCount].iCodecId   = CODEC_ID_EAC3;
      m_Streams.stream[m_Streams.iStreamCount].strLanguage[0]= language[0];
      m_Streams.stream[m_Streams.iStreamCount].strLanguage[1]= language[1];
      m_Streams.stream[m_Streams.iStreamCount].strLanguage[2]= language[2];
      m_Streams.stream[m_Streams.iStreamCount].strLanguage[3]= 0;
      m_Streams.stream[m_Streams.iStreamCount].iIdentifier = -1;
      m_Streams.iStreamCount++;
    }
    else if(!strcmp(type, "MPEG2VIDEO"))
    {
      m_Streams.stream[m_Streams.iStreamCount].iStreamIndex         = m_Streams.iStreamCount;
      m_Streams.stream[m_Streams.iStreamCount].iPhysicalId     = index;
      m_Streams.stream[m_Streams.iStreamCount].iCodecType = CODEC_TYPE_VIDEO;
      m_Streams.stream[m_Streams.iStreamCount].iCodecId   = CODEC_ID_MPEG2VIDEO;
      m_Streams.stream[m_Streams.iStreamCount].iFPSScale   = resp->extract_U32();
      m_Streams.stream[m_Streams.iStreamCount].iFPSRate    = resp->extract_U32();
      m_Streams.stream[m_Streams.iStreamCount].iHeight     = resp->extract_U32();
      m_Streams.stream[m_Streams.iStreamCount].iWidth      = resp->extract_U32();
      m_Streams.stream[m_Streams.iStreamCount].fAspect     = resp->extract_Double();
      m_Streams.stream[m_Streams.iStreamCount].strLanguage[0]= 0;
      m_Streams.stream[m_Streams.iStreamCount].strLanguage[1]= 0;
      m_Streams.stream[m_Streams.iStreamCount].strLanguage[2]= 0;
      m_Streams.stream[m_Streams.iStreamCount].strLanguage[3]= 0;
      m_Streams.stream[m_Streams.iStreamCount].iIdentifier = -1;
      m_Streams.iStreamCount++;
    }
    else if(!strcmp(type, "H264"))
    {
      m_Streams.stream[m_Streams.iStreamCount].iStreamIndex         = m_Streams.iStreamCount;
      m_Streams.stream[m_Streams.iStreamCount].iPhysicalId     = index;
      m_Streams.stream[m_Streams.iStreamCount].iCodecType = CODEC_TYPE_VIDEO;
      m_Streams.stream[m_Streams.iStreamCount].iCodecId   = CODEC_ID_H264;
      m_Streams.stream[m_Streams.iStreamCount].iFPSScale   = resp->extract_U32();
      m_Streams.stream[m_Streams.iStreamCount].iFPSRate    = resp->extract_U32();
      m_Streams.stream[m_Streams.iStreamCount].iHeight     = resp->extract_U32();
      m_Streams.stream[m_Streams.iStreamCount].iWidth      = resp->extract_U32();
      m_Streams.stream[m_Streams.iStreamCount].fAspect     = resp->extract_Double();
      m_Streams.stream[m_Streams.iStreamCount].strLanguage[0]= 0;
      m_Streams.stream[m_Streams.iStreamCount].strLanguage[1]= 0;
      m_Streams.stream[m_Streams.iStreamCount].strLanguage[2]= 0;
      m_Streams.stream[m_Streams.iStreamCount].strLanguage[3]= 0;
      m_Streams.stream[m_Streams.iStreamCount].iIdentifier = -1;
      m_Streams.iStreamCount++;
    }
    else if(!strcmp(type, "DVBSUB"))
    {
      const char *language    = resp->extract_String();
      uint32_t composition_id = resp->extract_U32();
      uint32_t ancillary_id   = resp->extract_U32();

      m_Streams.stream[m_Streams.iStreamCount].iStreamIndex         = m_Streams.iStreamCount;
      m_Streams.stream[m_Streams.iStreamCount].iPhysicalId     = index;
      m_Streams.stream[m_Streams.iStreamCount].iCodecType = CODEC_TYPE_SUBTITLE;
      m_Streams.stream[m_Streams.iStreamCount].iCodecId   = CODEC_ID_DVB_SUBTITLE;
      m_Streams.stream[m_Streams.iStreamCount].strLanguage[0]= language[0];
      m_Streams.stream[m_Streams.iStreamCount].strLanguage[1]= language[1];
      m_Streams.stream[m_Streams.iStreamCount].strLanguage[2]= language[2];
      m_Streams.stream[m_Streams.iStreamCount].strLanguage[3]= 0;
      m_Streams.stream[m_Streams.iStreamCount].iIdentifier = (composition_id & 0xffff) | ((ancillary_id & 0xffff) << 16);
      m_Streams.iStreamCount++;
    }
    else if(!strcmp(type, "TEXTSUB"))
    {
      const char *language = resp->extract_String();

      m_Streams.stream[m_Streams.iStreamCount].iStreamIndex         = m_Streams.iStreamCount;
      m_Streams.stream[m_Streams.iStreamCount].iPhysicalId     = index;
      m_Streams.stream[m_Streams.iStreamCount].iCodecType = CODEC_TYPE_SUBTITLE;
      m_Streams.stream[m_Streams.iStreamCount].iCodecId   = CODEC_ID_TEXT;
      m_Streams.stream[m_Streams.iStreamCount].strLanguage[0]= language[0];
      m_Streams.stream[m_Streams.iStreamCount].strLanguage[1]= language[1];
      m_Streams.stream[m_Streams.iStreamCount].strLanguage[2]= language[2];
      m_Streams.stream[m_Streams.iStreamCount].strLanguage[3]= 0;
      m_Streams.stream[m_Streams.iStreamCount].iIdentifier = -1;
      m_Streams.iStreamCount++;
    }
    else if(!strcmp(type, "TELETEXT"))
    {
      m_Streams.stream[m_Streams.iStreamCount].iStreamIndex         = m_Streams.iStreamCount;
      m_Streams.stream[m_Streams.iStreamCount].iPhysicalId     = index;
      m_Streams.stream[m_Streams.iStreamCount].iCodecType = CODEC_TYPE_SUBTITLE;
      m_Streams.stream[m_Streams.iStreamCount].iCodecId   = CODEC_ID_DVB_TELETEXT;
      m_Streams.stream[m_Streams.iStreamCount].strLanguage[0]= 0;
      m_Streams.stream[m_Streams.iStreamCount].strLanguage[1]= 0;
      m_Streams.stream[m_Streams.iStreamCount].strLanguage[2]= 0;
      m_Streams.stream[m_Streams.iStreamCount].strLanguage[3]= 0;
      m_Streams.stream[m_Streams.iStreamCount].iIdentifier = -1;
      m_Streams.iStreamCount++;
    }

    if (m_Streams.iStreamCount >= PVR_STREAM_MAX_STREAMS)
    {
      XBMC->Log(LOG_ERROR, "cVNSIDemux::StreamChange - max amount of streams reached");
      break;
    }
  }
}

void cVNSIDemux::StreamStatus(cResponsePacket *resp)
{
  const char* status = resp->extract_String();
  if(status == NULL)
    m_Status = "";
  else
  {
    m_StatusCount++;
    m_Status = status;
    XBMC->Log(LOG_DEBUG, "cVNSIDemux::StreamStatus - %s", status);
    XBMC->QueueNotification(QUEUE_INFO, status);
  }
}

void cVNSIDemux::StreamSignalInfo(cResponsePacket *resp)
{
  m_Quality.fe_name   = resp->extract_String();
  m_Quality.fe_status = resp->extract_String();
  m_Quality.fe_snr    = resp->extract_U32();
  m_Quality.fe_signal = resp->extract_U32();
  m_Quality.fe_ber    = resp->extract_U32();
  m_Quality.fe_unc    = resp->extract_U32();
}

void cVNSIDemux::StreamContentInfo(cResponsePacket *resp)
{
  for (unsigned int i = 0; i < m_Streams.iStreamCount && !resp->end(); i++)
  {
    int32_t index = resp->extract_U32();
    if (index == m_Streams.stream[i].iPhysicalId)
    {
      if (m_Streams.stream[i].iCodecType == CODEC_TYPE_AUDIO)
      {
        const char *language = resp->extract_String();
        m_Streams.stream[i].iChannels          = resp->extract_U32();
        m_Streams.stream[i].iSampleRate        = resp->extract_U32();
        m_Streams.stream[i].iBlockAlign        = resp->extract_U32();
        m_Streams.stream[i].iBitRate           = resp->extract_U32();
        m_Streams.stream[i].iBitsPerSample   = resp->extract_U32();
        m_Streams.stream[i].strLanguage[0]       = language[0];
        m_Streams.stream[i].strLanguage[1]       = language[1];
        m_Streams.stream[i].strLanguage[2]       = language[2];
        m_Streams.stream[i].strLanguage[3]       = 0;
      }
      else if (m_Streams.stream[i].iCodecType == CODEC_TYPE_VIDEO)
      {
        m_Streams.stream[i].iFPSScale         = resp->extract_U32();
        m_Streams.stream[i].iFPSRate          = resp->extract_U32();
        m_Streams.stream[i].iHeight           = resp->extract_U32();
        m_Streams.stream[i].iWidth            = resp->extract_U32();
        m_Streams.stream[i].fAspect           = resp->extract_Double();
      }
      else if (m_Streams.stream[i].iCodecType == CODEC_TYPE_SUBTITLE)
      {
        const char *language    = resp->extract_String();
        uint32_t composition_id = resp->extract_U32();
        uint32_t ancillary_id   = resp->extract_U32();
        m_Streams.stream[i].iIdentifier = (composition_id & 0xffff) | ((ancillary_id & 0xffff) << 16);
        m_Streams.stream[i].strLanguage[0]= language[0];
        m_Streams.stream[i].strLanguage[1]= language[1];
        m_Streams.stream[i].strLanguage[2]= language[2];
        m_Streams.stream[i].strLanguage[3]= 0;
      }
    }
  }
}
