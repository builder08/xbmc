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

#include "AudioDecoder.h"
#include "CodecFactory.h"
#include "GUISettings.h"
#include "FileItem.h"
#include "MusicInfoTag.h"
#include "utils/SingleLock.h"
#include "utils/log.h"
#include <math.h>

#define INTERNAL_BUFFER_LENGTH  sizeof(float)*2*44100       // float samples, 2 channels, 44100 samples per sec = 1 second

CAudioDecoder::CAudioDecoder()
{
  m_codec = NULL;

  m_eof = false;

  m_status = STATUS_NO_FILE;
  m_canPlay = false;

  m_blockSize = 4;
}

CAudioDecoder::~CAudioDecoder()
{
  Destroy();
}

void CAudioDecoder::Destroy()
{
  CSingleLock lock(m_critSection);
  m_status = STATUS_NO_FILE;

  m_pcmBuffer.Destroy();

  if ( m_codec )
    delete m_codec;
  m_codec = NULL;

  m_canPlay = false;
}

bool CAudioDecoder::Create(const CFileItem &file, __int64 seekOffset)
{
  Destroy();

  CSingleLock lock(m_critSection);

  // reset our playback timing variables
  m_eof = false;

  // get correct cache size
  unsigned int filecache = g_guiSettings.GetInt("cacheaudio.internet");
  if ( file.IsHD() )
    filecache = g_guiSettings.GetInt("cache.harddisk");
  else if ( file.IsOnDVD() )
    filecache = g_guiSettings.GetInt("cacheaudio.dvdrom");
  else if ( file.IsOnLAN() )
    filecache = g_guiSettings.GetInt("cacheaudio.lan");

  // create our codec
  m_codec=CodecFactory::CreateCodecDemux(file.m_strPath, file.GetMimeType(), filecache * 1024);

  if (!m_codec || !m_codec->Init(file.m_strPath, filecache * 1024))
  {
    CLog::Log(LOGERROR, "CAudioDecoder: Unable to Init Codec while loading file %s", file.m_strPath.c_str());
    Destroy();
    return false;
  }

  m_blockSize = m_codec->m_Channels * m_codec->m_BitsPerSample / 8;
  m_pcmBuffer.Create(2 * m_blockSize * m_codec->m_SampleRate);
  
  // set total time from the given tag
  if (file.HasMusicInfoTag() && file.GetMusicInfoTag()->GetDuration())
    m_codec->SetTotalTime(file.GetMusicInfoTag()->GetDuration());

  if (seekOffset)
    m_codec->Seek(seekOffset);

  m_status = STATUS_QUEUING;

  return true;
}

void CAudioDecoder::GetDataFormat(unsigned int *channels, unsigned int *samplerate, enum AEDataFormat *dataFormat)
{
  if (!m_codec)
    return;

  if (channels  ) *channels = m_codec->m_Channels;
  if (samplerate) *samplerate = m_codec->m_SampleRate;
  if (dataFormat) *dataFormat = m_codec->m_DataFormat;
}

__int64 CAudioDecoder::Seek(__int64 time)
{
  m_pcmBuffer.Clear();
  if (!m_codec)
    return 0;
  if (time < 0) time = 0;
  if (time > m_codec->m_TotalTime) time = m_codec->m_TotalTime;
  return m_codec->Seek(time);
}

__int64 CAudioDecoder::TotalTime()
{
  if (m_codec)
    return m_codec->m_TotalTime;
  return 0;
}

unsigned int CAudioDecoder::GetDataSize()
{
  if (m_status == STATUS_QUEUING || m_status == STATUS_NO_FILE)
    return 0;
  // check for end of file and end of buffer
  if (m_status == STATUS_ENDING && m_pcmBuffer.getMaxReadSize() < PACKET_SIZE)
    m_status = STATUS_ENDED;
  return m_pcmBuffer.getMaxReadSize() / (m_codec->m_BitsPerSample >> 3);
}

void *CAudioDecoder::GetData(unsigned int size)
{
  if (size > OUTPUT_SAMPLES)
  {
    CLog::Log(LOGWARNING, "CAudioDecoder::GetData() more bytes/samples (%i) requested than we have to give (%i)!", size, OUTPUT_SAMPLES);
    size = OUTPUT_SAMPLES;
  }

  if (m_pcmBuffer.ReadData( (char *)(m_outputBuffer), size * (m_codec->m_BitsPerSample >> 3)))
  {
    // check for end of file + end of buffer
    if (m_status == STATUS_ENDING && (int)m_pcmBuffer.getMaxReadSize() < (OUTPUT_SAMPLES * (m_codec->m_BitsPerSample >> 3)))
    {
      CLog::Log(LOGINFO, "CAudioDecoder::GetData() ending track - only have %lu samples left", (unsigned long)(m_pcmBuffer.getMaxReadSize() / (m_codec->m_BitsPerSample >> 3)));
      m_status = STATUS_ENDED;
    }
    return m_outputBuffer;
  }
  CLog::Log(LOGERROR, "CAudioDecoder::GetData() ReadBinary failed with %i samples", size);
  return NULL;
}

