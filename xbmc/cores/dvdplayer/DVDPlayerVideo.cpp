
#include "../../stdafx.h"
#include "DVDPlayer.h"
#include "DVDPlayerVideo.h"
#include "DVDCodecs\DVDFactoryCodec.h"
#include "DVDCodecs\DVDCodecUtils.h"
#include "DVDCodecs\DVDVideoPPFFmpeg.h"
#include "..\..\util.h"


DWORD video_refresh_thread(void *arg);

#define EMULATE_INTTYPES
#include "ffmpeg\avcodec.h"

#define ABS(a) ((a) >= 0 ? (a) : (-(a)))


void xbox_dvdplayer_render_update()
{
  g_renderManager.Update(false);
}

CDVDPlayerVideo::CDVDPlayerVideo(CDVDDemuxSPU* spu, CDVDClock* pClock) : CThread()
{
  m_pDVDSpu = spu;
  m_pClock = pClock;
  m_pVideoCodec = NULL;
  m_bInitializedOutputDevice = false;
  m_pOverlayPicture = NULL;
  m_iSpeed = 1;
  m_bRenderSubs = false;
  m_iVideoDelay = 0;
  m_fForcedAspectRatio = 0;
  m_iNrOfPicturesNotToSkip = 0;
  InitializeCriticalSection(&m_critCodecSection);
  m_packetQueue.SetMaxSize(5 * 256 * 1024); // 1310720

  m_iDroppedFrames = 0;
  
  // create sections and event for thread sync (should be destroyed at video stop)
  InitializeCriticalSection(&m_critSection);
  m_hEvent = CreateEvent(NULL, false, false, "dvd picture queue event");

}

CDVDPlayerVideo::~CDVDPlayerVideo()
{
  // close the stream, and don't wait for the audio to be finished
  // CloseStream(true);
  DeleteCriticalSection(&m_critCodecSection);

  DeleteCriticalSection(&m_critSection);
  CloseHandle(m_hEvent);
}

bool CDVDPlayerVideo::OpenStream(CodecID codecID, int iWidth, int iHeight, CDemuxStreamVideo* pDemuxStreamVideo)
{
  m_pDemuxStreamVideo = pDemuxStreamVideo;

  // should alway's be NULL!!!!, it will probably crash anyway when deleting m_pVideoCodec here.
  if (m_pVideoCodec)
  {
    CLog::Log(LOGFATAL, "CDVDPlayerVideo::OpenStream() m_pVideoCodec != NULL");
    return false;
  }

  CLog::Log(LOGNOTICE, "Creating video codec with codec id: %i", codecID);
  m_pVideoCodec = CDVDFactoryCodec::CreateVideoCodec(codecID);

  if( !m_pVideoCodec )
  {
    CLog::Log(LOGERROR, "Unsupported video codec");
    return false;
  }

  if (!m_pVideoCodec->Open(codecID, iWidth, iHeight))
  {
    m_pVideoCodec->Dispose();
    delete m_pVideoCodec;
    m_pVideoCodec = NULL;
    return false;
  }

  m_packetQueue.Init();

  CLog::Log(LOGNOTICE, "Creating video thread");
  Create();

  return true;
}

void CDVDPlayerVideo::CloseStream(bool bWaitForBuffers)
{
  m_packetQueue.Abort();

  /* note: we also signal this mutex to make sure we deblock the video thread in all cases */
  EnterCriticalSection(&m_critSection);
  SetEvent(m_hEvent);
  LeaveCriticalSection(&m_critSection);

  // wait for decode_video thread to end
  CLog::Log(LOGNOTICE, "waiting for video thread to exit");

  StopThread(); // will set this->m_bStop to true
  this->WaitForThreadExit(INFINITE);

  m_packetQueue.End();
  m_overlay.Clear();

  CLog::Log(LOGNOTICE, "deleting video codec");
  if (m_pVideoCodec)
  {
    m_pVideoCodec->Dispose();
    delete m_pVideoCodec;
    m_pVideoCodec = NULL;
  }

  m_pDemuxStreamVideo = NULL;
}

