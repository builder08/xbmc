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

#include "system.h"
#include "DVDPlayer.h"

#include "DVDInputStreams/DVDInputStream.h"
#include "DVDInputStreams/DVDFactoryInputStream.h"
#include "DVDInputStreams/DVDInputStreamNavigator.h"
#include "DVDInputStreams/DVDInputStreamTV.h"
#include "DVDInputStreams/DVDInputStreamPVRManager.h"

#include "DVDDemuxers/DVDDemux.h"
#include "DVDDemuxers/DVDDemuxUtils.h"
#include "DVDDemuxers/DVDDemuxVobsub.h"
#include "DVDDemuxers/DVDFactoryDemuxer.h"
#include "DVDDemuxers/DVDDemuxFFmpeg.h"

#include "DVDCodecs/DVDCodecs.h"
#include "DVDCodecs/DVDFactoryCodec.h"

#include "DVDFileInfo.h"

#include "Util.h"
#include "utils/GUIInfoManager.h"
#include "GUIWindowManager.h"
#include "GUIDialogFullScreenInfo.h"
#include "Application.h"
#include "DVDPerformanceCounter.h"
#include "FileSystem/File.h"
#include "Picture.h"
#include "Codecs/DllSwScale.h"
#ifdef HAS_VIDEO_PLAYBACK
#include "cores/VideoRenderers/RenderManager.h"
#endif
#ifdef HAS_PERFORMANCE_SAMPLE
#include "xbmc/utils/PerformanceSample.h"
#else
#define MEASURE_FUNCTION
#endif
#include "AdvancedSettings.h"
#include "FileItem.h"
#include "MouseStat.h"
#include "GUISettings.h"
#include "GUIUserMessages.h"
#include "Settings.h"
#include "LocalizeStrings.h"
#include "utils/log.h"
#include "utils/TimeUtils.h"
#include "utils/StreamDetails.h"
#include "MediaManager.h"
#include "PVRManager.h"
#include "FileSystem/PVRFile.h"
#include "GUIDialogBusy.h"

using namespace std;

#ifdef HAVE_LIBVDPAU
#include "cores/dvdplayer/DVDCodecs/Video/VDPAU.h"
#endif

void CSelectionStreams::Clear(StreamType type, StreamSource source)
{
  CSingleLock lock(m_section);
  for(int i=m_Streams.size()-1;i>=0;i--)
  {
    if(type && m_Streams[i].type != type)
      continue;

    if(source && m_Streams[i].source != source)
      continue;

    m_Streams.erase(m_Streams.begin() + i);
  }
}

SelectionStream& CSelectionStreams::Get(StreamType type, int index)
{
  CSingleLock lock(m_section);
  int count = -1;
  for(int i=0;i<(int)m_Streams.size();i++)
  {
    if(m_Streams[i].type != type)
      continue;
    count++;
    if(count == index)
      return m_Streams[i];
  }
  CLog::Log(LOGERROR, "%s - failed to get stream", __FUNCTION__);
  return m_invalid;
}

int CSelectionStreams::IndexOf(StreamType type, int source, int id)
{
  CSingleLock lock(m_section);
  int count = -1;
  for(int i=0;i<(int)m_Streams.size();i++)
  {
    if(type && m_Streams[i].type != type)
      continue;
    count++;
    if(source && m_Streams[i].source != source)
      continue;
    if(id < 0)
      continue;
    if(m_Streams[i].id == id)
      return count;
  }
  if(id < 0)
    return count;
  else
    return -1;
}

int CSelectionStreams::IndexOf(StreamType type, CDVDPlayer& p)
{
  if (p.m_pInputStream && p.m_pInputStream->IsStreamType(DVDSTREAM_TYPE_DVD))
  {
    int id = -1;
    if(type == STREAM_AUDIO)
      id = ((CDVDInputStreamNavigator*)p.m_pInputStream)->GetActiveAudioStream();
    else if(type == STREAM_VIDEO)
      id = p.m_CurrentVideo.id;
    else if(type == STREAM_SUBTITLE)
      id = ((CDVDInputStreamNavigator*)p.m_pInputStream)->GetActiveSubtitleStream();

    return IndexOf(type, STREAM_SOURCE_NAV, id);
  }

  if(type == STREAM_AUDIO)
    return IndexOf(type, p.m_CurrentAudio.source, p.m_CurrentAudio.id);
  else if(type == STREAM_VIDEO)
    return IndexOf(type, p.m_CurrentVideo.source, p.m_CurrentVideo.id);
  else if(type == STREAM_SUBTITLE)
    return IndexOf(type, p.m_CurrentSubtitle.source, p.m_CurrentSubtitle.id);
  else if(type == STREAM_TELETEXT)
    return IndexOf(type, p.m_CurrentTeletext.source, p.m_CurrentTeletext.id);

  return -1;
}

int CSelectionStreams::Source(StreamSource source, std::string filename)
{
  CSingleLock lock(m_section);
  int index = source - 1;
  for(int i=0;i<(int)m_Streams.size();i++)
  {
    SelectionStream &s = m_Streams[i];
    if(STREAM_SOURCE_MASK(s.source) != source)
      continue;
    // if it already exists, return same
    if(s.filename == filename)
      return s.source;
    if(index < s.source)
      index = s.source;
  }
  // return next index
  return index + 1;
}

void CSelectionStreams::Update(SelectionStream& s)
{
  CSingleLock lock(m_section);
  int index = IndexOf(s.type, s.source, s.id);
  if(index >= 0)
    Get(s.type, index) = s;
  else
    m_Streams.push_back(s);
}

void CSelectionStreams::Update(CDVDInputStream* input, CDVDDemux* demuxer)
{
  if(input && input->IsStreamType(DVDSTREAM_TYPE_DVD))
  {
    CDVDInputStreamNavigator* nav = (CDVDInputStreamNavigator*)input;
    string filename = nav->GetFileName();
    int source = Source(STREAM_SOURCE_NAV, filename);

    int count;
    count = nav->GetAudioStreamCount();
    for(int i=0;i<count;i++)
    {
      SelectionStream s;
      s.source   = source;
      s.type     = STREAM_AUDIO;
      s.id       = i;
      s.name     = nav->GetAudioStreamLanguage(i);
      s.filename = filename;
      Update(s);
    }

    count = nav->GetSubTitleStreamCount();
    for(int i=0;i<count;i++)
    {
      SelectionStream s;
      s.source   = source;
      s.type     = STREAM_SUBTITLE;
      s.id       = i;
      s.name     = nav->GetSubtitleStreamLanguage(i);
      s.filename = filename;
      Update(s);
    }
  }
  else if(demuxer)
  {
    string filename = demuxer->GetFileName();
    int count = demuxer->GetNrOfStreams();
    int source;
    if(input) /* hack to know this is sub decoder */
      source = Source(STREAM_SOURCE_DEMUX, filename);
    else
      source = Source(STREAM_SOURCE_DEMUX_SUB, filename);


    for(int i=0;i<count;i++)
    {
      CDemuxStream* stream = demuxer->GetStream(i);
      /* make sure stream is marked with right source */
      stream->source = source;

      SelectionStream s;
      s.source   = source;
      s.type     = stream->type;
      s.id       = stream->iId;
      s.language = stream->language;
      s.filename = demuxer->GetFileName();
      stream->GetStreamName(s.name);
      if(stream->type == STREAM_AUDIO)
      {
        std::string type;
        ((CDemuxStreamAudio*)stream)->GetStreamType(type);
        if(type.length() > 0)
        {
          if(s.name.length() > 0)
            s.name += " - ";
          s.name += type;
        }
      }
      Update(s);
    }
  }
}

CDVDPlayer::CDVDPlayer(IPlayerCallback& callback)
    : IPlayer(callback),
      CThread(),
      m_CurrentAudio(STREAM_AUDIO),
      m_CurrentVideo(STREAM_VIDEO),
      m_CurrentSubtitle(STREAM_SUBTITLE),
      m_CurrentTeletext(STREAM_TELETEXT),
      m_messenger("player"),
      m_dvdPlayerVideo(&m_clock, &m_overlayContainer, m_messenger),
      m_dvdPlayerAudio(&m_clock, m_messenger),
      m_dvdPlayerSubtitle(&m_overlayContainer),
      m_dvdPlayerTeletext(),
      m_ready(true)
{
  m_pDemuxer = NULL;
  m_pSubtitleDemuxer = NULL;
  m_pInputStream = NULL;

  InitializeCriticalSection(&m_critStreamSection);

  m_dvd.Clear();
  m_State.Clear();
  m_UpdateApplication = 0;

  m_bAbortRequest = false;
  m_errorCount = 0;
  m_playSpeed = DVD_PLAYSPEED_NORMAL;
  m_caching = CACHESTATE_DONE;

#ifdef DVDDEBUG_MESSAGE_TRACKER
  g_dvdMessageTracker.Init();
#endif
}

CDVDPlayer::~CDVDPlayer()
{
  CloseFile();

  DeleteCriticalSection(&m_critStreamSection);
#ifdef DVDDEBUG_MESSAGE_TRACKER
  g_dvdMessageTracker.DeInit();
#endif
}

bool CDVDPlayer::OpenFile(const CFileItem& file, const CPlayerOptions &options)
{
  try
  {
    CLog::Log(LOGNOTICE, "DVDPlayer: Opening: %s", file.m_strPath.c_str());

    // if playing a file close it first
    // this has to be changed so we won't have to close it.
    if(ThreadHandle())
      CloseFile();

    m_bAbortRequest = false;
    SetPlaySpeed(DVD_PLAYSPEED_NORMAL);

    m_State.Clear();
    m_UpdateApplication = 0;

    m_PlayerOptions = options;
    m_item     = file;
    m_content  = file.GetContentType();
    m_filename = file.m_strPath;

    m_ready.Reset();
    Create();
    if(!m_ready.WaitMSec(100))
    {
      CGUIDialogBusy* dialog = (CGUIDialogBusy*)g_windowManager.GetWindow(WINDOW_DIALOG_BUSY);
      dialog->Show();
      while(!m_ready.WaitMSec(1))
        g_windowManager.Process(true);
      dialog->Close();
    }

    // Playback might have been stopped due to some error
    if (m_bStop || m_bAbortRequest)
      return false;

    return true;
  }
  catch(...)
  {
    CLog::Log(LOGERROR, "%s - Exception thrown on open", __FUNCTION__);
    return false;
  }
}

bool CDVDPlayer::CloseFile()
{
  CLog::Log(LOGNOTICE, "CDVDPlayer::CloseFile()");

  // unpause the player
  SetPlaySpeed(DVD_PLAYSPEED_NORMAL);

  // set the abort request so that other threads can finish up
  m_bAbortRequest = true;

  // tell demuxer to abort
  if(m_pDemuxer)
    m_pDemuxer->Abort();

  if(m_pSubtitleDemuxer)
    m_pSubtitleDemuxer->Abort();

  CLog::Log(LOGNOTICE, "DVDPlayer: waiting for threads to exit");

  // wait for the main thread to finish up
  // since this main thread cleans up all other resources and threads
  // we are done after the StopThread call
  StopThread();

  // Added to stop live TV streams from the PVR addons at the backend side
  // Used for the MediaPortal PVR addon to stop the rtsp timeshift stream
  if (m_item.HasPVRChannelInfoTag())
  {
    if (m_item.GetPVRChannelInfoTag()->StreamURL().compare(0,13, "pvr://stream/") == 0)
    {
      m_filename = m_item.m_strPath; //Restore the original pvr path
      g_PVRManager.CloseStream();
    }
  }

  m_Edl.Clear();
  m_EdlAutoSkipMarkers.Clear();

  CLog::Log(LOGNOTICE, "DVDPlayer: finished waiting");
#if defined(HAS_VIDEO_PLAYBACK)
  g_renderManager.UnInit();
#endif
#ifdef HAVE_LIBVDPAU
  if (g_VDPAU)
    CloseVideoStream(!m_bAbortRequest);
#endif //HAVE_LIBVDPAU
  return true;
}

bool CDVDPlayer::IsPlaying() const
{
  return !m_bStop;
}

void CDVDPlayer::OnStartup()
{
  CThread::SetName("CDVDPlayer");
  m_CurrentVideo.Clear();
  m_CurrentAudio.Clear();
  m_CurrentSubtitle.Clear();
  m_CurrentTeletext.Clear();

  m_messenger.Init();

  g_dvdPerformanceCounter.EnableMainPerformance(ThreadHandle());
}

bool CDVDPlayer::OpenInputStream()
{
  if(m_pInputStream)
    SAFE_DELETE(m_pInputStream);

  CLog::Log(LOGNOTICE, "Creating InputStream");

  // correct the filename if needed
  CStdString filename(m_filename);
  if (filename.Find("dvd://") == 0
  ||  filename.CompareNoCase("d:\\video_ts\\video_ts.ifo") == 0
  ||  filename.CompareNoCase("iso9660://video_ts/video_ts.ifo") == 0)
  {
    m_filename = g_mediaManager.TranslateDevicePath("");
  }

  //margro: Added pvr://stream/ for MediaPortal streams
  //        XBMC can handle rtsp:// streams directly, but
  //        we don't know the URL of the live tv/radio stream on beforehand
  //        So, we need to ask the PVR plugin for the stream URL for this
  //        channel, before starting the right player
  if (m_filename.compare(0, 6, "pvr://") == 0)
  {
    m_filename = XFILE::CPVRFile::TranslatePVRFilename(m_filename);
    if(m_filename.length() == 0)
    {
      CLog::Log(LOGERROR, "CDVDPlayer::OpenInputStream - unable to translate [%s] into input stream", m_item.m_strPath.c_str());
      return false;
    }
  }

  m_pInputStream = CDVDFactoryInputStream::CreateInputStream(this, m_filename, m_content);
  if(m_pInputStream == NULL)
  {
    CLog::Log(LOGERROR, "CDVDPlayer::OpenInputStream - unable to create input stream for [%s]", m_filename.c_str());
    return false;
  }
  else
    m_pInputStream->SetFileItem(m_item);

  if (!m_pInputStream->Open(m_filename.c_str(), m_content))
  {
    CLog::Log(LOGERROR, "CDVDPlayer::OpenInputStream - error opening [%s]", m_filename.c_str());
    return false;
  }

  // find any available external subtitles for non dvd files
  if (!m_pInputStream->IsStreamType(DVDSTREAM_TYPE_DVD)
  &&  !m_pInputStream->IsStreamType(DVDSTREAM_TYPE_PVRMANAGER)
  &&  !m_pInputStream->IsStreamType(DVDSTREAM_TYPE_TV)
  &&  !m_pInputStream->IsStreamType(DVDSTREAM_TYPE_HTSP))
  {
    // find any available external subtitles
    std::vector<std::string> filenames;
    CDVDFactorySubtitle::GetSubtitles(filenames, m_filename);
    for(unsigned int i=0;i<filenames.size();i++)
      AddSubtitleFile(filenames[i]);

    g_settings.m_currentVideoSettings.m_SubtitleCached = true;
  }

  SetAVDelay(g_settings.m_currentVideoSettings.m_AudioDelay);
  SetSubTitleDelay(g_settings.m_currentVideoSettings.m_SubtitleDelay);
  m_clock.Discontinuity(CLOCK_DISC_FULL);
  m_dvd.Clear();
  m_errorCount = 0;
  m_ChannelEntryTimeOut = 0;

  return true;
}

bool CDVDPlayer::OpenDemuxStream()
{
  if(m_pDemuxer)
    SAFE_DELETE(m_pDemuxer);

  CLog::Log(LOGNOTICE, "Creating Demuxer");

  try
  {
    int attempts = 10;
    while(!m_bStop && attempts-- > 0)
    {
      m_pDemuxer = CDVDFactoryDemuxer::CreateDemuxer(m_pInputStream);
      if(!m_pDemuxer && m_pInputStream->IsStreamType(DVDSTREAM_TYPE_PVRMANAGER))
      {
        Sleep(200);
        continue;
      }
      else if(!m_pDemuxer && m_pInputStream->NextStream())
      {
        CLog::Log(LOGDEBUG, "%s - New stream available from input, retry open", __FUNCTION__);
        continue;
      }
      break;
    }

    if(!m_pDemuxer)
    {
      CLog::Log(LOGERROR, "%s - Error creating demuxer", __FUNCTION__);
      return false;
    }

  }
  catch(...)
  {
    CLog::Log(LOGERROR, "%s - Exception thrown when opening demuxer", __FUNCTION__);
    return false;
  }

  m_SelectionStreams.Clear(STREAM_NONE, STREAM_SOURCE_DEMUX);
  m_SelectionStreams.Clear(STREAM_NONE, STREAM_SOURCE_NAV);
  m_SelectionStreams.Update(m_pInputStream, m_pDemuxer);

  return true;
}