int CAudioDecoder::ReadSamples(int numsamples)
{
  if (m_status == STATUS_NO_FILE || m_status == STATUS_ENDING || m_status == STATUS_ENDED)
    return RET_SLEEP;             // nothing loaded yet

  // start playing once we're fully queued and we're ready to go
  if (m_status == STATUS_QUEUED && m_canPlay)
    m_status = STATUS_PLAYING;

  // grab a lock to ensure the codec is created at this point.
  CSingleLock lock(m_critSection);

  // Read in more data
  int maxsize = std::min<int>(INPUT_SAMPLES, m_pcmBuffer.getMaxWriteSize() / (m_codec->m_BitsPerSample >> 3));
  numsamples = std::min<int>(numsamples, maxsize);
  numsamples -= (numsamples % m_codec->m_Channels);  // make sure it's divisible by our number of channels
  if ( numsamples )
  {
    int samples = 0;
    int result = m_codec->ReadPCM(m_pcmInputBuffer, numsamples * (m_codec->m_BitsPerSample >> 3), &samples);

    if ( result != READ_ERROR && samples)
    {
      // move it into our buffer
      m_pcmBuffer.WriteData((char *)m_pcmInputBuffer, samples);

      // update status
      if (m_status == STATUS_QUEUING && m_pcmBuffer.getMaxReadSize() > m_pcmBuffer.getSize() * 0.9)
      {
        CLog::Log(LOGINFO, "AudioDecoder: File is queued");
        m_status = STATUS_QUEUED;
      }

      if (result == READ_EOF) // EOF reached
      {
        // setup ending if we're within set time of the end (currently just EOF)
        m_eof = true;
        if (m_status < STATUS_ENDING)
          m_status = STATUS_ENDING;
      }

      return RET_SUCCESS;
    }
    if (result == READ_ERROR)
    {
      // error decoding, lets finish up and get out
      CLog::Log(LOGERROR, "CAudioDecoder: Error while decoding %i", result);
      return RET_ERROR;
    }
    if (result == READ_EOF)
    {
      m_eof = true;
      // setup ending if we're within set time of the end (currently just EOF)
      if (m_status < STATUS_ENDING)
        m_status = STATUS_ENDING;
    }
  }
  return RET_SLEEP; // nothing to do
}

float CAudioDecoder::GetReplayGain()
{
#define REPLAY_GAIN_DEFAULT_LEVEL 89.0f
  if (g_guiSettings.m_replayGain.iType == REPLAY_GAIN_NONE)
    return 0.0f;

  // Compute amount of gain
  float replaydB = (float)g_guiSettings.m_replayGain.iNoGainPreAmp;
  float peak = 0.0f;
  if (g_guiSettings.m_replayGain.iType == REPLAY_GAIN_ALBUM)
  {
    if (m_codec->m_replayGain.iHasGainInfo & REPLAY_GAIN_HAS_ALBUM_INFO)
    {
      replaydB = (float)g_guiSettings.m_replayGain.iPreAmp + (float)m_codec->m_replayGain.iAlbumGain / 100.0f;
      peak = m_codec->m_replayGain.fAlbumPeak;
    }
    else if (m_codec->m_replayGain.iHasGainInfo & REPLAY_GAIN_HAS_TRACK_INFO)
    {
      replaydB = (float)g_guiSettings.m_replayGain.iPreAmp + (float)m_codec->m_replayGain.iTrackGain / 100.0f;
      peak = m_codec->m_replayGain.fTrackPeak;
    }
  }
  else if (g_guiSettings.m_replayGain.iType == REPLAY_GAIN_TRACK)
  {
    if (m_codec->m_replayGain.iHasGainInfo & REPLAY_GAIN_HAS_TRACK_INFO)
    {
      replaydB = (float)g_guiSettings.m_replayGain.iPreAmp + (float)m_codec->m_replayGain.iTrackGain / 100.0f;
      peak = m_codec->m_replayGain.fTrackPeak;
    }
    else if (m_codec->m_replayGain.iHasGainInfo & REPLAY_GAIN_HAS_ALBUM_INFO)
    {
      replaydB = (float)g_guiSettings.m_replayGain.iPreAmp + (float)m_codec->m_replayGain.iAlbumGain / 100.0f;
      peak = m_codec->m_replayGain.fAlbumPeak;
    }
  }
  // convert to a gain type
  float replaygain = pow(10.0f, (replaydB - REPLAY_GAIN_DEFAULT_LEVEL)* 0.05f);
  // check peaks
  if (g_guiSettings.m_replayGain.bAvoidClipping)
  {
    if (fabs(peak * replaygain) > 1.0f)
      replaygain = 1.0f / fabs(peak);
  }

  return replaygain;
}