void CDVDPlayerVideo::OnStartup()
{
  pictq_size = 0;
  pictq_rindex = 0;
  pictq_windex = 0;
  
  m_iDroppedFrames = 0;
}

void CDVDPlayerVideo::Process()
{
  CLog::Log(LOGNOTICE, "running thread: video_thread");

  HANDLE hVideoRefreshThread;
  CDVDDemux::DemuxPacket* pPacket;
  DVDVideoPicture picture;
  CDVDVideoPPFFmpeg mDeinterlace(CDVDVideoPPFFmpeg::ED_DEINT_FFMPEG);

  memset(&picture, 0, sizeof(DVDVideoPicture));
  __int64 pts = 0;
  int dvdstate;

  m_bRunningVideo = true;

  hVideoRefreshThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)video_refresh_thread, this, 0, NULL);
  SetThreadPriority(hVideoRefreshThread, THREAD_PRIORITY_TIME_CRITICAL);

  while (!m_bStop)
  {
    while (m_iSpeed == 0 && !m_packetQueue.RecievedAbortRequest()) Sleep(5);

    if (m_packetQueue.Get(&pPacket, 1, (void**)&dvdstate) < 0) break;

    if (dvdstate & DVDPACKET_MESSAGE_RESYNC)
    {
      //Discontinuity found..
      //Audio side normally handles discontinuities so don't do anything here.
    }
    if (dvdstate & DVDPACKET_MESSAGE_NOSKIP)
    {
      // libmpeg2 is also returning incomplete frames after a dvd cell change
      // so the first few pictures are not the correct ones to display in some cases
      // just display those together with the correct one.
      // (setting it to 2 will skip some menu stills, 5 is working ok for me).
      m_iNrOfPicturesNotToSkip = 5;
    }

    EnterCriticalSection(&m_critCodecSection);
    int iDecoderState = m_pVideoCodec->Decode(pPacket->pData, pPacket->iSize);


    // loop while no error
    while (!(iDecoderState & VC_ERROR))
    {
      // check for a new picture
      if (iDecoderState & VC_PICTURE)
      {

        // try to retrieve the picture (should never fail!), unless there is a demuxer bug ofcours
        fast_memset(&picture, 0, sizeof(DVDVideoPicture));
        if (m_pVideoCodec->GetPicture(&picture))
        {          
          if (picture.iDuration == 0)
          {
            if (m_pDemuxStreamVideo->iFpsRate && m_pDemuxStreamVideo->iFpsScale)
            {
              picture.iDuration = (unsigned int)(((__int64)DVD_TIME_BASE * m_pDemuxStreamVideo->iFpsScale) / m_pDemuxStreamVideo->iFpsRate);
            }
            else
            {
              picture.iDuration = DVD_TIME_BASE / 25;
            }
          }

          if (m_iNrOfPicturesNotToSkip > 0)
          {
            picture.iFlags |= DVP_FLAG_NOSKIP;
            m_iNrOfPicturesNotToSkip--;
          }
          
          //Deinterlace if codec said format was interlaced or if we have selected we want to deinterlace
          //this video

          // software deinterlacing takes about 40% cpu power, so we don't enable it.
          // and we have hardware deinterlacing now.
          /*
          if( (g_stSettings.m_currentVideoSettings.m_FieldSync == VS_FIELDSYNC_OFF && picture.iFlags & DVP_FLAG_INTERLACED)
              || g_stSettings.m_currentVideoSettings.m_Deinterlace )
          {
            mDeinterlace.Process(&picture);
            mDeinterlace.GetPicture(&picture);
          }
          */

          if ((picture.iFrameType == FRAME_TYPE_I || picture.iFrameType == FRAME_TYPE_UNDEF) &&
              pPacket->dts != DVD_NOPTS_VALUE) //Only use pts when we have an I frame, or unknown
          {
            pts = pPacket->dts;
          }
          
          //Check if dvd has forced an aspect ratio
          if( m_fForcedAspectRatio != 0.0f )
          {
            picture.iDisplayWidth = (int) (picture.iDisplayHeight * m_fForcedAspectRatio);
          }

          bool bResult;
          do 
          {
            bResult = OutputPicture(&picture, pts);
            if (!bResult) break;
            
            // guess next frame pts. iDuration is always valid
            pts += picture.iDuration;
          }
          while (picture.iRepeatPicture-- > 0);
          
          if( !bResult) break;
        }
        else
        {
          CLog::Log(LOGWARNING, "Decoder Error getting videoPicture.");
          m_pVideoCodec->Reset();
        }
      }

      // if the decoder needs more data, we just break this loop
      // and try to get more data from the videoQueue
      if (iDecoderState & VC_BUFFER) break;

      // the decoder didn't need more data, flush the remaning buffer
      iDecoderState = m_pVideoCodec->Decode(NULL, NULL);
    }
    LeaveCriticalSection(&m_critCodecSection);
    // all data is used by the decoder, we can safely free it now
    CDVDDemuxUtils::FreeDemuxPacket(pPacket);
  }

  CLog::Log(LOGNOTICE, "wating for video_refresh_thread");
  m_bRunningVideo = false;
  WaitForSingleObject(hVideoRefreshThread, INFINITE);

  CLog::Log(LOGNOTICE, "thread end: video_thread");

  CLog::Log(LOGNOTICE, "uninitting video device");
}