void CDVDPlayer::OpenDefaultStreams()
{
  int  count;
  bool valid;
  // open video stream
  count = m_SelectionStreams.Count(STREAM_VIDEO);
  valid = false;
  for(int i = 0;i<count && !valid;i++)
  {
    SelectionStream& s = m_SelectionStreams.Get(STREAM_VIDEO, i);
    if(OpenVideoStream(s.id, s.source))
      valid = true;
  }
  if(!valid)
    CloseVideoStream(true);

  if(!m_PlayerOptions.video_only)
  {
    // open audio stream
    count = m_SelectionStreams.Count(STREAM_AUDIO);
    valid = false;
    if(g_settings.m_currentVideoSettings.m_AudioStream >= 0
    && g_settings.m_currentVideoSettings.m_AudioStream < count)
    {
      SelectionStream& s = m_SelectionStreams.Get(STREAM_AUDIO, g_settings.m_currentVideoSettings.m_AudioStream);
      if(OpenAudioStream(s.id, s.source))
        valid = true;
      else
        CLog::Log(LOGWARNING, "%s - failed to restore selected audio stream (%d)", __FUNCTION__, g_settings.m_currentVideoSettings.m_AudioStream);
    }

    for(int i = 0; i<count && !valid; i++)
    {
      SelectionStream& s = m_SelectionStreams.Get(STREAM_AUDIO, i);
      if(OpenAudioStream(s.id, s.source))
        valid = true;
    }
    if(!valid)
      CloseAudioStream(true);
  }

  // open subtitle stream
  count = m_SelectionStreams.Count(STREAM_SUBTITLE);
  valid = false;
  if(g_settings.m_currentVideoSettings.m_SubtitleStream >= 0
  && g_settings.m_currentVideoSettings.m_SubtitleStream < count)
  {
    SelectionStream& s = m_SelectionStreams.Get(STREAM_SUBTITLE, g_settings.m_currentVideoSettings.m_SubtitleStream);
    if(OpenSubtitleStream(s.id, s.source))
      valid = true;
    else
      CLog::Log(LOGWARNING, "%s - failed to restore selected subtitle stream (%d)", __FUNCTION__, g_settings.m_currentVideoSettings.m_SubtitleStream);
  }

  for(int i = 0;i<count && !valid; i++)
  {
    SelectionStream& s = m_SelectionStreams.Get(STREAM_SUBTITLE, i);
    if(OpenSubtitleStream(s.id, s.source))
      valid = true;
  }
  if(!valid)
    CloseSubtitleStream(false);
  if(g_settings.m_currentVideoSettings.m_SubtitleOn && !m_PlayerOptions.video_only)
    m_dvdPlayerVideo.EnableSubtitle(true);
  else
    m_dvdPlayerVideo.EnableSubtitle(false);
  // open teletext data stream
  count = m_SelectionStreams.Count(STREAM_TELETEXT);
  valid = false;
  for(int i = 0;i<count && !valid;i++)
  {
    SelectionStream& s = m_SelectionStreams.Get(STREAM_TELETEXT, i);
    if(OpenTeletextStream(s.id, s.source))
      valid = true;
  }
  if(!valid)
    CloseTeletextStream(true);
}

bool CDVDPlayer::ReadPacket(DemuxPacket*& packet, CDemuxStream*& stream)
{

  // check if we should read from subtitle demuxer
  if(m_dvdPlayerSubtitle.AcceptsData() && m_pSubtitleDemuxer )
  {
    if(m_pSubtitleDemuxer)
      packet = m_pSubtitleDemuxer->Read();

    if(packet)
    {
      if(packet->iStreamId < 0)
        return true;

      stream = m_pSubtitleDemuxer->GetStream(packet->iStreamId);
      if (!stream)
      {
        CLog::Log(LOGERROR, "%s - Error demux packet doesn't belong to a valid stream", __FUNCTION__);
        return false;
      }
      if(stream->source == STREAM_SOURCE_NONE)
      {
        m_SelectionStreams.Clear(STREAM_NONE, STREAM_SOURCE_DEMUX_SUB);
        m_SelectionStreams.Update(NULL, m_pSubtitleDemuxer);
      }
      return true;
    }
  }

  // read a data frame from stream.
  if(m_pDemuxer)
    packet = m_pDemuxer->Read();

  if(packet)
  {

    // correct for timestamp errors, maybe should be inside demuxer
    if(m_pInputStream && m_pInputStream->IsStreamType(DVDSTREAM_TYPE_DVD))
    {
      CDVDInputStreamNavigator *pInput = static_cast<CDVDInputStreamNavigator*>(m_pInputStream);

      if (packet->dts != DVD_NOPTS_VALUE)
        packet->dts -= pInput->GetTimeStampCorrection();
      if (packet->pts != DVD_NOPTS_VALUE)
        packet->pts -= pInput->GetTimeStampCorrection();
    }

    // this groupId stuff is getting a bit messy, need to find a better way
    // currently it is used to determine if a menu overlay is associated with a picture
    // for dvd's we use as a group id, the current cell and the current title
    // to be a bit more precise we alse count the number of disc's in case of a pts wrap back in the same cell / title
    packet->iGroupId = m_pInputStream->GetCurrentGroupId();

    if(packet->iStreamId < 0)
      return true;

    stream = m_pDemuxer->GetStream(packet->iStreamId);
    if (!stream)
    {
      CLog::Log(LOGERROR, "%s - Error demux packet doesn't belong to a valid stream", __FUNCTION__);
      return false;
    }
    if(stream->source == STREAM_SOURCE_NONE)
    {
      m_SelectionStreams.Clear(STREAM_NONE, STREAM_SOURCE_DEMUX);
      m_SelectionStreams.Update(m_pInputStream, m_pDemuxer);
    }
    return true;
  }
  return false;
}

bool CDVDPlayer::IsValidStream(CCurrentStream& stream)
{
  if(stream.id<0)
    return true; // we consider non selected as valid

  int source = STREAM_SOURCE_MASK(stream.source);
  if(source == STREAM_SOURCE_TEXT)
    return true;
  if(source == STREAM_SOURCE_DEMUX_SUB)
  {
    CDemuxStream* st = m_pSubtitleDemuxer->GetStream(stream.id);
    if(st == NULL || st->disabled)
      return false;
    if(st->type != stream.type)
      return false;
    return true;
  }
  if(source == STREAM_SOURCE_DEMUX)
  {
    CDemuxStream* st = m_pDemuxer->GetStream(stream.id);
    if(st == NULL || st->disabled)
      return false;
    if(st->type != stream.type)
      return false;

    if (m_pInputStream && m_pInputStream->IsStreamType(DVDSTREAM_TYPE_DVD))
    {
      if(stream.type == STREAM_AUDIO    && st->iPhysicalId != m_dvd.iSelectedAudioStream)
        return false;
      if(stream.type == STREAM_SUBTITLE && st->iPhysicalId != m_dvd.iSelectedSPUStream)
        return false;
    }

    return true;
  }

  return false;
}

bool CDVDPlayer::IsBetterStream(CCurrentStream& current, CDemuxStream* stream)
{
  // Do not reopen non-video streams if we're in video-only mode
  if(m_PlayerOptions.video_only && current.type != STREAM_VIDEO)
    return false;

  if (m_pInputStream && m_pInputStream->IsStreamType(DVDSTREAM_TYPE_DVD))
  {
    int source_type;

    source_type = STREAM_SOURCE_MASK(current.source);
    if(source_type != STREAM_SOURCE_DEMUX
    && source_type != STREAM_SOURCE_NONE)
      return false;

    source_type = STREAM_SOURCE_MASK(stream->source);
    if(source_type  != STREAM_SOURCE_DEMUX
    || stream->type != current.type
    || stream->iId  == current.id)
      return false;

    if(current.type == STREAM_AUDIO    && stream->iPhysicalId == m_dvd.iSelectedAudioStream)
      return true;
    if(current.type == STREAM_SUBTITLE && stream->iPhysicalId == m_dvd.iSelectedSPUStream)
      return true;
    if(current.type == STREAM_VIDEO    && current.id < 0)
      return true;
  }
  else
  {
    if(stream->source == current.source
    && stream->iId    == current.id)
      return false;

    if(stream->disabled)
      return false;

    if(stream->type != current.type)
      return false;

    if(current.type == STREAM_SUBTITLE)
      return false;

    if(current.type == STREAM_TELETEXT)
      return false;

    if(current.id < 0)
      return true;
  }
  return false;
}

void CDVDPlayer::Process()
{
  if (!OpenInputStream())
  {
    m_bAbortRequest = true;
    return;
  }

  if(m_pInputStream->IsStreamType(DVDSTREAM_TYPE_DVD))
  {
    CLog::Log(LOGNOTICE, "DVDPlayer: playing a dvd with menu's");
    m_PlayerOptions.starttime = 0;


    if(m_PlayerOptions.state.size() > 0)
      ((CDVDInputStreamNavigator*)m_pInputStream)->SetNavigatorState(m_PlayerOptions.state);
    ((CDVDInputStreamNavigator*)m_pInputStream)->EnableSubtitleStream(g_settings.m_currentVideoSettings.m_SubtitleOn);

    g_settings.m_currentVideoSettings.m_SubtitleCached = true;
  }

  if(!OpenDemuxStream())
  {
    m_bAbortRequest = true;
    return;
  }

  // allow renderer to switch to fullscreen if requested
  m_dvdPlayerVideo.EnableFullscreen(m_PlayerOptions.fullscreen);

  OpenDefaultStreams();

  // look for any EDL files
  m_Edl.Clear();
  m_EdlAutoSkipMarkers.Clear();
  float fFramesPerSecond;
  if (m_CurrentVideo.id >= 0 && m_CurrentVideo.hint.fpsrate > 0 && m_CurrentVideo.hint.fpsscale > 0)
    fFramesPerSecond = (float)m_CurrentVideo.hint.fpsrate / (float)m_CurrentVideo.hint.fpsscale;
  else
  {
    fFramesPerSecond = 25.0; // TODO: Default to one of 50.0, 29.97, 25.0, or 23.976 fps. Advanced setting?
    CLog::Log(LOGWARNING, "%s - Could not detect frame rate for: %s. Using default of %.3f fps for conversion of any commercial break frame markers to times.",
              __FUNCTION__, m_filename.c_str(), fFramesPerSecond);
  }
  m_Edl.ReadEditDecisionLists(m_filename, fFramesPerSecond);

  /*
   * Check to see if the demuxer should start at something other than time 0. This will be the case
   * if there was a start time specified as part of the "Start from where last stopped" (aka
   * auto-resume) feature or if there is an EDL cut or commercial break that starts at time 0.
   */
  CEdl::Cut cut;
  int starttime = 0;
  if(m_PlayerOptions.starttime > 0)
  {
    starttime = m_Edl.RestoreCutTime((__int64)m_PlayerOptions.starttime * 1000); // s to ms
    CLog::Log(LOGDEBUG, "%s - Start position set to last stopped position: %d", __FUNCTION__, starttime);
  }
  else if(m_Edl.InCut(0, &cut)
      && (cut.action == CEdl::CUT || cut.action == CEdl::COMM_BREAK))
  {
    starttime = cut.end;
    CLog::Log(LOGDEBUG, "%s - Start position set to end of first cut or commercial break: %d", __FUNCTION__, starttime);
    if(cut.action == CEdl::COMM_BREAK)
    {
      /*
       * Setup auto skip markers as if the commercial break had been skipped using standard
       * detection.
       */
      m_EdlAutoSkipMarkers.commbreak_start = cut.start;
      m_EdlAutoSkipMarkers.commbreak_end   = cut.end;
      m_EdlAutoSkipMarkers.seek_to_start   = true;
    }
  }
  if(starttime > 0)
  {
    double startpts = DVD_NOPTS_VALUE;
    if(m_pDemuxer)
    {
      if (m_pDemuxer->SeekTime(starttime, false, &startpts))
        CLog::Log(LOGDEBUG, "%s - starting demuxer from: %d", __FUNCTION__, starttime);
      else
        CLog::Log(LOGDEBUG, "%s - failed to start demuxing from: %d", __FUNCTION__, starttime);
    }

    if(m_pSubtitleDemuxer)
    {
      if(m_pSubtitleDemuxer->SeekTime(starttime, false, &startpts))
        CLog::Log(LOGDEBUG, "%s - starting subtitle demuxer from: %d", __FUNCTION__, starttime);
      else
        CLog::Log(LOGDEBUG, "%s - failed to start subtitle demuxing from: %d", __FUNCTION__, starttime);
    }
  }

  // make sure application know our info
  UpdateApplication(0);
  UpdatePlayState(0);

  if(m_PlayerOptions.identify == false)
    m_callback.OnPlayBackStarted();

  // we are done initializing now, set the readyevent
  m_ready.Set();

  // make sure all selected stream have data on startup
  SetCaching(CACHESTATE_INIT);

  while (!m_bAbortRequest)
  {
    // handle messages send to this thread, like seek or demuxer reset requests
    HandleMessages();

    if(m_bAbortRequest)
      break;

    // should we open a new input stream?
    if(!m_pInputStream)
    {
      if (OpenInputStream() == false)
      {
        m_bAbortRequest = true;
        break;
      }
    }

    // should we open a new demuxer?
    if(!m_pDemuxer)
    {
      if (m_pInputStream->NextStream() == false)
        break;

      if (m_pInputStream->IsEOF())
        break;

      if (OpenDemuxStream() == false)
      {
        m_bAbortRequest = true;
        break;
      }

      OpenDefaultStreams();
      UpdateApplication(0);
      UpdatePlayState(0);
    }

    // handle eventual seeks due to playspeed
    HandlePlaySpeed();

    // update player state
    UpdatePlayState(200);

    // update application with our state
    UpdateApplication(1000);

    if (m_ChannelEntryTimeOut > 0 && m_pInputStream->IsStreamType(DVDSTREAM_TYPE_PVRMANAGER))
    {
      if (CTimeUtils::GetTimeMS() - m_ChannelEntryTimeOut > g_guiSettings.GetInt("pvrplayback.channelentrytimeout"))
      {
        m_ChannelEntryTimeOut = 0;
        CPlayerSeek m_pause(this);
        CDVDInputStreamPVRManager* pStream = static_cast<CDVDInputStreamPVRManager*>(m_pInputStream);
        int channel = pStream->GetSelectedChannel();
        if(channel > 0 && pStream->SelectChannel(channel, true))
        {
          FlushBuffers(false);
          CloseAudioStream(true);
          CloseVideoStream(true);
          SAFE_DELETE(m_pDemuxer);
          continue;
        }
        else
        {
          break;
        }
      }
    }

    // if the queues are full, no need to read more
    if ((!m_dvdPlayerAudio.AcceptsData() && m_CurrentAudio.id >= 0)
    ||  (!m_dvdPlayerVideo.AcceptsData() && m_CurrentVideo.id >= 0))
    {
      Sleep(10);
      if( m_caching != CACHESTATE_INIT
      || (m_dvdPlayerAudio.m_messageQueue.GetDataSize() == 0 && m_CurrentAudio.id >= 0)
      || (m_dvdPlayerVideo.m_messageQueue.GetDataSize() == 0 && m_CurrentVideo.id >= 0))
      {
        SetCaching(CACHESTATE_DONE);
        SAFE_RELEASE(m_CurrentAudio.startsync);
        SAFE_RELEASE(m_CurrentVideo.startsync);
      }
      continue;
    }

    // always yield to players if they have data
    if((m_dvdPlayerAudio.m_messageQueue.GetDataSize() > 0 || m_CurrentAudio.id < 0)
    && (m_dvdPlayerVideo.m_messageQueue.GetDataSize() > 0 || m_CurrentVideo.id < 0))
      Sleep(0);

    DemuxPacket* pPacket = NULL;
    CDemuxStream *pStream = NULL;
    ReadPacket(pPacket, pStream);
    if (pPacket && !pStream)
    {
      /* probably a empty packet, just free it and move on */
      CDVDDemuxUtils::FreeDemuxPacket(pPacket);
      continue;
    }

    if (!pPacket)
    {
      // when paused, demuxer could be be returning empty
      if (m_playSpeed == DVD_PLAYSPEED_PAUSE)
        continue;

      // if there is another stream available, let
      // player reopen demuxer
      if(m_pInputStream->NextStream())
      {
        SAFE_DELETE(m_pDemuxer);
        continue;
      }

      if (m_pInputStream->IsStreamType(DVDSTREAM_TYPE_DVD))
      {
        CDVDInputStreamNavigator* pStream = static_cast<CDVDInputStreamNavigator*>(m_pInputStream);

        // stream is holding back data until demuxer has flushed
        if(pStream->IsHeld())
        {
          pStream->SkipHold();
          continue;
        }

        // stills will be skipped
        if(m_dvd.state == DVDSTATE_STILL)
        {
          if (m_dvd.iDVDStillTime > 0)
          {
            if (CTimeUtils::GetTimeMS() >= (m_dvd.iDVDStillStartTime + m_dvd.iDVDStillTime))
            {
              m_dvd.iDVDStillTime = 0;
              m_dvd.iDVDStillStartTime = 0;
              m_dvd.state = DVDSTATE_NORMAL;
              pStream->SkipStill();
              continue;
            }
          }
        }

        // if playing a main title DVD/ISO rip, there is no menu structure so
        // dvdnav will tell us it's done by setting EOF on the stream.
        if (pStream->IsEOF())
          break;

        // always continue on dvd's
        Sleep(100);
        continue;
      }
      else if (m_pInputStream->IsStreamType(DVDSTREAM_TYPE_PVRMANAGER))
      {
        CDVDInputStreamPVRManager* pStream = static_cast<CDVDInputStreamPVRManager*>(m_pInputStream);

        if (pStream->IsEOF())
          break;

        Sleep(100);
        continue;
      }

      // make sure we tell all players to finish it's data
      if(m_CurrentAudio.inited)
        m_dvdPlayerAudio.SendMessage   (new CDVDMsg(CDVDMsg::GENERAL_EOF));
      if(m_CurrentVideo.inited)
        m_dvdPlayerVideo.SendMessage   (new CDVDMsg(CDVDMsg::GENERAL_EOF));
      if(m_CurrentSubtitle.inited)
        m_dvdPlayerSubtitle.SendMessage(new CDVDMsg(CDVDMsg::GENERAL_EOF));
      if(m_CurrentTeletext.inited)
        m_dvdPlayerTeletext.SendMessage(new CDVDMsg(CDVDMsg::GENERAL_EOF));
      m_CurrentAudio.inited    = false;
      m_CurrentVideo.inited    = false;
      m_CurrentSubtitle.inited = false;
      m_CurrentTeletext.inited = false;
      m_CurrentAudio.started    = false;
      m_CurrentVideo.started    = false;
      m_CurrentSubtitle.started = false;
      m_CurrentTeletext.started = false;

      // if we are caching, start playing it again
      SetCaching(CACHESTATE_DONE);

      // while players are still playing, keep going to allow seekbacks
      if(m_dvdPlayerAudio.m_messageQueue.GetDataSize() > 0
      || m_dvdPlayerVideo.m_messageQueue.GetDataSize() > 0)
      {
        Sleep(100);
        continue;
      }

      if (!m_pInputStream->IsEOF())
        CLog::Log(LOGINFO, "%s - eof reading from demuxer", __FUNCTION__);

      break;
    }

    // it's a valid data packet, reset error counter
    m_errorCount = 0;

    // check so that none of our streams has become invalid
    if (!IsValidStream(m_CurrentAudio)    && m_dvdPlayerAudio.IsStalled())    CloseAudioStream(true);
    if (!IsValidStream(m_CurrentVideo)    && m_dvdPlayerVideo.IsStalled())    CloseVideoStream(true);
    if (!IsValidStream(m_CurrentSubtitle) && m_dvdPlayerSubtitle.IsStalled()) CloseSubtitleStream(true);
    if (!IsValidStream(m_CurrentTeletext))                                    CloseTeletextStream(true);

    // see if we can find something better to play
    if (IsBetterStream(m_CurrentAudio,    pStream)) OpenAudioStream   (pStream->iId, pStream->source);
    if (IsBetterStream(m_CurrentVideo,    pStream)) OpenVideoStream   (pStream->iId, pStream->source);
    if (IsBetterStream(m_CurrentSubtitle, pStream)) OpenSubtitleStream(pStream->iId, pStream->source);
    if (IsBetterStream(m_CurrentTeletext, pStream)) OpenTeletextStream(pStream->iId, pStream->source);

    // process the packet
    ProcessPacket(pStream, pPacket);

    // check if in a cut or commercial break that should be automatically skipped
    CheckAutoSceneSkip();
  }
}

