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

#include "stdafx.h"
#include "DVDPlayerCodec.h"
#include "../../Util.h"

#include "DVDInputStreams/DVDFactoryInputStream.h"
#include "DVDDemuxers/DVDFactoryDemuxer.h"
#include "DVDDemuxers/DVDDemuxUtils.h"
#include "DVDStreamInfo.h"
#include "DVDCodecs/DVDFactoryCodec.h"

#include "AudioDecoder.h"

DVDPlayerCodec::DVDPlayerCodec()
{
  m_CodecName = "DVDPlayer";
  m_pDemuxer = NULL;
  m_pInputStream = NULL;
  m_pAudioCodec = NULL;
  m_audioPos = 0;
  m_pPacket = NULL;
  m_decoded = NULL;;
  m_nDecodedLen = 0;
}

DVDPlayerCodec::~DVDPlayerCodec()
{
  DeInit();
}

void DVDPlayerCodec::SetContentType(const CStdString &strContent)
{
  m_strContentType = strContent;
}

bool DVDPlayerCodec::Init(const CStdString &strFile, unsigned int filecache)
{
  m_decoded = NULL;;
  m_nDecodedLen = 0;

  CStdString strFileToOpen = strFile;

  CURL urlFile(strFile);
  if (urlFile.GetProtocol() == "shout" )
    strFileToOpen.Replace("shout://","http://");

  m_pInputStream = CDVDFactoryInputStream::CreateInputStream(NULL, strFileToOpen, m_strContentType);
  if (!m_pInputStream)
  {
    CLog::Log(LOGERROR, "%s: Error creating input stream for %s", __FUNCTION__, strFileToOpen.c_str());
    return false;
  }

  if (!m_pInputStream->Open(strFileToOpen.c_str(), m_strContentType))
  {
    CLog::Log(LOGERROR, "%s: Error opening file %s", __FUNCTION__, strFileToOpen.c_str());
    if (m_pInputStream)
      delete m_pInputStream;
    m_pInputStream = NULL;
    return false;
  }

  m_pDemuxer = NULL;

  try
  {
    m_pDemuxer = CDVDFactoryDemuxer::CreateDemuxer(m_pInputStream);
    if (!m_pDemuxer)
    {
      delete m_pInputStream;
      m_pInputStream = NULL;
      CLog::Log(LOGERROR, "%s: Error creating demuxer", __FUNCTION__);
      return false;
    }
  }
  catch(...)
  {
    CLog::Log(LOGERROR, "%s: Exception thrown when opeing demuxer", __FUNCTION__);
    if (m_pDemuxer)
    {
      delete m_pDemuxer;
      m_pDemuxer = NULL;
    }
    delete m_pInputStream;
    m_pInputStream = NULL;
    return false;
  }

  CDemuxStream* pStream = NULL;
  m_nAudioStream = -1;
  for (int i = 0; i < m_pDemuxer->GetNrOfStreams(); i++)
  {
    pStream = m_pDemuxer->GetStream(i);
    if (pStream && pStream->type == STREAM_AUDIO)
    {
      m_nAudioStream = i;
      break;
    }
  }

  if (m_nAudioStream == -1)
  {
    CLog::Log(LOGERROR, "%s: Could not find audio stream", __FUNCTION__);
    delete m_pDemuxer;
    m_pDemuxer = NULL;
    delete m_pInputStream;
    m_pInputStream = NULL;
    return false;
  }

  CDVDStreamInfo hint(*pStream, true);
  m_pAudioCodec = CDVDFactoryCodec::CreateAudioCodec(hint);
  if (!m_pAudioCodec)
  {
    CLog::Log(LOGERROR, "%s: Could not create audio codec", __FUNCTION__);
    delete m_pDemuxer;
    m_pDemuxer = NULL;
    delete m_pInputStream;
    m_pInputStream = NULL;
    return false;
  }

  // we have to decode initial data in order to get channels/samplerate
  // for sanity - we read no more than 10 packets
  for (int nPacket=0; nPacket < 10 && (m_Channels == 0 || m_SampleRate == 0); nPacket++)
  {
    BYTE dummy[256];
    int nSize = 256;
    ReadPCM(dummy, nSize, &nSize);

    // We always ask ffmpeg to return s16le
    m_BitsPerSample = m_pAudioCodec->GetBitsPerSample();
    m_SampleRate = m_pAudioCodec->GetSampleRate();
    m_Channels = m_pAudioCodec->GetChannels();

  }

  m_nDecodedLen = 0;

  if (m_Channels == 0) // no data - just guess and hope for the best
    m_Channels = 2;

  if (m_SampleRate == 0)
    m_SampleRate = 44100;

  m_TotalTime = m_pDemuxer->GetStreamLength();

  return true;
}