void CDVDPlayerVideo::OnExit()
{
  if (m_pOverlayPicture)
  {
    CLog::Log(LOGNOTICE, "CDVDPlayer::OnExit() freeing overlay picture");
    CDVDCodecUtils::FreePicture(m_pOverlayPicture);
  }
  m_pOverlayPicture = NULL;

  g_renderManager.UnInit();
  m_bInitializedOutputDevice = false;

  CLog::Log(LOGNOTICE, "thread end: video_thread");
}

void CDVDPlayerVideo::Pause()
{
  m_iSpeed = 0;
}

bool CDVDPlayerVideo::InitializedOutputDevice()
{
  return m_bInitializedOutputDevice;
}

void CDVDPlayerVideo::Resume()
{
  m_iSpeed = 1;
}

void CDVDPlayerVideo::Flush()
{
  EnterCriticalSection(&m_critCodecSection);
  m_packetQueue.Flush();
  m_overlay.Clear(); // not thread safe !!!!
  if (m_pVideoCodec)
  {
    //m_pVideoCodec->Reset();
  }
  LeaveCriticalSection(&m_critCodecSection);
}


bool CDVDPlayerVideo::OutputPicture(DVDVideoPicture* pPicture, __int64 pts1)
{
  DVDVideoPicture *vp;
  
  __int64 pts = pts1;

  // wait  until we have space to put a new picture
  EnterCriticalSection(&m_critSection);
  while (pictq_size >= VIDEO_PICTURE_QUEUE_SIZE && !m_packetQueue.RecievedAbortRequest())
  {
    LeaveCriticalSection(&m_critSection);
    WaitForSingleObject(m_hEvent, INFINITE);
    EnterCriticalSection(&m_critSection);
  }
  LeaveCriticalSection(&m_critSection);

  if (m_packetQueue.RecievedAbortRequest()) return false;

  vp = &pictq[pictq_windex];

  if (!m_bInitializedOutputDevice)
  {
    float fFPS;
    
    CLog::Log(LOGNOTICE, "Initializing video device");

    if( m_pDemuxStreamVideo->iFpsRate != 0 && m_pDemuxStreamVideo->iFpsScale != 0)
    {
      fFPS = ((float)m_pDemuxStreamVideo->iFpsRate / m_pDemuxStreamVideo->iFpsScale);
    }
    else
    {
      CLog::Log(LOGERROR, "Demuxer reported invalid framerate: %d or fpsscale: %d", m_pDemuxStreamVideo->iFpsRate, m_pDemuxStreamVideo->iFpsScale);
      fFPS = 25.0;
    }
  
    g_renderManager.PreInit();

    CLog::Log(LOGNOTICE, "  fps: %f, pwidth: %i, pheight: %i, dwidth: %i, dheight: %i",
              fFPS, pPicture->iWidth, pPicture->iHeight, pPicture->iDisplayWidth, pPicture->iDisplayHeight);

    g_renderManager.Configure(pPicture->iWidth, pPicture->iHeight, pPicture->iDisplayWidth, pPicture->iDisplayHeight, fFPS);
    m_bInitializedOutputDevice = true;
  }

  if (m_bInitializedOutputDevice)
  {

    // create overlay picture if not already done
    if (!m_pOverlayPicture)
    {
      m_pOverlayPicture = CDVDCodecUtils::AllocatePicture(pPicture->iWidth, pPicture->iHeight);
    }

    // copy picture to overlay
    YV12Image image;
    if (g_renderManager.GetImage(&image))
    {
      CDVDCodecUtils::CopyPictureToOverlay(&image, pPicture);

      // remove any overlays that are out of time
      m_overlay.CleanUp(pts);

      DVDOverlayPicture* pOverlayPicture = m_overlay.Get();

      //Check all overlays and render those that should be rendered, based on time and forced
      //Both forced and subs should check timeing, pts == 0 in the stillframe case
      while (pOverlayPicture)
      {
        if ((pOverlayPicture->bForced || m_bRenderSubs)
            && ((pOverlayPicture->iPTSStartTime <= pts && pOverlayPicture->iPTSStopTime >= pts) || pts == 0))
        {
          // display subtitle, if bForced is true, it's a menu overlay and we should crop it
          m_overlay.RenderYUV(&image, pOverlayPicture, pOverlayPicture->bForced);
        }

        pOverlayPicture = pOverlayPicture->pNext;
      }

      // tell the renderer that we've finished with the image (so it can do any
      // post processing before FlipPage() is called.)
      g_renderManager.ReleaseImage();
    }

    //Hmm why not just assigne the entire picure??
    //the vp isn't used at all is it?? overlay is what's rendered all the time
    vp->pts = pts;
    vp->iDuration = pPicture->iDuration;
    vp->iFrameType = pPicture->iFrameType;
    vp->iFlags = pPicture->iFlags;

    /* now we can update the picture count */
    if (++pictq_windex == VIDEO_PICTURE_QUEUE_SIZE) pictq_windex = 0;

    EnterCriticalSection(&m_critSection);
    pictq_size++;
    LeaveCriticalSection(&m_critSection);

    if (m_iSpeed < 0 && m_iSpeed > 1)
    {
      // ffwd or rw
      //is->seek_req = 1;
      //is->seek_pos = (unsigned __int64)(pts * AV_TIME_BASE) + (m_iSpeed * 400000);
    }

  }
  return true;
}