void CDVDPlayer::ProcessPacket(CDemuxStream* pStream, DemuxPacket* pPacket)
{
    /* process packet if it belongs to selected stream. for dvd's don't allow automatic opening of streams*/
    LockStreams();

    try
    {
      if (pPacket->iStreamId == m_CurrentAudio.id && pStream->source == m_CurrentAudio.source && pStream->type == STREAM_AUDIO)
        ProcessAudioData(pStream, pPacket);
      else if (pPacket->iStreamId == m_CurrentVideo.id && pStream->source == m_CurrentVideo.source && pStream->type == STREAM_VIDEO)
        ProcessVideoData(pStream, pPacket);
      else if (pPacket->iStreamId == m_CurrentSubtitle.id && pStream->source == m_CurrentSubtitle.source && pStream->type == STREAM_SUBTITLE)
        ProcessSubData(pStream, pPacket);
      else if (pPacket->iStreamId == m_CurrentTeletext.id && pStream->source == m_CurrentTeletext.source && pStream->type == STREAM_TELETEXT)
        ProcessTeletextData(pStream, pPacket);
      else
      {
        pStream->SetDiscard(AVDISCARD_ALL);
        CDVDDemuxUtils::FreeDemuxPacket(pPacket); // free it since we won't do anything with it
      }
    }
    catch(...)
    {
      CLog::Log(LOGERROR, "%s - Exception thrown when processing demux packet", __FUNCTION__);
    }

    UnlockStreams();
}

void CDVDPlayer::ProcessAudioData(CDemuxStream* pStream, DemuxPacket* pPacket)
{
  if (m_CurrentAudio.stream != (void*)pStream)
  {
    /* check so that dmuxer hints or extra data hasn't changed */
    /* if they have, reopen stream */

    if (m_CurrentAudio.hint != CDVDStreamInfo(*pStream, true))
      OpenAudioStream( pPacket->iStreamId, pStream->source );

    m_CurrentAudio.stream = (void*)pStream;
  }

  // check if we are too slow and need to recache
  CheckStartCaching(m_CurrentAudio);

  CheckContinuity(m_CurrentAudio, pPacket);
  if(pPacket->dts != DVD_NOPTS_VALUE)
    m_CurrentAudio.dts = pPacket->dts;
  else if(pPacket->pts != DVD_NOPTS_VALUE)
    m_CurrentAudio.dts = pPacket->pts;

  bool drop = false;
  if (CheckPlayerInit(m_CurrentAudio, DVDPLAYER_AUDIO))
    drop = true;

  if (CheckSceneSkip(m_CurrentAudio))
    drop = true;

  m_dvdPlayerAudio.SendMessage(new CDVDMsgDemuxerPacket(pPacket, drop));
}

void CDVDPlayer::ProcessVideoData(CDemuxStream* pStream, DemuxPacket* pPacket)
{
  if (m_CurrentVideo.stream != (void*)pStream)
  {
    /* check so that dmuxer hints or extra data hasn't changed */
    /* if they have reopen stream */

    if (m_CurrentVideo.hint != CDVDStreamInfo(*pStream, true))
      OpenVideoStream(pPacket->iStreamId, pStream->source);

    m_CurrentVideo.stream = (void*)pStream;
  }

  // check if we are too slow and need to recache
  CheckStartCaching(m_CurrentVideo);

  if( pPacket->iSize != 4) //don't check the EOF_SEQUENCE of stillframes
  {
    CheckContinuity(m_CurrentVideo, pPacket);
    if(pPacket->dts != DVD_NOPTS_VALUE)
      m_CurrentVideo.dts = pPacket->dts;
    else if(pPacket->pts != DVD_NOPTS_VALUE)
      m_CurrentVideo.dts = pPacket->pts;
  }

  bool drop = false;
  if (CheckPlayerInit(m_CurrentVideo, DVDPLAYER_VIDEO))
    drop = true;

  if (CheckSceneSkip(m_CurrentVideo))
    drop = true;

  m_dvdPlayerVideo.SendMessage(new CDVDMsgDemuxerPacket(pPacket, drop));
}

void CDVDPlayer::ProcessSubData(CDemuxStream* pStream, DemuxPacket* pPacket)
{
  if (m_CurrentSubtitle.stream != (void*)pStream)
  {
    /* check so that dmuxer hints or extra data hasn't changed */
    /* if they have reopen stream */

    if (m_CurrentSubtitle.hint != CDVDStreamInfo(*pStream, true))
      OpenSubtitleStream(pPacket->iStreamId, pStream->source);

    m_CurrentSubtitle.stream = (void*)pStream;
  }
  if(pPacket->dts != DVD_NOPTS_VALUE)
    m_CurrentSubtitle.dts = pPacket->dts;
  else if(pPacket->pts != DVD_NOPTS_VALUE)
    m_CurrentSubtitle.dts = pPacket->pts;

  bool drop = false;
  if (CheckPlayerInit(m_CurrentSubtitle, DVDPLAYER_SUBTITLE))
    drop = true;

  if (CheckSceneSkip(m_CurrentSubtitle))
    drop = true;

  m_dvdPlayerSubtitle.SendMessage(new CDVDMsgDemuxerPacket(pPacket, drop));

  if(m_pInputStream && m_pInputStream->IsStreamType(DVDSTREAM_TYPE_DVD))
    m_dvdPlayerSubtitle.UpdateOverlayInfo((CDVDInputStreamNavigator*)m_pInputStream, LIBDVDNAV_BUTTON_NORMAL);
}

void CDVDPlayer::ProcessTeletextData(CDemuxStream* pStream, DemuxPacket* pPacket)
{
  if (m_CurrentTeletext.stream != (void*)pStream)
  {
    /* check so that dmuxer hints or extra data hasn't changed */
    /* if they have, reopen stream */
    if (m_CurrentTeletext.hint != CDVDStreamInfo(*pStream, true))
      OpenTeletextStream( pPacket->iStreamId, pStream->source );

    m_CurrentTeletext.stream = (void*)pStream;
  }
  if(pPacket->dts != DVD_NOPTS_VALUE)
    m_CurrentTeletext.dts = pPacket->dts;
  else if(pPacket->pts != DVD_NOPTS_VALUE)
    m_CurrentTeletext.dts = pPacket->pts;

  bool drop = false;
  if (CheckPlayerInit(m_CurrentTeletext, DVDPLAYER_TELETEXT))
    drop = true;

  if (CheckSceneSkip(m_CurrentTeletext))
    drop = true;

  m_dvdPlayerTeletext.SendMessage(new CDVDMsgDemuxerPacket(pPacket, drop));
}

void CDVDPlayer::HandlePlaySpeed()
{
  if(IsInMenu() && m_caching != CACHESTATE_DONE)
    SetCaching(CACHESTATE_DONE);

  if(m_caching == CACHESTATE_INIT)
  {
    // if all enabled streams have been inited we are done
    if((m_CurrentVideo.id < 0 || (m_CurrentVideo.inited && m_CurrentVideo.started))
    && (m_CurrentAudio.id < 0 || (m_CurrentAudio.inited && m_CurrentAudio.started)))
      SetCaching(CACHESTATE_PLAY);
  }
  if(m_caching == CACHESTATE_PLAY)
  {
    // if all enabled streams have started playing we are done
    if((m_CurrentVideo.id < 0 || !m_dvdPlayerVideo.IsStalled())
    && (m_CurrentAudio.id < 0 || !m_dvdPlayerAudio.IsStalled()))
      SetCaching(CACHESTATE_DONE);
  }

  if(GetPlaySpeed() != DVD_PLAYSPEED_NORMAL && GetPlaySpeed() != DVD_PLAYSPEED_PAUSE)
  {
    if (IsInMenu())
    {
      // this can't be done in menu
      SetPlaySpeed(DVD_PLAYSPEED_NORMAL);

    }
    else if (m_CurrentVideo.id >= 0
          &&  m_CurrentVideo.inited == true
          &&  m_SpeedState.lastpts  != m_dvdPlayerVideo.GetCurrentPts()
          &&  m_SpeedState.lasttime != GetTime())
    {
      m_SpeedState.lastpts  = m_dvdPlayerVideo.GetCurrentPts();
      m_SpeedState.lasttime = GetTime();
      // check how much off clock video is when ff/rw:ing
      // a problem here is that seeking isn't very accurate
      // and since the clock will be resynced after seek
      // we might actually not really be playing at the wanted
      // speed. we'd need to have some way to not resync the clock
      // after a seek to remember timing. still need to handle
      // discontinuities somehow

      // when seeking, give the player a headstart to make sure
      // the time it takes to seek doesn't make a difference.
      double error;
      error  = m_clock.GetClock() - m_SpeedState.lastpts;
      error *= m_playSpeed / abs(m_playSpeed);

      if(error > DVD_MSEC_TO_TIME(1000))
      {
        CLog::Log(LOGDEBUG, "CDVDPlayer::Process - Seeking to catch up");
        __int64 iTime = (__int64)(m_SpeedState.lasttime + 500.0 * m_playSpeed / DVD_PLAYSPEED_NORMAL);
        m_messenger.Put(new CDVDMsgPlayerSeek(iTime, (GetPlaySpeed() < 0), true, false));
      }
    }
  }
}

bool CDVDPlayer::CheckStartCaching(CCurrentStream& current)
{
  if(m_caching   != CACHESTATE_DONE
  || m_playSpeed != DVD_PLAYSPEED_NORMAL)
    return false;

  if(IsInMenu())
    return false;

  if((current.type == STREAM_AUDIO && m_dvdPlayerAudio.IsStalled())
  || (current.type == STREAM_VIDEO && m_dvdPlayerVideo.IsStalled()))
  {
    // don't start caching if it's only a single stream that has run dry
    if(m_dvdPlayerAudio.m_messageQueue.GetLevel() > 50
    || m_dvdPlayerVideo.m_messageQueue.GetLevel() > 50)
      return false;

    if(m_pInputStream->IsStreamType(DVDSTREAM_TYPE_HTSP)
    || m_pInputStream->IsStreamType(DVDSTREAM_TYPE_TV)
    || m_pInputStream->IsStreamType(DVDSTREAM_TYPE_PVRMANAGER))
      SetCaching(CACHESTATE_INIT);
    else
    {
      if(current.inited)
        SetCaching(CACHESTATE_FULL);
      else
        SetCaching(CACHESTATE_INIT);
    }
    return true;
  }
  return false;
}

bool CDVDPlayer::CheckPlayerInit(CCurrentStream& current, unsigned int source)
{
  if(current.startsync)
  {
    if ((current.startpts < current.dts && current.dts != DVD_NOPTS_VALUE)
    ||  (current.startpts == DVD_NOPTS_VALUE))
    {
      SendPlayerMessage(current.startsync, source);

      current.startpts = DVD_NOPTS_VALUE;
      current.startsync = NULL;
    }
    else if((current.startpts - current.dts) > DVD_SEC_TO_TIME(20)
         &&  current.dts != DVD_NOPTS_VALUE)
    {
      CLog::Log(LOGDEBUG, "%s - too far to decode before finishing seek", __FUNCTION__);
      if(m_CurrentAudio.startpts != DVD_NOPTS_VALUE)
        m_CurrentAudio.startpts = current.dts;
      if(m_CurrentVideo.startpts != DVD_NOPTS_VALUE)
        m_CurrentVideo.startpts = current.dts;
      if(m_CurrentSubtitle.startpts != DVD_NOPTS_VALUE)
        m_CurrentSubtitle.startpts = current.dts;
      if(m_CurrentTeletext.startpts != DVD_NOPTS_VALUE)
        m_CurrentTeletext.startpts = current.dts;
    }
  }

  // await start sync to be finished
  if(current.startsync)
  {
    CLog::Log(LOGDEBUG, "%s - dropping packet type:%d dts:%f to get to start point at %f", __FUNCTION__, source,  current.dts, current.startpts);
    return true;
  }


  //If this is the first packet after a discontinuity, send it as a resync
  if (current.inited == false && current.dts != DVD_NOPTS_VALUE)
  {
    current.inited   = true;
    current.startpts = current.dts;

    bool setclock = false;
    if(m_playSpeed == DVD_PLAYSPEED_NORMAL)
    {
      if(     source == DVDPLAYER_AUDIO)
        setclock = !m_CurrentVideo.inited;
      else if(source == DVDPLAYER_VIDEO)
        setclock = !m_CurrentAudio.inited;
    }
    else
    {
      if(source == DVDPLAYER_VIDEO)
        setclock = true;
    }

    double starttime = current.startpts;
    if(m_CurrentAudio.inited
    && m_CurrentAudio.startpts != DVD_NOPTS_VALUE
    && m_CurrentAudio.startpts < starttime)
      starttime = m_CurrentAudio.startpts;
    if(m_CurrentVideo.inited
    && m_CurrentVideo.startpts != DVD_NOPTS_VALUE
    && m_CurrentVideo.startpts < starttime)
      starttime = m_CurrentVideo.startpts;

    starttime = current.startpts - starttime;
    if(starttime > 0)
    {
      if(starttime > DVD_SEC_TO_TIME(2))
        CLog::Log(LOGWARNING, "CDVDPlayer::CheckPlayerInit(%d) - Ignoring too large delay of %f", source, starttime);
      else
        SendPlayerMessage(new CDVDMsgDouble(CDVDMsg::GENERAL_DELAY, starttime), source);
    }

    SendPlayerMessage(new CDVDMsgGeneralResync(current.dts, setclock), source);
  }
  return false;
}