void DVDPlayerCodec::DeInit()
{
  if (m_pPacket)
    CDVDDemuxUtils::FreeDemuxPacket(m_pPacket);
  m_pPacket = NULL;

  if (m_pDemuxer != NULL)
  {
    delete m_pDemuxer;
    m_pDemuxer = NULL;
  }

  if (m_pInputStream != NULL)
  {
    delete m_pInputStream;
    m_pInputStream = NULL;
  }

  if (m_pAudioCodec != NULL)
  {
    delete m_pAudioCodec;
    m_pAudioCodec = NULL;
  }

  m_audioPos = 0;
  m_decoded = NULL;;
  m_nDecodedLen = 0;
}

__int64 DVDPlayerCodec::Seek(__int64 iSeekTime)
{
  if (m_pPacket)
    CDVDDemuxUtils::FreeDemuxPacket(m_pPacket);
  m_pPacket = NULL;

  m_pDemuxer->SeekTime((int)iSeekTime, true);

  m_decoded = NULL;;
  m_nDecodedLen = 0;

  return iSeekTime;
}

int DVDPlayerCodec::ReadPCM(BYTE *pBuffer, int size, int *actualsize)
{
  if (m_decoded && m_nDecodedLen > 0)
  {
    int nLen = (size<m_nDecodedLen)?size:m_nDecodedLen;
    *actualsize = nLen;
    memcpy(pBuffer, m_decoded, *actualsize);
    m_nDecodedLen -= nLen;
    m_decoded += (*actualsize);
    return READ_SUCCESS;
  }

  m_decoded = NULL;
  m_nDecodedLen = 0;

  if (m_pPacket && m_audioPos >= m_pPacket->iSize)
  {
    CDVDDemuxUtils::FreeDemuxPacket(m_pPacket);
    m_audioPos = 0;
    m_pPacket = NULL;
  }

  if (m_pPacket == NULL)
  {
    do
    {
      m_pPacket = m_pDemuxer->Read();
    } while (m_pPacket && m_pPacket->iStreamId != m_nAudioStream);

    if (!m_pPacket)
    {
      return READ_EOF;
    }
    m_audioPos = 0;
  }

  int decodeLen = m_pAudioCodec->Decode(m_pPacket->pData + m_audioPos, m_pPacket->iSize - m_audioPos);

  if (decodeLen < 0)
  {
    CDVDDemuxUtils::FreeDemuxPacket(m_pPacket);
    m_pPacket = NULL;
    m_audioPos = 0;
    return READ_ERROR;
  }

  m_audioPos += decodeLen;

  m_nDecodedLen = m_pAudioCodec->GetData(&m_decoded);

  *actualsize = (m_nDecodedLen <= size) ? m_nDecodedLen : size;
  if (*actualsize > 0)
  {
    memcpy(pBuffer, m_decoded, *actualsize);
    m_nDecodedLen -= *actualsize;
    m_decoded += (*actualsize);
  }

  return READ_SUCCESS;
}

bool DVDPlayerCodec::CanInit()
{
  return true;
}

bool DVDPlayerCodec::CanSeek()
{
  return true;
}