void CDVDPlayerVideo::Update(bool bPauseDrawing)
{
  g_renderManager.Update(bPauseDrawing);
}

void CDVDPlayerVideo::UpdateMenuPicture()
{
  if (m_pVideoCodec)
  {
    DVDVideoPicture picture;
    EnterCriticalSection(&m_critCodecSection);
    if (m_pVideoCodec->GetPicture(&picture))
    {
      picture.iFlags |= DVP_FLAG_NOSKIP;
      OutputPicture(&picture, 0);
    }
    LeaveCriticalSection(&m_critCodecSection);
  }
}

void CDVDPlayerVideo::GetVideoRect(RECT& SrcRect, RECT& DestRect)
{
  g_renderManager.GetVideoRect(SrcRect, DestRect);
}

float CDVDPlayerVideo::GetAspectRatio()
{
  return g_renderManager.GetAspectRatio();
}

__int64 CDVDPlayerVideo::GetDelay()
{
  return m_iVideoDelay;
}

void CDVDPlayerVideo::SetDelay(__int64 delay)
{
  m_iVideoDelay = delay;
}

__int64 CDVDPlayerVideo::GetDiff()
{
  return 0LL;
}

void CDVDPlayerVideo::SetAspectRatio(float fAspectRatio)
{
  m_fForcedAspectRatio = fAspectRatio;
}