void CDVDPlayer::CheckContinuity(CCurrentStream& current, DemuxPacket* pPacket)
{
  if (m_playSpeed < DVD_PLAYSPEED_PAUSE)
    return;

  if( pPacket->dts == DVD_NOPTS_VALUE )
    return;

#if 0
  // these checks seem to cause more harm, than good
  // looping stillframes are not common in normal files
  // and a better fix for this behaviour would be to
  // correct the timestamps with some offset

  if (current.type == STREAM_VIDEO
  && m_CurrentAudio.dts != DVD_NOPTS_VALUE
  && m_CurrentVideo.dts != DVD_NOPTS_VALUE)
  {
    /* check for looping stillframes on non dvd's, dvd's will be detected by long duration check later */
    if( m_pInputStream && !m_pInputStream->IsStreamType(DVDSTREAM_TYPE_DVD))
    {
      /* special case for looping stillframes THX test discs*/
      /* only affect playback when not from dvd */
      if( (m_CurrentAudio.dts > m_CurrentVideo.dts + DVD_MSEC_TO_TIME(200))
      && (m_CurrentVideo.dts == pPacket->dts) )
      {
        CLog::Log(LOGDEBUG, "CDVDPlayer::CheckContinuity - Detected looping stillframe");
        SynchronizePlayers(SYNCSOURCE_VIDEO);
        return;
      }
    }

    /* if we haven't received video for a while, but we have been */
    /* getting audio much more recently, make sure video wait's  */
    /* this occurs especially on thx test disc */
    if( (pPacket->dts > m_CurrentVideo.dts + DVD_MSEC_TO_TIME(200))
     && (pPacket->dts < m_CurrentAudio.dts + DVD_MSEC_TO_TIME(50)) )
    {
      CLog::Log(LOGDEBUG, "CDVDPlayer::CheckContinuity - Potential long duration frame");
      SynchronizePlayers(SYNCSOURCE_VIDEO);
      return;
    }
  }
#endif

  double mindts, maxdts;
  if(m_CurrentAudio.dts == DVD_NOPTS_VALUE)
    maxdts = mindts = m_CurrentVideo.dts;
  else if(m_CurrentVideo.dts == DVD_NOPTS_VALUE)
    maxdts = mindts = m_CurrentAudio.dts;
  else
  {
    maxdts = max(m_CurrentAudio.dts, m_CurrentVideo.dts);
    mindts = min(m_CurrentAudio.dts, m_CurrentVideo.dts);
  }

  /* if we don't have max and min, we can't do anything more */
  if( mindts == DVD_NOPTS_VALUE || maxdts == DVD_NOPTS_VALUE )
    return;

  if( pPacket->dts < mindts - DVD_MSEC_TO_TIME(100) && current.inited)
  {
    /* if video player is rendering a stillframe, we need to make sure */
    /* audio has finished processing it's data otherwise it will be */
    /* displayed too early */

    CLog::Log(LOGWARNING, "CDVDPlayer::CheckContinuity - resync backword :%d, prev:%f, curr:%f, diff:%f"
                            , current.type, current.dts, pPacket->dts, pPacket->dts - current.dts);
    if (m_dvdPlayerVideo.IsStalled() && m_CurrentVideo.dts != DVD_NOPTS_VALUE)
      SynchronizePlayers(SYNCSOURCE_VIDEO);
    else if (m_dvdPlayerAudio.IsStalled() && m_CurrentAudio.dts != DVD_NOPTS_VALUE)
      SynchronizePlayers(SYNCSOURCE_AUDIO);

    m_CurrentAudio.inited = false;
    m_CurrentVideo.inited = false;
    m_CurrentSubtitle.inited = false;
    m_CurrentTeletext.inited = false;
  }

  /* stream jump forward */
  if( pPacket->dts > maxdts + DVD_MSEC_TO_TIME(1000) && current.inited)
  {
    CLog::Log(LOGWARNING, "CDVDPlayer::CheckContinuity - resync forward :%d, prev:%f, curr:%f, diff:%f"
                            , current.type, current.dts, pPacket->dts, pPacket->dts - current.dts);
    /* normally don't need to sync players since video player will keep playing at normal fps */
    /* after a discontinuity */
    //SynchronizePlayers(dts, pts, MSGWAIT_ALL);

    m_CurrentAudio.inited = false;
    m_CurrentVideo.inited = false;
    m_CurrentSubtitle.inited = false;
    m_CurrentTeletext.inited = false;
  }

  if(current.dts != DVD_NOPTS_VALUE && pPacket->dts < current.dts && current.inited)
  {
    /* warn if dts is moving backwords */
    CLog::Log(LOGWARNING, "CDVDPlayer::CheckContinuity - wrapback of stream:%d, prev:%f, curr:%f, diff:%f"
                        , current.type, current.dts, pPacket->dts, pPacket->dts - current.dts);
  }

}

bool CDVDPlayer::CheckSceneSkip(CCurrentStream& current)
{
  if(!m_Edl.HasCut())
    return false;

  if(current.dts == DVD_NOPTS_VALUE)
    return false;

  CEdl::Cut cut;
  if (!m_Edl.InCut(DVD_TIME_TO_MSEC(current.dts), &cut))
    return false;

  /*
   * It's more efficient to drop packets than to seek if the cut is shorter than 10 seconds, but
   * only do this if the cut is short otherwise the clock never reaches the point where it should do
   * a full seek on Windows (instead it clears the audio queue and gets into a loop of starting and
   * stopping caching).
   */
  else if(cut.action == CEdl::CUT
  &&      cut.end - cut.start < 10*1000) // 10 seconds in ms
    return true;
  else
    return false;
}

void CDVDPlayer::CheckAutoSceneSkip()
{
  if(!m_Edl.HasCut())
    return;

  if(m_CurrentAudio.dts == DVD_NOPTS_VALUE
  || m_CurrentVideo.dts == DVD_NOPTS_VALUE)
    return;

  const int clock = DVD_TIME_TO_MSEC(m_clock.GetClock());

  CEdl::Cut cut;
  if(!m_Edl.InCut(clock, &cut))
    return;

  /*
   * HACK: If there was a start time specified by the "Start from where last stopped" (aka
   * auto-resume) feature or the cut starts at time 0, the demuxer is set to start from that time.
   *
   * However, there is a race condition that sometimes prevents the clock being updated to reflect
   * the actual playback time after setting the initial demuxer start time. So, only try and seek
   * past the cut in either for either of these cases if the current clock time is greater than 2
   * seconds (arbitrary timeout).
   *
   * If this check is not performed the intended start time set will be overwritten if there is a
   * cut near the start of the file or playback will be jittery as the first cut that starts at 0
   * keeps trying to break out of an initial general resync cycle.
   */
  if((m_PlayerOptions.starttime > 0 || cut.start == 0)
     && clock < 2*1000) // 2 seconds in msec
    return;

  /*
   * TODO: 10 second cutoff between seeking and dropping packets seems a bit arbitrary. Is there
   * some reason that length of time seems to work well? Perhaps that's about the length of time
   * that is already cached.
   */
  if(cut.action == CEdl::CUT
  && cut.end - cut.start > 10*1000 // More efficient to drop packets than to seek if cut shorter than 10 seconds
  && !(cut.end == m_EdlAutoSkipMarkers.cut || cut.start == m_EdlAutoSkipMarkers.cut)) // To prevent looping if same cut again
  {
    CLog::Log(LOGDEBUG, "%s - Clock in EDL cut [%s - %s]: %s. Automatically skipping over.",
              __FUNCTION__, CEdl::MillisecondsToTimeString(cut.start).c_str(),
              CEdl::MillisecondsToTimeString(cut.end).c_str(), CEdl::MillisecondsToTimeString(clock).c_str());
    /*
     * Seeking either goes to the start or the end of the cut depending on the play direction.
     */
    __int64 seek = GetPlaySpeed() >= 0 ? cut.end : cut.start;
    /*
     * Seeking is flushed and accurate, just like SeekTime()
     */
    m_messenger.Put(new CDVDMsgPlayerSeek((int)seek, GetPlaySpeed() < 0, true, true, false));
    /*
     * Seek doesn't always work reliably. Last physical seek time is recorded to prevent looping
     * if there was an error with seeking and it landed somewhere unexpected, perhaps back in the
     * cut. The cut automatic skip marker is reset every 500ms allowing another attempt at the seek.
     */
    m_EdlAutoSkipMarkers.cut = GetPlaySpeed() >= 0 ? cut.end : cut.start;
  }
  else if(cut.action == CEdl::COMM_BREAK
  &&      GetPlaySpeed() >= 0
  &&      cut.start > m_EdlAutoSkipMarkers.commbreak_end)
  {
    CLog::Log(LOGDEBUG, "%s - Clock in commercial break [%s - %s]: %s. Automatically skipping to end of commercial break (only done once per break)",
              __FUNCTION__, CEdl::MillisecondsToTimeString(cut.start).c_str(), CEdl::MillisecondsToTimeString(cut.end).c_str(),
              CEdl::MillisecondsToTimeString(clock).c_str());
    /*
     * Seeking is flushed and inaccurate, just like Seek()
     */
    m_messenger.Put(new CDVDMsgPlayerSeek(cut.end + 1, false, true, false, false));
    /*
     * Each commercial break is only skipped once so poorly detected commercial breaks can be
     * manually re-entered. Start and end are recorded to prevent looping and to allow seeking back
     * to the start of the commercial break if incorrectly flagged.
     */
    m_EdlAutoSkipMarkers.commbreak_start = cut.start;
    m_EdlAutoSkipMarkers.commbreak_end   = cut.end;
    m_EdlAutoSkipMarkers.seek_to_start   = true; // Allow backwards Seek() to go directly to the start
  }

  /*
   * Reset the EDL automatic skip cut marker every 500 ms.
   */
  m_EdlAutoSkipMarkers.ResetCutMarker(500); // in msec
}


void CDVDPlayer::SynchronizeDemuxer(DWORD timeout)
{
  if(IsCurrentThread())
    return;
  if(!m_messenger.IsInited())
    return;

  CDVDMsgGeneralSynchronize* message = new CDVDMsgGeneralSynchronize(timeout, 0);
  m_messenger.Put(message->Acquire());
  message->Wait(&m_bStop, 0);
  message->Release();
}

void CDVDPlayer::SynchronizePlayers(DWORD sources, double pts)
{
  /* if we are awaiting a start sync, we can't sync here or we could deadlock */
  if(m_CurrentAudio.startsync
  || m_CurrentVideo.startsync
  || m_CurrentSubtitle.startsync
  || m_CurrentTeletext.startsync)
  {
    CLog::Log(LOGDEBUG, "%s - can't sync since we are already awaiting a sync", __FUNCTION__);
    return;
  }

  /* we need a big timeout as audio queue is about 8seconds for 2ch ac3 */
  const int timeout = 10*1000; // in milliseconds

  CDVDMsgGeneralSynchronize* message = new CDVDMsgGeneralSynchronize(timeout, sources);
  if (m_CurrentAudio.id >= 0)
  {
    m_CurrentAudio.dts = DVD_NOPTS_VALUE;
    m_CurrentAudio.startpts  = pts;
    m_CurrentAudio.startsync = message->Acquire();
  }
  if (m_CurrentVideo.id >= 0)
  {
    m_CurrentVideo.dts = DVD_NOPTS_VALUE;
    m_CurrentVideo.startpts  = pts;
    m_CurrentVideo.startsync = message->Acquire();
  }
/* TODO - we have to rewrite the sync class, to not require
          all other players waiting for subtitle, should only
          be the oposite way
  if (m_CurrentSubtitle.id >= 0)
  {
    m_CurrentSubtitle.dts = DVD_NOPTS_VALUE;
    m_CurrentSubtitle.startpts  = pts;
    m_CurrentSubtitle.startsync = message->Acquire();
  }
*/
  message->Release();
}

void CDVDPlayer::SendPlayerMessage(CDVDMsg* pMsg, unsigned int target)
{
  if(target == DVDPLAYER_AUDIO)
    m_dvdPlayerAudio.SendMessage(pMsg);
  if(target == DVDPLAYER_VIDEO)
    m_dvdPlayerVideo.SendMessage(pMsg);
  if(target == DVDPLAYER_SUBTITLE)
    m_dvdPlayerSubtitle.SendMessage(pMsg);
  if(target == DVDPLAYER_TELETEXT)
    m_dvdPlayerTeletext.SendMessage(pMsg);
}

void CDVDPlayer::OnExit()
{
  g_dvdPerformanceCounter.DisableMainPerformance();

  try
  {
    CLog::Log(LOGNOTICE, "CDVDPlayer::OnExit()");

    // set event to inform openfile something went wrong in case openfile is still waiting for this event
    SetCaching(CACHESTATE_DONE);

    // close each stream
    if (!m_bAbortRequest) CLog::Log(LOGNOTICE, "DVDPlayer: eof, waiting for queues to empty");
    if (m_CurrentAudio.id >= 0)
    {
      CLog::Log(LOGNOTICE, "DVDPlayer: closing audio stream");
      CloseAudioStream(!m_bAbortRequest);
    }
    if (m_CurrentVideo.id >= 0)
    {
      CLog::Log(LOGNOTICE, "DVDPlayer: closing video stream");
#ifdef HAVE_LIBVDPAU
      if (!g_VDPAU)
#endif
      CloseVideoStream(!m_bAbortRequest);
    }
    if (m_CurrentSubtitle.id >= 0)
    {
      CLog::Log(LOGNOTICE, "DVDPlayer: closing subtitle stream");
      CloseSubtitleStream(!m_bAbortRequest);
    }
    if (m_CurrentTeletext.id >= 0)
    {
      CLog::Log(LOGNOTICE, "DVDPlayer: closing teletext stream");
      CloseTeletextStream(!m_bAbortRequest);
    }
    // destroy the demuxer
    if (m_pDemuxer)
    {
      CLog::Log(LOGNOTICE, "CDVDPlayer::OnExit() deleting demuxer");
      delete m_pDemuxer;
    }
    m_pDemuxer = NULL;

    if (m_pSubtitleDemuxer)
    {
      CLog::Log(LOGNOTICE, "CDVDPlayer::OnExit() deleting subtitle demuxer");
      delete m_pSubtitleDemuxer;
    }
    m_pSubtitleDemuxer = NULL;

    // destroy the inputstream
    if (m_pInputStream)
    {
      CLog::Log(LOGNOTICE, "CDVDPlayer::OnExit() deleting input stream");
      delete m_pInputStream;
    }
    m_pInputStream = NULL;

    // clean up all selection streams
    m_SelectionStreams.Clear(STREAM_NONE, STREAM_SOURCE_NONE);

    m_messenger.End();

  }
  catch (...)
  {
    CLog::Log(LOGERROR, "%s - Exception thrown when trying to close down player, memory leak will follow", __FUNCTION__);
    m_pInputStream = NULL;
    m_pDemuxer = NULL;
  }

  m_bStop = true;
  // if we didn't stop playing, advance to the next item in xbmc's playlist
  if(m_PlayerOptions.identify == false)
  {
    if (m_bAbortRequest)
      m_callback.OnPlayBackStopped();
    else
      m_callback.OnPlayBackEnded();
  }

  // set event to inform openfile something went wrong in case openfile is still waiting for this event
  m_ready.Set();
}

void CDVDPlayer::HandleMessages()
{
  CDVDMsg* pMsg;

  MsgQueueReturnCode ret = m_messenger.Get(&pMsg, 0);

  while (ret == MSGQ_OK)
  {
    LockStreams();

    try
    {
      if (pMsg->IsType(CDVDMsg::PLAYER_SEEK))
      {
        CPlayerSeek m_pause(this);

        CDVDMsgPlayerSeek &msg(*((CDVDMsgPlayerSeek*)pMsg));
        double start = DVD_NOPTS_VALUE;

        int time = msg.GetRestore() ? (int)m_Edl.RestoreCutTime(msg.GetTime()) : msg.GetTime();
        CLog::Log(LOGDEBUG, "demuxer seek to: %d", time);
        if (m_pDemuxer && m_pDemuxer->SeekTime(time, msg.GetBackward(), &start))
        {
          CLog::Log(LOGDEBUG, "demuxer seek to: %d, success", time);
          if(m_pSubtitleDemuxer)
          {
            if(!m_pSubtitleDemuxer->SeekTime(time, msg.GetBackward()))
              CLog::Log(LOGDEBUG, "failed to seek subtitle demuxer: %d, success", time);
          }
          FlushBuffers(!msg.GetFlush());
          if(msg.GetAccurate())
            SynchronizePlayers(SYNCSOURCE_ALL, start);
          else
            SynchronizePlayers(SYNCSOURCE_ALL, DVD_NOPTS_VALUE);
        }
        else
          CLog::Log(LOGWARNING, "error while seeking");

        // set flag to indicate we have finished a seeking request
        g_infoManager.m_performingSeek = false;
      }
      else if (pMsg->IsType(CDVDMsg::PLAYER_SEEK_CHAPTER))
      {
        CPlayerSeek m_pause(this);

        CDVDMsgPlayerSeekChapter &msg(*((CDVDMsgPlayerSeekChapter*)pMsg));
        double start = DVD_NOPTS_VALUE;

        // This should always be the case.
        if(m_pDemuxer && m_pDemuxer->SeekChapter(msg.GetChapter(), &start))
        {
          FlushBuffers(false);
          SynchronizePlayers(SYNCSOURCE_ALL, start);
          m_callback.OnPlayBackSeekChapter(msg.GetChapter());
        }
      }
      else if (pMsg->IsType(CDVDMsg::DEMUXER_RESET))
      {
          m_CurrentAudio.stream = NULL;
          m_CurrentVideo.stream = NULL;
          m_CurrentSubtitle.stream = NULL;

          // we need to reset the demuxer, probably because the streams have changed
          if(m_pDemuxer)
            m_pDemuxer->Reset();
          if(m_pSubtitleDemuxer)
            m_pSubtitleDemuxer->Reset();
      }
      else if (pMsg->IsType(CDVDMsg::PLAYER_SET_AUDIOSTREAM))
      {
        CDVDMsgPlayerSetAudioStream* pMsg2 = (CDVDMsgPlayerSetAudioStream*)pMsg;

        SelectionStream& st = m_SelectionStreams.Get(STREAM_AUDIO, pMsg2->GetStreamId());
        if(st.source != STREAM_SOURCE_NONE)
        {
          if(st.source == STREAM_SOURCE_NAV && m_pInputStream && m_pInputStream->IsStreamType(DVDSTREAM_TYPE_DVD))
          {
            CDVDInputStreamNavigator* pStream = (CDVDInputStreamNavigator*)m_pInputStream;
            if(pStream->SetActiveAudioStream(st.id))
            {
              m_dvd.iSelectedAudioStream = -1;
              CloseAudioStream(false);
            }
          }
          else
          {
            CloseAudioStream(false);
            OpenAudioStream(st.id, st.source);
          }
        }
      }
      else if (pMsg->IsType(CDVDMsg::PLAYER_SET_SUBTITLESTREAM))
      {
        CDVDMsgPlayerSetSubtitleStream* pMsg2 = (CDVDMsgPlayerSetSubtitleStream*)pMsg;

        SelectionStream& st = m_SelectionStreams.Get(STREAM_SUBTITLE, pMsg2->GetStreamId());
        if(st.source != STREAM_SOURCE_NONE)
        {
          if(st.source == STREAM_SOURCE_NAV && m_pInputStream && m_pInputStream->IsStreamType(DVDSTREAM_TYPE_DVD))
          {
            CDVDInputStreamNavigator* pStream = (CDVDInputStreamNavigator*)m_pInputStream;
            if(pStream->SetActiveSubtitleStream(st.id))
            {
              m_dvd.iSelectedSPUStream = -1;
              CloseSubtitleStream(false);
            }
          }
          else
          {
            CloseSubtitleStream(false);
            OpenSubtitleStream(st.id, st.source);
          }
        }
      }
      else if (pMsg->IsType(CDVDMsg::PLAYER_SET_SUBTITLESTREAM_VISIBLE))
      {
        CDVDMsgBool* pValue = (CDVDMsgBool*)pMsg;

        m_dvdPlayerVideo.EnableSubtitle(pValue->m_value);

        if (m_pInputStream && m_pInputStream->IsStreamType(DVDSTREAM_TYPE_DVD))
          static_cast<CDVDInputStreamNavigator*>(m_pInputStream)->EnableSubtitleStream(pValue->m_value);
      }
      else if (pMsg->IsType(CDVDMsg::PLAYER_SET_STATE))
      {
        CPlayerSeek m_pause(this);

        CDVDMsgPlayerSetState* pMsgPlayerSetState = (CDVDMsgPlayerSetState*)pMsg;

        if (m_pInputStream && m_pInputStream->IsStreamType(DVDSTREAM_TYPE_DVD))
        {
          std::string s = pMsgPlayerSetState->GetState();
          ((CDVDInputStreamNavigator*)m_pInputStream)->SetNavigatorState(s);
          m_dvd.state = DVDSTATE_NORMAL;
          m_dvd.iDVDStillStartTime = 0;
          m_dvd.iDVDStillTime = 0;
        }
      }
      else if (pMsg->IsType(CDVDMsg::PLAYER_SET_RECORD))
      {
        CDVDInputStream::IChannel* input = dynamic_cast<CDVDInputStream::IChannel*>(m_pInputStream);
        if(input)
          input->Record(*(CDVDMsgBool*)pMsg);
      }
      else if (pMsg->IsType(CDVDMsg::GENERAL_FLUSH))
      {
        FlushBuffers(false);
      }
      else if (pMsg->IsType(CDVDMsg::PLAYER_SETSPEED))
      {
        int speed = static_cast<CDVDMsgInt*>(pMsg)->m_value;

        // correct our current clock, as it would start going wrong otherwise
        if(m_State.timestamp > 0)
        {
          double offset;
          offset  = CDVDClock::GetAbsoluteClock() - m_State.timestamp;
          offset *= m_playSpeed / DVD_PLAYSPEED_NORMAL;
          if(offset >  1000) offset =  1000;
          if(offset < -1000) offset = -1000;
          m_State.time     += DVD_TIME_TO_MSEC(offset);
          m_State.timestamp =  CDVDClock::GetAbsoluteClock();
        }

        if (speed != DVD_PLAYSPEED_PAUSE)
        {
          if (m_playSpeed != DVD_PLAYSPEED_PAUSE)
          {
            if ((m_playSpeed != DVD_PLAYSPEED_NORMAL * 32 && speed == DVD_PLAYSPEED_NORMAL) ||
                (m_playSpeed != DVD_PLAYSPEED_NORMAL * -32 && speed == DVD_PLAYSPEED_NORMAL))
              m_callback.OnPlayBackSpeedChanged(speed / DVD_PLAYSPEED_NORMAL );
          }
        }

        // if playspeed is different then DVD_PLAYSPEED_NORMAL or DVD_PLAYSPEED_PAUSE
        // audioplayer, stops outputing audio to audiorendere, but still tries to
        // sleep an correct amount for each packet
        // videoplayer just plays faster after the clock speed has been increased
        // 1. disable audio
        // 2. skip frames and adjust their pts or the clock
        m_playSpeed = speed;
        m_caching = CACHESTATE_DONE;
        m_clock.SetSpeed(speed);
        m_dvdPlayerAudio.SetSpeed(speed);
        m_dvdPlayerVideo.SetSpeed(speed);

        // TODO - we really shouldn't pause demuxer
        //        until our buffers are somewhat filled
        if(m_pDemuxer)
          m_pDemuxer->SetSpeed(speed);
      }
      else if (pMsg->IsType(CDVDMsg::PLAYER_CHANNEL_SELECT))
      {
        CPlayerSeek m_pause(this);

        CDVDInputStream::IChannel* input = dynamic_cast<CDVDInputStream::IChannel*>(m_pInputStream);
        if(input && input->SelectChannel(static_cast<CDVDMsgInt*>(pMsg)->m_value))
        {
          FlushBuffers(false);
          CloseAudioStream(true);
          CloseVideoStream(true);
          SAFE_DELETE(m_pDemuxer);
        }
      }
      else if (pMsg->IsType(CDVDMsg::PLAYER_CHANNEL_NEXT) || pMsg->IsType(CDVDMsg::PLAYER_CHANNEL_PREV))
      {
        CPlayerSeek m_pause(this);

        CDVDInputStream::IChannel* input = dynamic_cast<CDVDInputStream::IChannel*>(m_pInputStream);
        if(input)
        {
          bool result;
          bool fastSwitch = g_guiSettings.GetInt("pvrplayback.channelentrytimeout") > 0;
          if(pMsg->IsType(CDVDMsg::PLAYER_CHANNEL_NEXT))
            result = input->NextChannel(fastSwitch);
          else
            result = input->PrevChannel(fastSwitch);

          if(result)
          {
            if (fastSwitch)
            {
              CFileItem item(g_application.CurrentFileItem());
              if(input->UpdateItem(item))
              {
                g_application.CurrentFileItem() = item;
                g_infoManager.SetCurrentItem(item);
              }
              m_ChannelEntryTimeOut = CTimeUtils::GetTimeMS();
            }
            else
            {
              m_ChannelEntryTimeOut = 0;
              FlushBuffers(false);
              CloseAudioStream(true);
              CloseVideoStream(true);
              SAFE_DELETE(m_pDemuxer);
            }
          }
        }
      }
      else if (pMsg->IsType(CDVDMsg::GENERAL_GUI_ACTION))
        OnAction(((CDVDMsgType<CAction>*)pMsg)->m_value);
      else if (pMsg->IsType(CDVDMsg::PLAYER_STARTED))
      {
        int player = ((CDVDMsgInt*)pMsg)->m_value;
        if(player == DVDPLAYER_AUDIO)
          m_CurrentAudio.started = true;
        if(player == DVDPLAYER_VIDEO)
          m_CurrentVideo.started = true;
        CLog::Log(LOGDEBUG, "CDVDPlayer::HandleMessages - player started %d", player);
      }
    }
    catch (...)
    {
      CLog::Log(LOGERROR, "%s - Exception thrown when handling message", __FUNCTION__);
    }

    UnlockStreams();

    pMsg->Release();
    ret = m_messenger.Get(&pMsg, 0);
  }
}

void CDVDPlayer::SetCaching(ECacheState state)
{
  if(m_caching == state)
    return;

  CLog::Log(LOGDEBUG, "CDVDPlayer::SetCaching - caching state %d", state);
  if(state == CACHESTATE_FULL
  || state == CACHESTATE_INIT)
  {
    m_clock.SetSpeed(DVD_PLAYSPEED_PAUSE);
    m_dvdPlayerAudio.SetSpeed(DVD_PLAYSPEED_PAUSE);
    m_dvdPlayerVideo.SetSpeed(DVD_PLAYSPEED_PAUSE);
  }

  if(state == CACHESTATE_PLAY
  ||(state == CACHESTATE_DONE && m_caching != CACHESTATE_PLAY))
  {
    m_clock.SetSpeed(m_playSpeed);
    m_dvdPlayerAudio.SetSpeed(m_playSpeed);
    m_dvdPlayerVideo.SetSpeed(m_playSpeed);
  }
  m_caching = state;
}

void CDVDPlayer::SetPlaySpeed(int speed)
{
  m_messenger.Put(new CDVDMsgInt(CDVDMsg::PLAYER_SETSPEED, speed));
  m_dvdPlayerAudio.SetSpeed(speed);
  m_dvdPlayerVideo.SetSpeed(speed);
  SynchronizeDemuxer(100);
}

void CDVDPlayer::Pause()
{
  if(m_playSpeed != DVD_PLAYSPEED_PAUSE && m_caching == CACHESTATE_FULL)
  {
    SetCaching(CACHESTATE_DONE);
    return;
  }

  // return to normal speed if it was paused before, pause otherwise
  if (m_playSpeed == DVD_PLAYSPEED_PAUSE)
  {
    SetPlaySpeed(DVD_PLAYSPEED_NORMAL);
    m_callback.OnPlayBackResumed();
  }
  else
  {
    SetPlaySpeed(DVD_PLAYSPEED_PAUSE);
    m_callback.OnPlayBackPaused();
  }
}

bool CDVDPlayer::IsPaused() const
{
  return (m_playSpeed == DVD_PLAYSPEED_PAUSE) || m_caching == CACHESTATE_FULL;
}

bool CDVDPlayer::HasVideo() const
{
  if (m_pInputStream)
  {
    if (m_pInputStream->IsStreamType(DVDSTREAM_TYPE_DVD) || m_CurrentVideo.id >= 0) return true;
  }
  return false;
}

bool CDVDPlayer::HasAudio() const
{
  return (m_CurrentAudio.id >= 0);
}

bool CDVDPlayer::IsPassthrough() const
{
  return m_dvdPlayerAudio.IsPassthrough();
}

bool CDVDPlayer::CanSeek()
{
  return GetTotalTime() > 0;
}

void CDVDPlayer::Seek(bool bPlus, bool bLargeStep)
{
#if 0
  // sadly this doesn't work for now, audio player must
  // drop packets at the same rate as we play frames
  if( m_playSpeed == DVD_PLAYSPEED_PAUSE && bPlus && !bLargeStep)
  {
    m_dvdPlayerVideo.StepFrame();
    return;
  }
#endif

  if(((bPlus && GetChapter() < GetChapterCount())
  || (!bPlus && GetChapter() > 1)) && bLargeStep)
  {
    if(bPlus)
      SeekChapter(GetChapter() + 1);
    else
      SeekChapter(GetChapter() - 1);
    return;
  }

  __int64 seek;
  if (g_advancedSettings.m_videoUseTimeSeeking && GetTotalTime() > 2*g_advancedSettings.m_videoTimeSeekForwardBig)
  {
    if (bLargeStep)
      seek = bPlus ? g_advancedSettings.m_videoTimeSeekForwardBig : g_advancedSettings.m_videoTimeSeekBackwardBig;
    else
      seek = bPlus ? g_advancedSettings.m_videoTimeSeekForward : g_advancedSettings.m_videoTimeSeekBackward;
    seek *= 1000;
    seek += GetTime();
  }
  else
  {
    float percent;
    if (bLargeStep)
      percent = bPlus ? g_advancedSettings.m_videoPercentSeekForwardBig : g_advancedSettings.m_videoPercentSeekBackwardBig;
    else
      percent = bPlus ? g_advancedSettings.m_videoPercentSeekForward : g_advancedSettings.m_videoPercentSeekBackward;
    seek = (__int64)(GetTotalTimeInMsec()*(GetPercentage()+percent)/100);
  }

  bool restore = true;
  if (m_Edl.HasCut())
  {
    /*
     * Alter the standard seek position based on whether any commercial breaks have been
     * automatically skipped.
     */
    const int clock = DVD_TIME_TO_MSEC(m_clock.GetClock());
    /*
     * If a backwards seek (either small or large) occurs within 10 seconds of the end of the last
     * automated commercial skip, then seek back to the start of the commercial break under the
     * assumption that it was flagged incorrectly. 10 seconds grace period is allowed in case the
     * watcher has to fumble around finding the remote. Only happens once per commercial break.
     */
    if (!bPlus && m_EdlAutoSkipMarkers.seek_to_start
    &&  clock >= m_EdlAutoSkipMarkers.commbreak_end
    &&  clock <= m_EdlAutoSkipMarkers.commbreak_end + 10*1000) // Only if within 10 seconds of the end (in msec)
    {
      CLog::Log(LOGDEBUG, "%s - Seeking back to start of commercial break [%s - %s] as backwards skip activated within 10 seconds of the automatic commercial skip (only done once per break).",
                __FUNCTION__, CEdl::MillisecondsToTimeString(m_EdlAutoSkipMarkers.commbreak_start).c_str(),
                CEdl::MillisecondsToTimeString(m_EdlAutoSkipMarkers.commbreak_end).c_str());
      seek = m_EdlAutoSkipMarkers.commbreak_start;
      restore = false;
      m_EdlAutoSkipMarkers.seek_to_start = false; // So this will only happen within the 10 second grace period once.
    }
    /*
     * If big skip forward within the last "reverted" commercial break, seek to the end of the
     * commercial break under the assumption that the break was incorrectly flagged and playback has
     * now reached the actual start of the commercial break. Assume that the end is flagged more
     * correctly than the landing point for a standard big skip (ends seem to be flagged more
     * accurately than the start).
     */
    else if (bPlus && bLargeStep
    &&       clock >= m_EdlAutoSkipMarkers.commbreak_start
    &&       clock <= m_EdlAutoSkipMarkers.commbreak_end)
    {
      CLog::Log(LOGDEBUG, "%s - Seeking to end of previously skipped commercial break [%s - %s] as big forwards skip activated within the break.",
                __FUNCTION__, CEdl::MillisecondsToTimeString(m_EdlAutoSkipMarkers.commbreak_start).c_str(),
                CEdl::MillisecondsToTimeString(m_EdlAutoSkipMarkers.commbreak_end).c_str());
      seek = m_EdlAutoSkipMarkers.commbreak_end;
      restore = false;
    }
  }

  m_messenger.Put(new CDVDMsgPlayerSeek((int)seek, !bPlus, true, false, restore));
  SynchronizeDemuxer(100);
  m_callback.OnPlayBackSeek((int)seek);
}