/* called to display each frame */
DWORD video_refresh_thread(void *arg)
{
  CDVDPlayerVideo* pDVDPlayerVideo = (CDVDPlayerVideo*)arg;
  DVDVideoPicture *vp;

  CLog::Log(LOGNOTICE, "running thread: video_refresh_thread");
  int iSleepTime = 0;
  CDVDClock frameclock;
  __int64 iTimeStamp = frameclock.GetClock();
  int iFrameTime;
  int iFrameTimeError=0;

  while (pDVDPlayerVideo->m_bRunningVideo)
  {
    if (pDVDPlayerVideo->pictq_size == 0 || pDVDPlayerVideo->m_iSpeed == 0)
    {
      // if no picture, need to wait
      usleep(1000);
    }
    else
    {
      // dequeue the picture
      vp = &pDVDPlayerVideo->pictq[pDVDPlayerVideo->pictq_rindex];

      if( vp->iFlags & DVP_FLAG_INTERLACED && !g_stSettings.m_currentVideoSettings.m_Deinterlace )
      {
        if( vp->iFlags & DVP_FLAG_TOP_FIELD_FIRST )
          g_renderManager.SetFieldSync(FS_ODD);
        else
          g_renderManager.SetFieldSync(FS_EVEN);
      }
      else
        g_renderManager.SetFieldSync( FS_NONE );
      
      //Prepare image for display, will speed up rendering as it frees resources for decode earlier
      g_renderManager.PrepareDisplay();


      bool bDiscontinuity = pDVDPlayerVideo->m_pClock->HadDiscontinuity(1 * DVD_TIME_BASE);
      if (bDiscontinuity)
      {
        //Playback at normal fps untill 1 sec after discontinuity
        iFrameTime = vp->iDuration;
        iSleepTime = iFrameTime - (int)(frameclock.GetClock() - iTimeStamp);
      }
      else
        iSleepTime = (int)((vp->pts + pDVDPlayerVideo->m_iVideoDelay - pDVDPlayerVideo->m_pClock->GetClock()) & 0xFFFFFFFF);
      

      //Adjust for flippage delay
      iSleepTime = iSleepTime + iFrameTimeError;


      if (iSleepTime > 500000) iSleepTime = 500000; // drop to a minimum of 2 frames/sec

      // we could drop some frames here too if iSleepTime < 0, but I don't think it will be any
      // use at this stage currently (drawing pictures isn't taking the most processing power)

      // sleep
      if (iSleepTime > 0) usleep(iSleepTime);

      // display picture
      // we expect the video device to be initialized here
      // skip this flip should we be later than a full frame
      iTimeStamp = frameclock.GetClock();
      
      // menu pictures should never be skipped!
      if ((vp->iFlags & DVP_FLAG_NOSKIP) || iSleepTime > -(int)vp->iDuration) 
      {
        g_renderManager.FlipPage();
      
        if( bDiscontinuity )
        {
          //Adjust using the delay in flippage
          iFrameTimeError = (int)((iTimeStamp - frameclock.GetClock()) & 0xFFFFFFFF);
        }
        else
        {
          //Recalculate the sleep time, so we can adjust with how much we were off
          iFrameTimeError = (int)((vp->pts + pDVDPlayerVideo->m_iVideoDelay - frameclock.GetClock()) & 0xFFFFFFFF);
        }

        //Check bounds for this as it can be way off sometimes
        if( iFrameTimeError > (int)vp->iDuration )
          iFrameTimeError = (int)vp->iDuration ;
        else if( iFrameTimeError < -(int)vp->iDuration  )
          iFrameTimeError = -(int)vp->iDuration;
      }
      else
      {
        pDVDPlayerVideo->m_iDroppedFrames++;
      }

      // update queue size and signal for next picture
      if (++pDVDPlayerVideo->pictq_rindex == VIDEO_PICTURE_QUEUE_SIZE) pDVDPlayerVideo->pictq_rindex = 0;

      EnterCriticalSection(&pDVDPlayerVideo->m_critSection);
      pDVDPlayerVideo->pictq_size--;
      SetEvent(pDVDPlayerVideo->m_hEvent);
      LeaveCriticalSection(&pDVDPlayerVideo->m_critSection);
    }
  }
  CLog::Log(LOGNOTICE, "thread end: video_refresh_thread");

  return 0;
}