bool CDVDPlayer::SeekScene(bool bPlus)
{
  if (!m_Edl.HasSceneMarker())
    return false;

  /*
   * There is a 5 second grace period applied when seeking for scenes backwards. If there is no
   * grace period applied it is impossible to go backwards past a scene marker.
   */
  __int64 clock = GetTime();
  if (!bPlus && clock > 5 * 1000) // 5 seconds
    clock -= 5 * 1000;

  int64_t iScenemarker;
  if (m_Edl.GetNextSceneMarker(bPlus, clock, &iScenemarker))
  {
    /*
     * Seeking is flushed and inaccurate, just like Seek()
     */
    m_messenger.Put(new CDVDMsgPlayerSeek((int)iScenemarker, !bPlus, true, false, false));
    SynchronizeDemuxer(100);
    return true;
  }
  return false;
}

void CDVDPlayer::GetAudioInfo(CStdString& strAudioInfo)
{
  CSingleLock lock(m_StateSection);
  strAudioInfo.Format("D(%s) P(%s)", m_State.demux_audio.c_str()
                                   , m_dvdPlayerAudio.GetPlayerInfo().c_str());
}

void CDVDPlayer::GetVideoInfo(CStdString& strVideoInfo)
{
  CSingleLock lock(m_StateSection);
  strVideoInfo.Format("D(%s) P(%s)", m_State.demux_video.c_str()
                                   , m_dvdPlayerVideo.GetPlayerInfo().c_str());
}

void CDVDPlayer::GetGeneralInfo(CStdString& strGeneralInfo)
{
  if (!m_bStop)
  {
    double dDelay = (double)m_dvdPlayerVideo.GetDelay() / DVD_TIME_BASE;

    double apts = m_dvdPlayerAudio.GetCurrentPts();
    double vpts = m_dvdPlayerVideo.GetCurrentPts();
    double dDiff = 0;

    if( apts != DVD_NOPTS_VALUE && vpts != DVD_NOPTS_VALUE )
      dDiff = (apts - vpts) / DVD_TIME_BASE;

    CStdString strEDL;
    strEDL.AppendFormat(", edl:%s", m_Edl.GetInfo().c_str());

    strGeneralInfo.Format("C( ad:% 6.3f, a/v:% 6.3f%s, dcpu:%2i%% acpu:%2i%% vcpu:%2i%% )"
                         , dDelay
                         , dDiff
                         , strEDL.c_str()
                         , (int)(CThread::GetRelativeUsage()*100)
                         , (int)(m_dvdPlayerAudio.GetRelativeUsage()*100)
                         , (int)(m_dvdPlayerVideo.GetRelativeUsage()*100));
  }
}

void CDVDPlayer::SeekPercentage(float iPercent)
{
  __int64 iTotalTime = GetTotalTimeInMsec();

  if (!iTotalTime)
    return;

  SeekTime((__int64)(iTotalTime * iPercent / 100));
}

float CDVDPlayer::GetPercentage()
{
  __int64 iTotalTime = GetTotalTimeInMsec();

  if (!iTotalTime)
    return 0.0f;

  return GetTime() * 100 / (float)iTotalTime;
}

void CDVDPlayer::SetAVDelay(float fValue)
{
  m_dvdPlayerVideo.SetDelay( (fValue * DVD_TIME_BASE) ) ;
}

float CDVDPlayer::GetAVDelay()
{
  return m_dvdPlayerVideo.GetDelay() / (float)DVD_TIME_BASE;
}

void CDVDPlayer::SetSubTitleDelay(float fValue)
{
  m_dvdPlayerVideo.SetSubtitleDelay(-fValue * DVD_TIME_BASE);
}

float CDVDPlayer::GetSubTitleDelay()
{
  return -m_dvdPlayerVideo.GetSubtitleDelay() / DVD_TIME_BASE;
}

// priority: 1: libdvdnav, 2: external subtitles, 3: muxed subtitles
int CDVDPlayer::GetSubtitleCount()
{
  LockStreams();
  m_SelectionStreams.Update(m_pInputStream, m_pDemuxer);
  UnlockStreams();
  return m_SelectionStreams.Count(STREAM_SUBTITLE);
}

int CDVDPlayer::GetSubtitle()
{
  return m_SelectionStreams.IndexOf(STREAM_SUBTITLE, *this);
}

void CDVDPlayer::GetSubtitleName(int iStream, CStdString &strStreamName)
{
  strStreamName = "";
  SelectionStream& s = m_SelectionStreams.Get(STREAM_SUBTITLE, iStream);
  if(s.name.length() > 0)
    strStreamName = s.name;
  else
    strStreamName = "Unknown";

  if(s.type == STREAM_NONE)
    strStreamName += "(Invalid)";
}

void CDVDPlayer::SetSubtitle(int iStream)
{
  m_messenger.Put(new CDVDMsgPlayerSetSubtitleStream(iStream));
}

bool CDVDPlayer::GetSubtitleVisible()
{
  if (m_pInputStream && m_pInputStream->IsStreamType(DVDSTREAM_TYPE_DVD))
  {
    CDVDInputStreamNavigator* pStream = (CDVDInputStreamNavigator*)m_pInputStream;
    if(pStream->IsInMenu())
      return g_settings.m_currentVideoSettings.m_SubtitleOn;
    else
      return pStream->IsSubtitleStreamEnabled();
  }

  return m_dvdPlayerVideo.IsSubtitleEnabled();
}

void CDVDPlayer::SetSubtitleVisible(bool bVisible)
{
  g_settings.m_currentVideoSettings.m_SubtitleOn = bVisible;
  m_messenger.Put(new CDVDMsgBool(CDVDMsg::PLAYER_SET_SUBTITLESTREAM_VISIBLE, bVisible));
}

int CDVDPlayer::GetAudioStreamCount()
{
  LockStreams();
  m_SelectionStreams.Update(m_pInputStream, m_pDemuxer);
  UnlockStreams();
  return m_SelectionStreams.Count(STREAM_AUDIO);
}

int CDVDPlayer::GetAudioStream()
{
  return m_SelectionStreams.IndexOf(STREAM_AUDIO, *this);
}

void CDVDPlayer::GetAudioStreamName(int iStream, CStdString& strStreamName)
{
  strStreamName = "";
  SelectionStream& s = m_SelectionStreams.Get(STREAM_AUDIO, iStream);
  if(s.name.length() > 0)
    strStreamName += s.name;
  else
    strStreamName += "Unknown";

  if(s.type == STREAM_NONE)
    strStreamName += " (Invalid)";
}

void CDVDPlayer::SetAudioStream(int iStream)
{
  m_messenger.Put(new CDVDMsgPlayerSetAudioStream(iStream));
  SynchronizeDemuxer(100);
}

TextCacheStruct_t* CDVDPlayer::GetTeletextCache()
{
  if (m_CurrentTeletext.id < 0)
    return 0;

  return m_dvdPlayerTeletext.GetTeletextCache();
}

void CDVDPlayer::LoadPage(int p, int sp, unsigned char* buffer)
{
  if (m_CurrentTeletext.id < 0)
      return;

  return m_dvdPlayerTeletext.LoadPage(p, sp, buffer);
}

void CDVDPlayer::SeekTime(__int64 iTime)
{
  m_messenger.Put(new CDVDMsgPlayerSeek((int)iTime, true, true, true));
  SynchronizeDemuxer(100);
  m_callback.OnPlayBackSeek((int)iTime);
}

// return the time in milliseconds
__int64 CDVDPlayer::GetTime()
{
  CSingleLock lock(m_StateSection);
  double offset = 0;
  if(m_State.timestamp > 0)
  {
    offset  = CDVDClock::GetAbsoluteClock() - m_State.timestamp;
    offset *= m_playSpeed / DVD_PLAYSPEED_NORMAL;
    if(offset >  1000) offset =  1000;
    if(offset < -1000) offset = -1000;
  }
  return llrint(m_State.time + DVD_TIME_TO_MSEC(offset));
}

// return length in msec
__int64 CDVDPlayer::GetTotalTimeInMsec()
{
  CSingleLock lock(m_StateSection);
  return llrint(m_State.time_total);
}

// return length in seconds.. this should be changed to return in milleseconds throughout xbmc
int CDVDPlayer::GetTotalTime()
{
  return (int)(GetTotalTimeInMsec() / 1000);
}

void CDVDPlayer::ToFFRW(int iSpeed)
{
  // can't rewind in menu as seeking isn't possible
  // forward is fine
  if (iSpeed < 0 && IsInMenu()) return;
  SetPlaySpeed(iSpeed * DVD_PLAYSPEED_NORMAL);
}

bool CDVDPlayer::OpenAudioStream(int iStream, int source)
{
  CLog::Log(LOGNOTICE, "Opening audio stream: %i source: %i", iStream, source);

  if (!m_pDemuxer)
    return false;

  CDemuxStream* pStream = m_pDemuxer->GetStream(iStream);
  if (!pStream || pStream->disabled)
    return false;

  if( m_CurrentAudio.id < 0 &&  m_CurrentVideo.id >= 0 )
  {
    // up until now we wheren't playing audio, but we did play video
    // this will change what is used to sync the dvdclock.
    // since the new audio data doesn't have to have any relation
    // to the current video data in the packet que, we have to
    // wait for it to empty

    // this happens if a new cell has audio data, but previous didn't
    // and both have video data

    SynchronizePlayers(SYNCSOURCE_AUDIO);
  }

  CDVDStreamInfo hint(*pStream, true);

  if(m_CurrentAudio.id    < 0
  || m_CurrentAudio.hint != hint)
  {
    if(m_CurrentAudio.id >= 0)
    {
      CLog::Log(LOGDEBUG, " - codecs hints have changed, must close previous stream");
      CloseAudioStream(true);
    }

    if (!m_dvdPlayerAudio.OpenStream( hint ))
    {
      /* mark stream as disabled, to disallaw further attempts*/
      CLog::Log(LOGWARNING, "%s - Unsupported stream %d. Stream disabled.", __FUNCTION__, iStream);
      pStream->disabled = true;
      pStream->SetDiscard(AVDISCARD_ALL);
      return false;
    }
  }
  else
    m_dvdPlayerAudio.SendMessage(new CDVDMsg(CDVDMsg::GENERAL_RESET));

  /* store information about stream */
  m_CurrentAudio.id = iStream;
  m_CurrentAudio.source = source;
  m_CurrentAudio.hint = hint;
  m_CurrentAudio.stream = (void*)pStream;
  m_CurrentAudio.started = false;

  /* audio normally won't consume full cpu, so let it have prio */
  m_dvdPlayerAudio.SetPriority(GetThreadPriority(*this)+1);

  return true;
}

bool CDVDPlayer::OpenVideoStream(int iStream, int source)
{
  CLog::Log(LOGNOTICE, "Opening video stream: %i source: %i", iStream, source);

  if (!m_pDemuxer)
    return false;

  CDemuxStream* pStream = m_pDemuxer->GetStream(iStream);
  if(!pStream || pStream->disabled)
    return false;
  pStream->SetDiscard(AVDISCARD_NONE);

  CDVDStreamInfo hint(*pStream, true);

  if( m_pInputStream && m_pInputStream->IsStreamType(DVDSTREAM_TYPE_DVD) )
  {
    /* set aspect ratio as requested by navigator for dvd's */
    float aspect = static_cast<CDVDInputStreamNavigator*>(m_pInputStream)->GetVideoAspectRatio();
    if(aspect != 0.0)
      hint.aspect = aspect;
  }

  if(m_CurrentVideo.id    < 0
  || m_CurrentVideo.hint != hint)
  {
    if(m_CurrentVideo.id >= 0)
    {
      CLog::Log(LOGDEBUG, " - codecs hints have changed, must close previous stream");
      CloseVideoStream(true);
    }

    if (!m_dvdPlayerVideo.OpenStream(hint))
    {
      /* mark stream as disabled, to disallaw further attempts */
      CLog::Log(LOGWARNING, "%s - Unsupported stream %d. Stream disabled.", __FUNCTION__, iStream);
      pStream->disabled = true;
      pStream->SetDiscard(AVDISCARD_ALL);
      return false;
    }
  }
  else
    m_dvdPlayerVideo.SendMessage(new CDVDMsg(CDVDMsg::GENERAL_RESET));

  /* store information about stream */
  m_CurrentVideo.id = iStream;
  m_CurrentVideo.source = source;
  m_CurrentVideo.hint = hint;
  m_CurrentVideo.stream = (void*)pStream;
  m_CurrentVideo.started = false;

  /* use same priority for video thread as demuxing thread, as */
  /* otherwise demuxer will starve if video consumes the full cpu */
  m_dvdPlayerVideo.SetPriority(GetThreadPriority(*this));
  return true;

}

bool CDVDPlayer::OpenSubtitleStream(int iStream, int source)
{
  CLog::Log(LOGNOTICE, "Opening Subtitle stream: %i source: %i", iStream, source);

  CDemuxStream* pStream = NULL;
  std::string filename;
  CDVDStreamInfo hint;

  if(STREAM_SOURCE_MASK(source) == STREAM_SOURCE_DEMUX_SUB)
  {
    int index = m_SelectionStreams.IndexOf(STREAM_SUBTITLE, source, iStream);
    if(index < 0)
      return false;
    SelectionStream st = m_SelectionStreams.Get(STREAM_SUBTITLE, index);

    if(!m_pSubtitleDemuxer || m_pSubtitleDemuxer->GetFileName() != st.filename)
    {
      CLog::Log(LOGNOTICE, "Opening Subtitle file: %s", st.filename.c_str());
      auto_ptr<CDVDDemuxVobsub> demux(new CDVDDemuxVobsub());
      if(!demux->Open(st.filename))
        return false;
      m_pSubtitleDemuxer = demux.release();
    }

    pStream = m_pSubtitleDemuxer->GetStream(iStream);
    if(!pStream || pStream->disabled)
      return false;
    pStream->SetDiscard(AVDISCARD_NONE);
    double pts = m_dvdPlayerVideo.GetCurrentPts();
    if(pts == DVD_NOPTS_VALUE)
      pts = m_CurrentVideo.dts;
    if(pts != DVD_NOPTS_VALUE)
      m_pSubtitleDemuxer->SeekTime((int)(1000.0 * pts / (double)DVD_TIME_BASE));

    hint.Assign(*pStream, true);
  }
  else if(STREAM_SOURCE_MASK(source) == STREAM_SOURCE_TEXT)
  {
    int index = m_SelectionStreams.IndexOf(STREAM_SUBTITLE, source, iStream);
    if(index < 0)
      return false;
    filename = m_SelectionStreams.Get(STREAM_SUBTITLE, index).filename;

    hint.Clear();
    hint.fpsscale = m_CurrentVideo.hint.fpsscale;
    hint.fpsrate  = m_CurrentVideo.hint.fpsrate;
  }
  else
  {
    if(!m_pDemuxer)
      return false;
    pStream = m_pDemuxer->GetStream(iStream);
    if(!pStream || pStream->disabled)
      return false;
    pStream->SetDiscard(AVDISCARD_NONE);

    hint.Assign(*pStream, true);

    if(m_pInputStream && m_pInputStream->IsStreamType(DVDSTREAM_TYPE_DVD))
      filename = "dvd";
  }

  if(m_CurrentSubtitle.id    < 0
  || m_CurrentSubtitle.hint != hint)
  {
    if(m_CurrentSubtitle.id >= 0)
    {
      CLog::Log(LOGDEBUG, " - codecs hints have changed, must close previous stream");
      CloseSubtitleStream(false);
    }

    if(!m_dvdPlayerSubtitle.OpenStream(hint, filename))
    {
      CLog::Log(LOGWARNING, "%s - Unsupported stream %d. Stream disabled.", __FUNCTION__, iStream);
      if(pStream)
      {
        pStream->disabled = true;
        pStream->SetDiscard(AVDISCARD_ALL);
      }
      return false;
    }
  }
  else
    m_dvdPlayerSubtitle.SendMessage(new CDVDMsg(CDVDMsg::GENERAL_RESET));

  m_CurrentSubtitle.id     = iStream;
  m_CurrentSubtitle.source = source;
  m_CurrentSubtitle.hint   = hint;
  m_CurrentSubtitle.stream = (void*)pStream;
  m_CurrentSubtitle.started = false;

  return true;
}

bool CDVDPlayer::OpenTeletextStream(int iStream, int source)
{
  if (!m_pDemuxer)
    return false;

  CDemuxStream* pStream = m_pDemuxer->GetStream(iStream);
  if(!pStream || pStream->disabled)
    return false;

  CDVDStreamInfo hint(*pStream, true);

  if (!m_dvdPlayerTeletext.CheckStream(hint))
    return false;

  CLog::Log(LOGNOTICE, "Opening teletext stream: %i source: %i", iStream, source);

  if(m_CurrentTeletext.id    < 0
  || m_CurrentTeletext.hint != hint)
  {
    if(m_CurrentTeletext.id >= 0)
    {
      CLog::Log(LOGDEBUG, " - teletext codecs hints have changed, must close previous stream");
      CloseTeletextStream(true);
    }

    if (!m_dvdPlayerTeletext.OpenStream(hint))
    {
      /* mark stream as disabled, to disallaw further attempts*/
      CLog::Log(LOGWARNING, "%s - Unsupported teletext stream %d. Stream disabled.", __FUNCTION__, iStream);
      pStream->disabled = true;
      pStream->SetDiscard(AVDISCARD_ALL);
      return false;
    }
  }
  else
    m_dvdPlayerTeletext.SendMessage(new CDVDMsg(CDVDMsg::GENERAL_RESET));

  /* store information about stream */
  m_CurrentTeletext.id      = iStream;
  m_CurrentTeletext.source  = source;
  m_CurrentTeletext.hint    = hint;
  m_CurrentTeletext.stream  = (void*)pStream;
  m_CurrentTeletext.started = false;

  return true;
}

bool CDVDPlayer::CloseAudioStream(bool bWaitForBuffers)
{
  if (m_CurrentAudio.id < 0)
    return false;

  CLog::Log(LOGNOTICE, "Closing audio stream");

  m_dvdPlayerAudio.CloseStream(bWaitForBuffers);

  m_CurrentAudio.Clear();
  return true;
}

bool CDVDPlayer::CloseVideoStream(bool bWaitForBuffers)
{
  if (m_CurrentVideo.id < 0)
    return false;

  CLog::Log(LOGNOTICE, "Closing video stream");

  m_dvdPlayerVideo.CloseStream(bWaitForBuffers);

  m_CurrentVideo.Clear();
  return true;
}

bool CDVDPlayer::CloseSubtitleStream(bool bKeepOverlays)
{
  if (m_CurrentSubtitle.id < 0)
    return false;

  CLog::Log(LOGNOTICE, "Closing subtitle stream");

  m_dvdPlayerSubtitle.CloseStream(!bKeepOverlays);

  m_CurrentSubtitle.Clear();
  return true;
}

bool CDVDPlayer::CloseTeletextStream(bool bWaitForBuffers)
{
  if (m_CurrentTeletext.id < 0)
    return false;

  CLog::Log(LOGNOTICE, "Closing teletext stream");

  m_dvdPlayerTeletext.CloseStream(bWaitForBuffers);

  m_CurrentTeletext.Clear();
  return true;
}

void CDVDPlayer::FlushBuffers(bool queued)
{
  if(queued)
  {
    m_dvdPlayerAudio.SendMessage(new CDVDMsg(CDVDMsg::GENERAL_RESET));
    m_dvdPlayerVideo.SendMessage(new CDVDMsg(CDVDMsg::GENERAL_RESET));
    m_dvdPlayerVideo.SendMessage(new CDVDMsg(CDVDMsg::VIDEO_NOSKIP));
    m_dvdPlayerSubtitle.SendMessage(new CDVDMsg(CDVDMsg::GENERAL_RESET));
    m_dvdPlayerTeletext.SendMessage(new CDVDMsg(CDVDMsg::GENERAL_RESET));
    SynchronizePlayers(SYNCSOURCE_ALL);
  }
  else
  {
    m_dvdPlayerAudio.Flush();
    m_dvdPlayerVideo.Flush();
    m_dvdPlayerSubtitle.Flush();
    m_dvdPlayerTeletext.Flush();

    // clear subtitle and menu overlays
    m_overlayContainer.Clear();

    // make sure players are properly flushed, should put them in stalled state
    CDVDMsgGeneralSynchronize* msg = new CDVDMsgGeneralSynchronize(1000, 0);
    m_dvdPlayerAudio.m_messageQueue.Put(msg->Acquire(), 1);
    m_dvdPlayerVideo.m_messageQueue.Put(msg->Acquire(), 1);
    msg->Wait(&m_bStop, 0);
    msg->Release();

    // we should now wait for init cache
    SetCaching(CACHESTATE_INIT);
    m_CurrentAudio.started    = false;
    m_CurrentVideo.started    = false;
    m_CurrentSubtitle.started = false;
    m_CurrentTeletext.started = false;
  }
  m_CurrentAudio.inited = false;
  m_CurrentVideo.inited = false;
  m_CurrentSubtitle.inited = false;
  m_CurrentTeletext.inited = false;
}

// since we call ffmpeg functions to decode, this is being called in the same thread as ::Process() is
int CDVDPlayer::OnDVDNavResult(void* pData, int iMessage)
{
  if (m_pInputStream->IsStreamType(DVDSTREAM_TYPE_DVD))
  {
    CDVDInputStreamNavigator* pStream = (CDVDInputStreamNavigator*)m_pInputStream;

    switch (iMessage)
    {
    case DVDNAV_STILL_FRAME:
      {
        //CLog::Log(LOGDEBUG, "DVDNAV_STILL_FRAME");

        dvdnav_still_event_t *still_event = (dvdnav_still_event_t *)pData;
        // should wait the specified time here while we let the player running
        // after that call dvdnav_still_skip(m_dvdnav);

        if (m_dvd.state != DVDSTATE_STILL)
        {
          // else notify the player we have received a still frame

          if(still_event->length < 0xff)
            m_dvd.iDVDStillTime = still_event->length * 1000;
          else
            m_dvd.iDVDStillTime = 0;

          m_dvd.iDVDStillStartTime = CTimeUtils::GetTimeMS();

          /* adjust for the output delay in the video queue */
          DWORD time = 0;
          if( m_CurrentVideo.stream && m_dvd.iDVDStillTime > 0 )
          {
            time = (DWORD)(m_dvdPlayerVideo.GetOutputDelay() / ( DVD_TIME_BASE / 1000 ));
            if( time < 10000 && time > 0 )
              m_dvd.iDVDStillTime += time;
          }
          m_dvd.state = DVDSTATE_STILL;
          CLog::Log(LOGDEBUG,
                    "DVDNAV_STILL_FRAME - waiting %i sec, with delay of %d sec",
                    still_event->length, time / 1000);
        }
        return NAVRESULT_HOLD;
      }
      break;
    case DVDNAV_SPU_CLUT_CHANGE:
      {
        CLog::Log(LOGDEBUG, "DVDNAV_SPU_CLUT_CHANGE");
        m_dvdPlayerSubtitle.SendMessage(new CDVDMsgSubtitleClutChange((BYTE*)pData));
      }
      break;
    case DVDNAV_SPU_STREAM_CHANGE:
      {
        CLog::Log(LOGDEBUG, "DVDNAV_SPU_STREAM_CHANGE");

        dvdnav_spu_stream_change_event_t* event = (dvdnav_spu_stream_change_event_t*)pData;

        int iStream = event->physical_wide;
        bool visible = !(iStream & 0x80);

        m_dvdPlayerVideo.EnableSubtitle(visible);

        if (iStream >= 0)
          m_dvd.iSelectedSPUStream = (iStream & ~0x80);
        else
          m_dvd.iSelectedSPUStream = -1;

        m_CurrentSubtitle.stream = NULL;
      }
      break;
    case DVDNAV_AUDIO_STREAM_CHANGE:
      {
        CLog::Log(LOGDEBUG, "DVDNAV_AUDIO_STREAM_CHANGE");

        // This should be the correct way i think, however we don't have any streams right now
        // since the demuxer hasn't started so it doesn't change. not sure how to do this.
        dvdnav_audio_stream_change_event_t* event = (dvdnav_audio_stream_change_event_t*)pData;

        // Tell system what audiostream should be opened by default
        if (event->logical >= 0)
          m_dvd.iSelectedAudioStream = event->physical;
        else
          m_dvd.iSelectedAudioStream = -1;

        m_CurrentAudio.stream = NULL;
      }
      break;
    case DVDNAV_HIGHLIGHT:
      {
        //dvdnav_highlight_event_t* pInfo = (dvdnav_highlight_event_t*)pData;
        int iButton = pStream->GetCurrentButton();
        CLog::Log(LOGDEBUG, "DVDNAV_HIGHLIGHT: Highlight button %d\n", iButton);
        m_dvdPlayerSubtitle.UpdateOverlayInfo((CDVDInputStreamNavigator*)m_pInputStream, LIBDVDNAV_BUTTON_NORMAL);
      }
      break;
    case DVDNAV_VTS_CHANGE:
      {
        //dvdnav_vts_change_event_t* vts_change_event = (dvdnav_vts_change_event_t*)pData;
        CLog::Log(LOGDEBUG, "DVDNAV_VTS_CHANGE");

        //Make sure we clear all the old overlays here, or else old forced items are left.
        m_overlayContainer.Clear();

        // reset the demuxer, this also imples closing the video and the audio system
        // this is a bit tricky cause it's the demuxer that's is making this call in the end
        // so we send a message to indicate the main loop that the demuxer needs a reset
        // this also means the libdvdnav may not return any data packets after this command
        m_messenger.Put(new CDVDMsgDemuxerReset());

        //Force an aspect ratio that is set in the dvdheaders if available
        m_CurrentVideo.hint.aspect = pStream->GetVideoAspectRatio();
        if( m_dvdPlayerAudio.m_messageQueue.IsInited() )
          m_dvdPlayerVideo.SendMessage(new CDVDMsgDouble(CDVDMsg::VIDEO_SET_ASPECT, m_CurrentVideo.hint.aspect));

        m_SelectionStreams.Clear(STREAM_NONE, STREAM_SOURCE_NAV);
        m_SelectionStreams.Update(m_pInputStream, m_pDemuxer);

        // we must hold here once more, otherwise the demuxer message
        // will be executed after demuxer have filled with data
        return NAVRESULT_HOLD;
      }
      break;
    case DVDNAV_CELL_CHANGE:
      {
        //dvdnav_cell_change_event_t* cell_change_event = (dvdnav_cell_change_event_t*)pData;
        CLog::Log(LOGDEBUG, "DVDNAV_CELL_CHANGE");

        m_dvd.state = DVDSTATE_NORMAL;

        if( m_dvdPlayerVideo.m_messageQueue.IsInited() )
          m_dvdPlayerVideo.SendMessage(new CDVDMsg(CDVDMsg::VIDEO_NOSKIP));
      }
      break;
    case DVDNAV_NAV_PACKET:
      {
          //pci_t* pci = (pci_t*)pData;

          // this should be possible to use to make sure we get
          // seamless transitions over these boundaries
          // if we remember the old vobunits boundaries
          // when a packet comes out of demuxer that has
          // pts values outside that boundary, it belongs
          // to the new vobunit, wich has new timestamps
      }
      break;
    case DVDNAV_HOP_CHANNEL:
      {
        // This event is issued whenever a non-seamless operation has been executed.
        // Applications with fifos should drop the fifos content to speed up responsiveness.
        CLog::Log(LOGDEBUG, "DVDNAV_HOP_CHANNEL");
        m_messenger.Put(new CDVDMsg(CDVDMsg::GENERAL_FLUSH));
        return NAVRESULT_ERROR;
      }
      break;
    case DVDNAV_STOP:
      {
        CLog::Log(LOGDEBUG, "DVDNAV_STOP");
        m_dvd.state = DVDSTATE_NORMAL;
      }
      break;
    default:
    {}
      break;
    }
  }
  return NAVRESULT_NOP;
}

bool CDVDPlayer::OnAction(const CAction &action)
{
#define THREAD_ACTION(action) \
  do { \
    if (!IsCurrentThread()) { \
      m_messenger.Put(new CDVDMsgType<CAction>(CDVDMsg::GENERAL_GUI_ACTION, action)); \
      return true; \
    } \
  } while(false)

  if (m_pInputStream && m_pInputStream->IsStreamType(DVDSTREAM_TYPE_DVD))
  {
    CDVDInputStreamNavigator* pStream = (CDVDInputStreamNavigator*)m_pInputStream;


    if( m_dvd.state == DVDSTATE_STILL && m_dvd.iDVDStillTime != 0 && pStream->GetTotalButtons() == 0 )
    {
      switch(action.GetID())
      {
        case ACTION_NEXT_ITEM:
        case ACTION_MOVE_RIGHT:
        case ACTION_MOVE_UP:
        case ACTION_SELECT_ITEM:
          {
            THREAD_ACTION(action);
            /* this will force us out of the stillframe */
            CLog::Log(LOGDEBUG, "%s - User asked to exit stillframe", __FUNCTION__);
            m_dvd.iDVDStillStartTime = 0;
            m_dvd.iDVDStillTime = 1;
          }
          return true;
      }
    }


    switch (action.GetID())
    {
/* this code is disabled to allow switching playlist items (dvdimage "stacks") */
#if 0
    case ACTION_PREV_ITEM:  // SKIP-:
      {
        THREAD_ACTION(action);
        CLog::Log(LOGDEBUG, " - pushed prev");
        pStream->OnPrevious();
        g_infoManager.SetDisplayAfterSeek();
        return true;
      }
      break;
    case ACTION_NEXT_ITEM:  // SKIP+:
      {
        THREAD_ACTION(action);
        CLog::Log(LOGDEBUG, " - pushed next");
        pStream->OnNext();
        g_infoManager.SetDisplayAfterSeek();
        return true;
      }
      break;
#endif
    case ACTION_SHOW_VIDEOMENU:   // start button
      {
        THREAD_ACTION(action);
        CLog::Log(LOGDEBUG, " - go to menu");
        pStream->OnMenu();
        // send a message to everyone that we've gone to the menu
        CGUIMessage msg(GUI_MSG_VIDEO_MENU_STARTED, 0, 0);
        g_windowManager.SendMessage(msg);
        return true;
      }
      break;
    }

    if (pStream->IsInMenu())
    {
      switch (action.GetID())
      {
      case ACTION_PREVIOUS_MENU:
        {
          THREAD_ACTION(action);
          CLog::Log(LOGDEBUG, " - menu back");
          pStream->OnBack();
        }
        break;
      case ACTION_MOVE_LEFT:
        {
          THREAD_ACTION(action);
          CLog::Log(LOGDEBUG, " - move left");
          pStream->OnLeft();
        }
        break;
      case ACTION_MOVE_RIGHT:
        {
          THREAD_ACTION(action);
          CLog::Log(LOGDEBUG, " - move right");
          pStream->OnRight();
        }
        break;
      case ACTION_MOVE_UP:
        {
          THREAD_ACTION(action);
          CLog::Log(LOGDEBUG, " - move up");
          pStream->OnUp();
        }
        break;
      case ACTION_MOVE_DOWN:
        {
          THREAD_ACTION(action);
          CLog::Log(LOGDEBUG, " - move down");
          pStream->OnDown();
        }
        break;

      case ACTION_MOUSE_MOVE:
      case ACTION_MOUSE_LEFT_CLICK:
        {
          CRect rs, rd;
          GetVideoRect(rs, rd);
          CPoint pt(action.GetAmount(), action.GetAmount(1));
          if (!rd.PtInRect(pt))
            return false; // out of bounds
          THREAD_ACTION(action);
          // convert to video coords...
          pt -= CPoint(rd.x1, rd.y1);
          pt.x *= rs.Width() / rd.Width();
          pt.y *= rs.Height() / rd.Height();
          pt += CPoint(rs.x1, rs.y1);
          if (action.GetID() == ACTION_MOUSE_LEFT_CLICK)
            return pStream->OnMouseClick(pt);
          return pStream->OnMouseMove(pt);
        }
        break;
      case ACTION_SELECT_ITEM:
        {
          THREAD_ACTION(action);
          CLog::Log(LOGDEBUG, " - button select");
          // show button pushed overlay
          m_dvdPlayerSubtitle.UpdateOverlayInfo((CDVDInputStreamNavigator*)m_pInputStream, LIBDVDNAV_BUTTON_CLICKED);

          pStream->ActivateButton();
        }
        break;
      case REMOTE_0:
      case REMOTE_1:
      case REMOTE_2:
      case REMOTE_3:
      case REMOTE_4:
      case REMOTE_5:
      case REMOTE_6:
      case REMOTE_7:
      case REMOTE_8:
      case REMOTE_9:
        {
          THREAD_ACTION(action);
          // Offset from key codes back to button number
          int button = action.GetID() - REMOTE_0;
          CLog::Log(LOGDEBUG, " - button pressed %d", button);
          pStream->SelectButton(button);
        }
       break;
      default:
        return false;
        break;
      }
      return true; // message is handled
    }
  }

  if (dynamic_cast<CDVDInputStream::IChannel*>(m_pInputStream))
  {
    switch (action.GetID())
    {
      case ACTION_MOVE_UP:
      case ACTION_NEXT_ITEM:
      case ACTION_PAGE_UP:
        m_messenger.Put(new CDVDMsg(CDVDMsg::PLAYER_CHANNEL_NEXT));
        g_infoManager.SetDisplayAfterSeek();
        if (g_guiSettings.GetBool("pvrmenu.infoswitch"))
        {
          CGUIDialogFullScreenInfo* pDialog = (CGUIDialogFullScreenInfo*)g_windowManager.GetWindow(WINDOW_DIALOG_FULLSCREEN_INFO);
          if (pDialog)
          {
            if (g_guiSettings.GetBool("pvrmenu.infotimeout"))
            {
              pDialog->SetAutoClose(g_guiSettings.GetInt("pvrmenu.infotime")*1000);
            }
            if (m_ChannelEntryTimeOut == 0)
              pDialog->DoModal();
          }
        }
        return true;
      break;

      case ACTION_MOVE_DOWN:
      case ACTION_PREV_ITEM:
      case ACTION_PAGE_DOWN:
        m_messenger.Put(new CDVDMsg(CDVDMsg::PLAYER_CHANNEL_PREV));
        g_infoManager.SetDisplayAfterSeek();
        if (g_guiSettings.GetBool("pvrmenu.infoswitch"))
        {
          CGUIDialogFullScreenInfo* pDialog = (CGUIDialogFullScreenInfo*)g_windowManager.GetWindow(WINDOW_DIALOG_FULLSCREEN_INFO);
          if (pDialog)
          {
            if (g_guiSettings.GetBool("pvrmenu.infotimeout"))
            {
              pDialog->SetAutoClose(g_guiSettings.GetInt("pvrmenu.infotime")*1000);
            }
            if (m_ChannelEntryTimeOut == 0)
              pDialog->DoModal();
          }
        }
        return true;
      break;

      case ACTION_CHANNEL_SWITCH:
      {
        // Offset from key codes back to button number
        int channel = action.GetAmount();
        m_messenger.Put(new CDVDMsgInt(CDVDMsg::PLAYER_CHANNEL_SELECT, channel));
        g_infoManager.SetDisplayAfterSeek();
        if (g_guiSettings.GetBool("pvrmenu.infoswitch"))
        {
          CGUIDialogFullScreenInfo* pDialog = (CGUIDialogFullScreenInfo*)g_windowManager.GetWindow(WINDOW_DIALOG_FULLSCREEN_INFO);
          if (pDialog)
          {
            if (g_guiSettings.GetBool("pvrmenu.infotimeout"))
            {
              pDialog->SetAutoClose(g_guiSettings.GetInt("pvrmenu.infotime")*1000);
            }
            pDialog->DoModal();
          }
        }
        return true;
      }
      break;
    }
  }

  switch (action.GetID())
  {
    case ACTION_NEXT_ITEM:
    case ACTION_PAGE_UP:
      if(GetChapterCount() > 0)
      {
        m_messenger.Put(new CDVDMsgPlayerSeekChapter(GetChapter()+1));
        g_infoManager.SetDisplayAfterSeek();
        return true;
      }
      else
        break;
    case ACTION_PREV_ITEM:
    case ACTION_PAGE_DOWN:
      if(GetChapterCount() > 0)
      {
        m_messenger.Put(new CDVDMsgPlayerSeekChapter(GetChapter()-1));
        g_infoManager.SetDisplayAfterSeek();
        return true;
      }
      else
        break;
  }

  // return false to inform the caller we didn't handle the message
  return false;
}

bool CDVDPlayer::IsInMenu() const
{
  if (m_pInputStream && m_pInputStream->IsStreamType(DVDSTREAM_TYPE_DVD))
  {
    CDVDInputStreamNavigator* pStream = (CDVDInputStreamNavigator*)m_pInputStream;
    if( m_dvd.state == DVDSTATE_STILL )
      return true;
    else
      return pStream->IsInMenu();
  }
  return false;
}

bool CDVDPlayer::HasMenu()
{
  if (m_pInputStream && m_pInputStream->IsStreamType(DVDSTREAM_TYPE_DVD))
    return true;
  else
    return false;
}

bool CDVDPlayer::GetCurrentSubtitle(CStdString& strSubtitle)
{
  double pts = m_clock.GetClock();

  if (m_pInputStream && m_pInputStream->IsStreamType(DVDSTREAM_TYPE_DVD))
    return false;

  return m_dvdPlayerSubtitle.GetCurrentSubtitle(strSubtitle, pts - m_dvdPlayerVideo.GetSubtitleDelay());
}

CStdString CDVDPlayer::GetPlayerState()
{
  CSingleLock lock(m_StateSection);
  return m_State.player_state;
}

bool CDVDPlayer::SetPlayerState(CStdString state)
{
  m_messenger.Put(new CDVDMsgPlayerSetState(state));
  return true;
}

int CDVDPlayer::GetChapterCount()
{
  CSingleLock lock(m_StateSection);
  return m_State.chapter_count;
}

int CDVDPlayer::GetChapter()
{
  CSingleLock lock(m_StateSection);
  return m_State.chapter;
}

void CDVDPlayer::GetChapterName(CStdString& strChapterName)
{
  CSingleLock lock(m_StateSection);
  strChapterName = m_State.chapter_name;
}

int CDVDPlayer::SeekChapter(int iChapter)
{
  if (GetChapterCount() > 0)
  {
    if (iChapter < 0)
      iChapter = 0;
    if (iChapter > GetChapterCount())
      return 0;

    // Seek to the chapter.
    m_messenger.Put(new CDVDMsgPlayerSeekChapter(iChapter));
    SynchronizeDemuxer(100);
  }
  else
  {
    // Do a regular big jump.
    if (GetChapter() > 0 && iChapter > GetChapter())
      Seek(true, true);
    else
      Seek(false, true);
  }
  return 0;
}

bool CDVDPlayer::AddSubtitle(const CStdString& strSubPath)
{
  return AddSubtitleFile(strSubPath);
}

int CDVDPlayer::GetCacheLevel() const
{
  int a = m_dvdPlayerAudio.m_messageQueue.GetLevel();
  int v = m_dvdPlayerVideo.m_messageQueue.GetLevel();
  return max(a, v);
}

int CDVDPlayer::GetAudioBitrate()
{
  return m_dvdPlayerAudio.GetAudioBitrate();
}

int CDVDPlayer::GetVideoBitrate()
{
  return m_dvdPlayerVideo.GetVideoBitrate();
}

int CDVDPlayer::GetSourceBitrate()
{
  if (m_pInputStream)
    return (int)m_pInputStream->GetBitstreamStats().GetBitrate();

  return 0;
}


bool CDVDPlayer::AddSubtitleFile(const std::string& filename)
{
  std::string ext = CUtil::GetExtension(filename);
  if(ext == ".idx")
  {
    CDVDDemuxVobsub v;
    if(!v.Open(filename))
      return false;

    m_SelectionStreams.Update(NULL, &v);
    return true;
  }
  if(ext == ".sub")
  {
    CStdString strReplace;
    CUtil::ReplaceExtension(filename,".idx",strReplace);
    if (XFILE::CFile::Exists(strReplace))
      return false;
  }
  SelectionStream s;
  s.source   = m_SelectionStreams.Source(STREAM_SOURCE_TEXT, filename);
  s.type     = STREAM_SUBTITLE;
  s.id       = 0;
  s.filename = filename;
  s.name     = CUtil::GetFileName(filename);
  m_SelectionStreams.Update(s);
  return true;
}

void CDVDPlayer::UpdatePlayState(double timeout)
{
  CSingleLock lock(m_StateSection);

  if(m_State.timestamp != 0
  && m_State.timestamp + DVD_MSEC_TO_TIME(timeout) > CDVDClock::GetAbsoluteClock())
    return;

  if(m_pDemuxer)
  {
    m_State.chapter       = m_pDemuxer->GetChapter();
    m_State.chapter_count = m_pDemuxer->GetChapterCount();
    m_pDemuxer->GetChapterName(m_State.chapter_name);

    m_State.time       = DVD_TIME_TO_MSEC(m_clock.GetClock());
    m_State.time_total = m_pDemuxer->GetStreamLength();
  }

  if(m_pInputStream)
  {
    // override from input stream if needed

    CDVDInputStream::IDisplayTime* pDisplayTime = dynamic_cast<CDVDInputStream::IDisplayTime*>(m_pInputStream);
    if (pDisplayTime)
    {
      m_State.time       = pDisplayTime->GetTime();
      m_State.time_total = pDisplayTime->GetTotalTime();
    }

    if (m_pInputStream->IsStreamType(DVDSTREAM_TYPE_DVD))
    {
      if(m_dvd.state == DVDSTATE_STILL)
      {
        m_State.time       = CTimeUtils::GetTimeMS() - m_dvd.iDVDStillStartTime;
        m_State.time_total = m_dvd.iDVDStillTime;
      }

      if(!((CDVDInputStreamNavigator*)m_pInputStream)->GetNavigatorState(m_State.player_state))
        m_State.player_state = "";
    }
    else
        m_State.player_state = "";

    CDVDInputStream::IChannel* input = dynamic_cast<CDVDInputStream::IChannel*>(m_pInputStream);
    if(input)
    {
      m_State.canrecord = input->CanRecord();
      m_State.recording = input->IsRecording();

      if(input->GetTotalTime() > 0)
      {
        m_State.time       = input->GetStartTime();
        m_State.time_total = input->GetTotalTime();
      }
    }
  }

  if (m_Edl.HasCut())
  {
    m_State.time        = m_Edl.RemoveCutTime(llrint(m_State.time));
    m_State.time_total  = m_Edl.RemoveCutTime(llrint(m_State.time_total));
  }

  if (m_CurrentAudio.id >= 0 && m_pDemuxer)
  {
    CDemuxStream* pStream = m_pDemuxer->GetStream(m_CurrentAudio.id);
    if (pStream && pStream->type == STREAM_AUDIO)
      ((CDemuxStreamAudio*)pStream)->GetStreamInfo(m_State.demux_audio);
  }
  else
    m_State.demux_audio = "";

  if (m_CurrentVideo.id >= 0 && m_pDemuxer)
  {
    CDemuxStream* pStream = m_pDemuxer->GetStream(m_CurrentVideo.id);
    if (pStream && pStream->type == STREAM_VIDEO)
      ((CDemuxStreamVideo*)pStream)->GetStreamInfo(m_State.demux_video);
  }
  else
    m_State.demux_video = "";

  m_State.timestamp = CDVDClock::GetAbsoluteClock();
}

void CDVDPlayer::UpdateApplication(double timeout)
{
  if(m_UpdateApplication != 0
  && m_UpdateApplication + DVD_MSEC_TO_TIME(timeout) > CDVDClock::GetAbsoluteClock())
    return;

  CDVDInputStream::IChannel* pStream = dynamic_cast<CDVDInputStream::IChannel*>(m_pInputStream);
  if(pStream)
  {
    CFileItem item(g_application.CurrentFileItem());
    if(pStream->UpdateItem(item))
    {
      g_application.CurrentFileItem() = item;
      g_infoManager.SetCurrentItem(item);
    }
  }
  m_UpdateApplication = CDVDClock::GetAbsoluteClock();
}

bool CDVDPlayer::CanRecord()
{
  CSingleLock lock(m_StateSection);
  return m_State.canrecord;
}

bool CDVDPlayer::IsRecording()
{
  CSingleLock lock(m_StateSection);
  return m_State.recording;
}

bool CDVDPlayer::Record(bool bOnOff)
{
  if (m_pInputStream && (m_pInputStream->IsStreamType(DVDSTREAM_TYPE_TV) ||
                         m_pInputStream->IsStreamType(DVDSTREAM_TYPE_PVRMANAGER)) )
  {
    m_messenger.Put(new CDVDMsgBool(CDVDMsg::PLAYER_SET_RECORD, bOnOff));
    return true;
  }
  return false;
}

int CDVDPlayer::GetChannels()
{
  if (m_pDemuxer && (m_CurrentAudio.id != -1))
  {
    CDemuxStreamAudio* stream = static_cast<CDemuxStreamAudio*>(m_pDemuxer->GetStream(m_CurrentAudio.id));
    if (stream)
      return stream->iChannels;
  }
  return -1;
}

CStdString CDVDPlayer::GetAudioCodecName()
{
  CStdString retVal;
  if (m_pDemuxer && (m_CurrentAudio.id != -1))
    m_pDemuxer->GetStreamCodecName(m_CurrentAudio.id, retVal);
  return retVal;
}

CStdString CDVDPlayer::GetVideoCodecName()
{
  CStdString retVal;
  if (m_pDemuxer && (m_CurrentVideo.id != -1))
    m_pDemuxer->GetStreamCodecName(m_CurrentVideo.id, retVal);
  return retVal;
}

int CDVDPlayer::GetPictureWidth()
{
  if (m_pDemuxer && (m_CurrentVideo.id != -1))
  {
    CDemuxStreamVideo* stream = static_cast<CDemuxStreamVideo*>(m_pDemuxer->GetStream(m_CurrentVideo.id));
    if (stream)
      return stream->iWidth;
  }
  return 0;
}

bool CDVDPlayer::GetStreamDetails(CStreamDetails &details)
{
  if (m_pDemuxer)
  {
    bool result=CDVDFileInfo::DemuxerToStreamDetails(m_pDemuxer, details);
    if (result) // this is more correct (dvds in particular)
      GetVideoAspectRatio(((CStreamDetailVideo*)details.GetNthStream(CStreamDetail::VIDEO,0))->m_fAspect);
    return result;
  }
  else
    return false;
}

CStdString CDVDPlayer::GetPlayingTitle()
{
  /* Currently we support only Title Name from Teletext line 30 */
  TextCacheStruct_t* ttcache = m_dvdPlayerTeletext.GetTeletextCache();
  if (ttcache && !ttcache->line30.empty())
    return ttcache->line30;

  return "";
}

CDVDPlayer::CPlayerSeek::CPlayerSeek(CDVDPlayer* player)
      : m_player(*player)
{
  g_infoManager.SetDisplayAfterSeek(100000);
  m_player.SetCaching(CACHESTATE_INIT);
}

CDVDPlayer::CPlayerSeek::~CPlayerSeek()
{
  g_infoManager.SetDisplayAfterSeek();
}
