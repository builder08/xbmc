/*
 *      Copyright (C) 2005-2013 Team XBMC
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
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include "system.h"
#if defined(HAS_GL)
  #include "system_gl.h"
#endif
#include "RenderManager.h"
#include "threads/CriticalSection.h"
#include "video/VideoReferenceClock.h"
#include "utils/MathUtils.h"
#include "threads/SingleLock.h"
#include "utils/log.h"
#include "utils/TimeUtils.h"

#include "Application.h"
#include "ApplicationMessenger.h"
#include "settings/AdvancedSettings.h"
#include "settings/MediaSettings.h"
#include "settings/Settings.h"

#if defined(HAS_GL)
  #include "LinuxRendererGL.h"
#elif HAS_GLES == 2
  #include "LinuxRendererGLES.h"
#elif defined(HAS_DX)
  #include "WinRenderer.h"
#elif defined(HAS_SDL)
  #include "LinuxRenderer.h"
#endif

#include "RenderCapture.h"

/* to use the same as player */
#include "../dvdplayer/DVDClock.h"
#include "../dvdplayer/DVDCodecs/Video/DVDVideoCodec.h"
#include "../dvdplayer/DVDCodecs/DVDCodecUtils.h"

#define MAXPRESENTDELAY 0.500

/* at any point we want an exclusive lock on rendermanager */
/* we must make sure we don't have a graphiccontext lock */
/* these two functions allow us to step out from that lock */
/* and reaquire it after having the exclusive lock */

template<class T>
class CRetakeLock
{
public:
  CRetakeLock(CSharedSection &section, CCriticalSection &owned = g_graphicsContext)
    : m_count(owned.exit())
    , m_lock (section),
      m_owned(owned)
  {
    m_owned.restore(m_count);
  }

  void Leave() { m_lock.Leave(); }
  void Enter()
  {
    m_count = m_owned.exit();
    m_lock.Enter();
    m_owned.restore(m_count);
  }

private:
  DWORD             m_count;
  T                 m_lock;
  CCriticalSection &m_owned;
};

static void requeue(std::deque<int> &trg, std::deque<int> &src)
{
  trg.push_back(src.front());
  src.pop_front();
}

CXBMCRenderManager::CXBMCRenderManager()
{
  m_pRenderer = NULL;
  m_bIsStarted = false;

  m_presentstep = PRESENT_IDLE;
  m_rendermethod = 0;
  m_presentsource = 0;
  m_bReconfigured = false;
  m_hasCaptures = false;
  m_displayLatency = 0.0f;
  m_presentcorr = 0.0;
  m_presenterr = 0.0;
  memset(&m_errorbuff, 0, ERRORBUFFSIZE);
  m_errorindex = 0;
  m_QueueSize   = 2;
  m_QueueSkip   = 0;
  m_format      = RENDER_FMT_NONE;
}

CXBMCRenderManager::~CXBMCRenderManager()
{
  delete m_pRenderer;
  m_pRenderer = NULL;
}

void CXBMCRenderManager::GetVideoRect(CRect &source, CRect &dest)
{
  CSharedLock lock(m_sharedSection);
  if (m_pRenderer)
    m_pRenderer->GetVideoRect(source, dest);
}

float CXBMCRenderManager::GetAspectRatio()
{
  CSharedLock lock(m_sharedSection);
  if (m_pRenderer)
    return m_pRenderer->GetAspectRatio();
  else
    return 1.0f;
}

/* These is based on CurrentHostCounter() */
double CXBMCRenderManager::GetPresentTime()
{
  return CDVDClock::GetAbsoluteClock(false) / DVD_TIME_BASE;
}

static double wrap(double x, double minimum, double maximum)
{
  if(x >= minimum
  && x <= maximum)
    return x;
  x = fmod(x - minimum, maximum - minimum) + minimum;
  if(x < minimum)
    x += maximum - minimum;
  if(x > maximum)
    x -= maximum - minimum;
  return x;
}

void CXBMCRenderManager::WaitPresentTime(double presenttime)
{
  double frametime;
  int fps = g_VideoReferenceClock.GetRefreshRate(&frametime);
  if(fps <= 0)
  {
    /* smooth video not enabled */
    CDVDClock::WaitAbsoluteClock(presenttime * DVD_TIME_BASE);
    return;
  }

  bool ismaster = CDVDClock::IsMasterClock();

  //the videoreferenceclock updates its clock on every vertical blank
  //we want every frame's presenttime to end up in the middle of two vblanks
  //if CDVDPlayerAudio is the master clock, we add a correction to the presenttime
  if (ismaster)
    presenttime += m_presentcorr * frametime;

  double clock     = CDVDClock::WaitAbsoluteClock(presenttime * DVD_TIME_BASE) / DVD_TIME_BASE;
  double target    = 0.5;
  double error     = ( clock - presenttime ) / frametime - target;

  m_presenterr     = error;

  // correct error so it targets the closest vblank
  error = wrap(error, 0.0 - target, 1.0 - target);

  // scale the error used for correction,
  // based on how much buffer we have on
  // that side of the target
  if(error > 0)
    error /= 2.0 * (1.0 - target);
  if(error < 0)
    error /= 2.0 * (0.0 + target);

  //save error in the buffer
  m_errorindex = (m_errorindex + 1) % ERRORBUFFSIZE;
  m_errorbuff[m_errorindex] = error;

  //get the average error from the buffer
  double avgerror = 0.0;
  for (int i = 0; i < ERRORBUFFSIZE; i++)
    avgerror += m_errorbuff[i];

  avgerror /= ERRORBUFFSIZE;


  //if CDVDPlayerAudio is not the master clock, we change the clock speed slightly
  //to make every frame's presenttime end up in the middle of two vblanks
  if (!ismaster)
  {
    //integral correction, clamp to -0.5:0.5 range
    m_presentcorr = std::max(std::min(m_presentcorr + avgerror * 0.01, 0.1), -0.1);
    g_VideoReferenceClock.SetFineAdjust(1.0 - avgerror * 0.01 - m_presentcorr * 0.01);
  }
  else
  {
    //integral correction, wrap to -0.5:0.5 range
    m_presentcorr = wrap(m_presentcorr + avgerror * 0.01, target - 1.0, target);
    g_VideoReferenceClock.SetFineAdjust(1.0);
  }

  //printf("%f %f % 2.0f%% % f % f\n", presenttime, clock, m_presentcorr * 100, error, error_org);
}

CStdString CXBMCRenderManager::GetVSyncState()
{
  double avgerror = 0.0;
  for (int i = 0; i < ERRORBUFFSIZE; i++)
    avgerror += m_errorbuff[i];
  avgerror /= ERRORBUFFSIZE;

  CStdString state;
  state.Format("sync:%+3d%% avg:%3d%% error:%2d%%"
              ,     MathUtils::round_int(m_presentcorr * 100)
              ,     MathUtils::round_int(avgerror      * 100)
              , abs(MathUtils::round_int(m_presenterr  * 100)));
  return state;
}

bool CXBMCRenderManager::Configure(unsigned int width, unsigned int height, unsigned int d_width, unsigned int d_height, float fps, unsigned flags, ERenderFormat format, unsigned extended_format, unsigned int orientation, int buffers)
{

  CSingleLock    lock2(m_presentlock);

  /* make sure any queued frame was fully presented */
  XbmcThreads::EndTime endtime(5000);
  while(m_presentstep != PRESENT_IDLE)
  {
    if(endtime.IsTimePast())
    {
      CLog::Log(LOGWARNING, "CRenderManager::Configure - timeout waiting for state");
      return false;
    }
    m_presentevent.wait(lock2, endtime.MillisLeft());
  };
  lock2.Leave();

  CExclusiveLock lock(m_sharedSection);
  if(!m_pRenderer)
  {
    CLog::Log(LOGERROR, "%s called without a valid Renderer object", __FUNCTION__);
    return false;
  }


  bool result = m_pRenderer->Configure(width, height, d_width, d_height, fps, flags, format, extended_format, orientation);
  if(result)
  {
    if( flags & CONF_FLAGS_FULLSCREEN )
    {
      lock.Leave();
      CApplicationMessenger::Get().SwitchToFullscreen();
      lock.Enter();
    }
    lock2.Enter();
    m_format = format;

    int processor = m_pRenderer->GetProcessorSize();
    if(processor)
      m_QueueSize = buffers - processor + 1;         /* respect maximum refs */
    else
      m_QueueSize = m_pRenderer->GetMaxBufferSize(); /* no refs to data */

    m_QueueSize = std::min(m_QueueSize, (int)m_pRenderer->GetMaxBufferSize());
    m_QueueSize = std::min(m_QueueSize, NUM_BUFFERS);
    if(m_QueueSize < 2)
    {
      m_QueueSize = 2;
      CLog::Log(LOGWARNING, "CXBMCRenderManager::Configure - queue size too small (%d, %d, %d)", m_QueueSize, processor, buffers);
    }

    m_pRenderer->SetBufferSize(m_QueueSize);
    m_pRenderer->Update();

    m_queued.clear();
    m_discard.clear();
    m_free.clear();
    m_presentsource = 0;
    for (int i=1; i < m_QueueSize; i++)
      m_free.push_back(i);

    m_bIsStarted = true;
    m_bReconfigured = true;
    m_presentstep = PRESENT_IDLE;
    m_presentevent.notifyAll();

    CLog::Log(LOGDEBUG, "CXBMCRenderManager::Configure - %d", m_QueueSize);
  }

  return result;
}

bool CXBMCRenderManager::RendererHandlesPresent() const
{
  return IsConfigured() && m_format != RENDER_FMT_BYPASS;
}

bool CXBMCRenderManager::IsConfigured() const
{
  if (!m_pRenderer)
    return false;
  return m_pRenderer->IsConfigured();
}

void CXBMCRenderManager::Update()
{
  CRetakeLock<CExclusiveLock> lock(m_sharedSection);

  if (m_pRenderer)
    m_pRenderer->Update();
}

bool CXBMCRenderManager::FrameWait(int ms)
{
  XbmcThreads::EndTime timeout(ms);
  CSingleLock lock(m_presentlock);
  while(m_presentstep == PRESENT_IDLE && !timeout.IsTimePast())
    m_presentevent.wait(lock, timeout.MillisLeft());
  return m_presentstep != PRESENT_IDLE;
}

void CXBMCRenderManager::FrameMove()
{
  { CSharedLock lock(m_sharedSection);
    CSingleLock lock2(m_presentlock);

    if (!m_pRenderer)
      return;

    if (m_presentstep == PRESENT_FRAME2)
    {
      if(!m_queued.empty())
      {
        double timestamp = GetPresentTime();
        SPresent& m = m_Queue[m_presentsource];
        SPresent& q = m_Queue[m_queued.front()];
        if(timestamp > m.timestamp + (q.timestamp - m.timestamp) * 0.5)
        {
          m_presentstep = PRESENT_READY;
          m_presentevent.notifyAll();
        }
      }
    }

    if (m_presentstep == PRESENT_READY)
      PrepareNextRender();

    if(m_presentstep == PRESENT_FLIP)
    {
      m_pRenderer->FlipPage(m_presentsource);
      m_presentstep = PRESENT_FRAME;
      m_presentevent.notifyAll();
    }

    /* release all previous */
    for(std::deque<int>::iterator it = m_discard.begin(); it != m_discard.end(); )
    {
      // TODO check for fence
      m_pRenderer->ReleaseBuffer(*it);
      m_overlays.Release(*it);
      m_free.push_back(*it);
      it = m_discard.erase(it);
    }
  }
}

void CXBMCRenderManager::FrameFinish()
{
  /* wait for this present to be valid */
  SPresent& m = m_Queue[m_presentsource];

  if(g_graphicsContext.IsFullScreenVideo())
    WaitPresentTime(m.timestamp);

  { CSingleLock lock(m_presentlock);

    if(m_presentstep == PRESENT_FRAME)
    {
      if( m.presentmethod == PRESENT_METHOD_BOB
      ||  m.presentmethod == PRESENT_METHOD_WEAVE)
        m_presentstep = PRESENT_FRAME2;
      else
        m_presentstep = PRESENT_IDLE;
    }
    else if(m_presentstep == PRESENT_FRAME2)
      m_presentstep = PRESENT_IDLE;


    if(m_presentstep == PRESENT_IDLE)
    {
      if(!m_queued.empty())
        m_presentstep = PRESENT_READY;
    }

    m_presentevent.notifyAll();
  }
}

unsigned int CXBMCRenderManager::PreInit()
{
  CRetakeLock<CExclusiveLock> lock(m_sharedSection);

  m_presentcorr = 0.0;
  m_presenterr  = 0.0;
  m_errorindex  = 0;
  memset(m_errorbuff, 0, sizeof(m_errorbuff));

  m_bIsStarted = false;
  if (!m_pRenderer)
  {
#if defined(HAS_GL)
    m_pRenderer = new CLinuxRendererGL();
#elif HAS_GLES == 2
    m_pRenderer = new CLinuxRendererGLES();
#elif defined(HAS_DX)
    m_pRenderer = new CWinRenderer();
#elif defined(HAS_SDL)
    m_pRenderer = new CLinuxRenderer();
#endif
  }

  UpdateDisplayLatency();

  m_QueueSize   = 2;
  m_QueueSkip   = 0;

  return m_pRenderer->PreInit();
}

void CXBMCRenderManager::UnInit()
{
  CRetakeLock<CExclusiveLock> lock(m_sharedSection);

  m_bIsStarted = false;

  m_overlays.Flush();

  // free renderer resources.
  // TODO: we may also want to release the renderer here.
  if (m_pRenderer)
    m_pRenderer->UnInit();
}

bool CXBMCRenderManager::Flush()
{
  if (!m_pRenderer)
    return true;

  if (g_application.IsCurrentThread())
  {
    CLog::Log(LOGDEBUG, "%s - flushing renderer", __FUNCTION__);

    CRetakeLock<CExclusiveLock> lock(m_sharedSection);
    m_pRenderer->Flush();
    m_overlays.Flush();
    m_flushEvent.Set();
  }
  else
  {
    ThreadMessage msg = {TMSG_RENDERER_FLUSH};
    m_flushEvent.Reset();
    CApplicationMessenger::Get().SendMessage(msg, false);
    if (!m_flushEvent.WaitMSec(1000))
    {
      CLog::Log(LOGERROR, "%s - timed out waiting for renderer to flush", __FUNCTION__);
      return false;
    }
    else
      return true;
  }
  return true;
}

void CXBMCRenderManager::SetupScreenshot()
{
  CSharedLock lock(m_sharedSection);
  if (m_pRenderer)
    m_pRenderer->SetupScreenshot();
}

CRenderCapture* CXBMCRenderManager::AllocRenderCapture()
{
  return new CRenderCapture;
}

void CXBMCRenderManager::ReleaseRenderCapture(CRenderCapture* capture)
{
  CSingleLock lock(m_captCritSect);

  RemoveCapture(capture);

  //because a CRenderCapture might have some gl things allocated, it can only be deleted from app thread
  if (g_application.IsCurrentThread())
  {
    delete capture;
  }
  else
  {
    capture->SetState(CAPTURESTATE_NEEDSDELETE);
    m_captures.push_back(capture);
  }

  if (!m_captures.empty())
    m_hasCaptures = true;
}

void CXBMCRenderManager::Capture(CRenderCapture* capture, unsigned int width, unsigned int height, int flags)
{
  CSingleLock lock(m_captCritSect);

  RemoveCapture(capture);

  capture->SetState(CAPTURESTATE_NEEDSRENDER);
  capture->SetUserState(CAPTURESTATE_WORKING);
  capture->SetWidth(width);
  capture->SetHeight(height);
  capture->SetFlags(flags);
  capture->GetEvent().Reset();

  if (g_application.IsCurrentThread())
  {
    if (flags & CAPTUREFLAG_IMMEDIATELY)
    {
      //render capture and read out immediately
      RenderCapture(capture);
      capture->SetUserState(capture->GetState());
      capture->GetEvent().Set();
    }

    if ((flags & CAPTUREFLAG_CONTINUOUS) || !(flags & CAPTUREFLAG_IMMEDIATELY))
    {
      //schedule this capture for a render and readout
      m_captures.push_back(capture);
    }
  }
  else
  {
    //schedule this capture for a render and readout
    m_captures.push_back(capture);
  }

  if (!m_captures.empty())
    m_hasCaptures = true;
}

void CXBMCRenderManager::ManageCaptures()
{
  //no captures, return here so we don't do an unnecessary lock
  if (!m_hasCaptures)
    return;

  CSingleLock lock(m_captCritSect);

  std::list<CRenderCapture*>::iterator it = m_captures.begin();
  while (it != m_captures.end())
  {
    CRenderCapture* capture = *it;

    if (capture->GetState() == CAPTURESTATE_NEEDSDELETE)
    {
      delete capture;
      it = m_captures.erase(it);
      continue;
    }

    if (capture->GetState() == CAPTURESTATE_NEEDSRENDER)
      RenderCapture(capture);
    else if (capture->GetState() == CAPTURESTATE_NEEDSREADOUT)
      capture->ReadOut();

    if (capture->GetState() == CAPTURESTATE_DONE || capture->GetState() == CAPTURESTATE_FAILED)
    {
      //tell the thread that the capture is done or has failed
      capture->SetUserState(capture->GetState());
      capture->GetEvent().Set();

      if (capture->GetFlags() & CAPTUREFLAG_CONTINUOUS)
      {
        capture->SetState(CAPTURESTATE_NEEDSRENDER);

        //if rendering this capture continuously, and readout is async, render a new capture immediately
        if (capture->IsAsync() && !(capture->GetFlags() & CAPTUREFLAG_IMMEDIATELY))
          RenderCapture(capture);

        ++it;
      }
      else
      {
        it = m_captures.erase(it);
      }
    }
    else
    {
      ++it;
    }
  }

  if (m_captures.empty())
    m_hasCaptures = false;
}

void CXBMCRenderManager::RenderCapture(CRenderCapture* capture)
{
  CSharedLock lock(m_sharedSection);
  if (!m_pRenderer || !m_pRenderer->RenderCapture(capture))
    capture->SetState(CAPTURESTATE_FAILED);
}

void CXBMCRenderManager::RemoveCapture(CRenderCapture* capture)
{
  //remove this CRenderCapture from the list
  std::list<CRenderCapture*>::iterator it;
  while ((it = find(m_captures.begin(), m_captures.end(), capture)) != m_captures.end())
    m_captures.erase(it);
}

void CXBMCRenderManager::SetViewMode(int iViewMode)
{
  CSharedLock lock(m_sharedSection);
  if (m_pRenderer)
    m_pRenderer->SetViewMode(iViewMode);
}

void CXBMCRenderManager::FlipPage(volatile bool& bStop, double timestamp /* = 0LL*/, int source /*= -1*/, EFIELDSYNC sync /*= FS_NONE*/)
{
  { CSharedLock lock(m_sharedSection);

    if(bStop)
      return;

    if(!m_pRenderer) return;

    EPRESENTMETHOD presentmethod;

    EDEINTERLACEMODE deinterlacemode = CMediaSettings::Get().GetCurrentVideoSettings().m_DeinterlaceMode;
    EINTERLACEMETHOD interlacemethod = AutoInterlaceMethodInternal(CMediaSettings::Get().GetCurrentVideoSettings().m_InterlaceMethod);

    if(g_advancedSettings.m_videoDisableBackgroundDeinterlace && !g_graphicsContext.IsFullScreenVideo())
      deinterlacemode = VS_DEINTERLACEMODE_OFF;

    if (deinterlacemode == VS_DEINTERLACEMODE_OFF)
      presentmethod = PRESENT_METHOD_SINGLE;
    else
    {
      if (deinterlacemode == VS_DEINTERLACEMODE_AUTO && sync == FS_NONE)
        presentmethod = PRESENT_METHOD_SINGLE;
      else
      {
        bool invert = false;
        if      (interlacemethod == VS_INTERLACEMETHOD_RENDER_BLEND)            presentmethod = PRESENT_METHOD_BLEND;
        else if (interlacemethod == VS_INTERLACEMETHOD_RENDER_WEAVE)            presentmethod = PRESENT_METHOD_WEAVE;
        else if (interlacemethod == VS_INTERLACEMETHOD_RENDER_WEAVE_INVERTED) { presentmethod = PRESENT_METHOD_WEAVE ; invert = true; }
        else if (interlacemethod == VS_INTERLACEMETHOD_RENDER_BOB)              presentmethod = PRESENT_METHOD_BOB;
        else if (interlacemethod == VS_INTERLACEMETHOD_RENDER_BOB_INVERTED)   { presentmethod = PRESENT_METHOD_BOB; invert = true; }
        else if (interlacemethod == VS_INTERLACEMETHOD_DXVA_BOB)                presentmethod = PRESENT_METHOD_BOB;
        else if (interlacemethod == VS_INTERLACEMETHOD_DXVA_BEST)               presentmethod = PRESENT_METHOD_BOB;
        else                                                                    presentmethod = PRESENT_METHOD_SINGLE;

        /* default to odd field if we want to deinterlace and don't know better */
        if (deinterlacemode == VS_DEINTERLACEMODE_FORCE && sync == FS_NONE)
          sync = FS_TOP;

        /* invert present field */
        if(invert)
        {
          if( sync == FS_BOT )
            sync = FS_TOP;
          else
            sync = FS_BOT;
        }
      }
    }

    /* failsafe for invalid timestamps, to make sure queue always empties */
    if(timestamp > GetPresentTime() + 5.0)
      timestamp = GetPresentTime() + 5.0;

    CSingleLock lock2(m_presentlock);

    if(m_free.empty())
      return;

    if(source < 0)
      source = m_free.front();

    SPresent& m = m_Queue[source];
    m.timestamp     = timestamp;
    m.presentfield  = sync;
    m.presentmethod = presentmethod;
    requeue(m_queued, m_free);

    /* signal to any waiters to check state */
    if(m_presentstep == PRESENT_IDLE)
    {
      m_presentstep = PRESENT_READY;
      m_presentevent.notifyAll();
    }
  }
}

void CXBMCRenderManager::Reset()
{
  CSharedLock lock(m_sharedSection);
  if (m_pRenderer)
    m_pRenderer->Reset();
}

RESOLUTION CXBMCRenderManager::GetResolution()
{
  CSharedLock lock(m_sharedSection);
  if (m_pRenderer)
    return m_pRenderer->GetResolution();
  else
    return RES_INVALID;
}

float CXBMCRenderManager::GetMaximumFPS()
{
  float fps;

  if (CSettings::Get().GetInt("videoscreen.vsync") != VSYNC_DISABLED)
  {
    fps = (float)g_VideoReferenceClock.GetRefreshRate();
    if (fps <= 0) fps = g_graphicsContext.GetFPS();
  }
  else
    fps = 1000.0f;

  return fps;
}

void CXBMCRenderManager::RegisterRenderUpdateCallBack(const void *ctx, RenderUpdateCallBackFn fn)
{
  if (m_pRenderer)
    m_pRenderer->RegisterRenderUpdateCallBack(ctx, fn);
}

void CXBMCRenderManager::RegisterRenderFeaturesCallBack(const void *ctx, RenderFeaturesCallBackFn fn)
{
  if (m_pRenderer)
    m_pRenderer->RegisterRenderFeaturesCallBack(ctx, fn);
}

void CXBMCRenderManager::Render(bool clear, DWORD flags, DWORD alpha)
{
  CSharedLock lock(m_sharedSection);

  SPresent& m = m_Queue[m_presentsource];

  if( m.presentmethod == PRESENT_METHOD_BOB )
    PresentFields(clear, flags, alpha);
  else if( m.presentmethod == PRESENT_METHOD_WEAVE )
    PresentFields(clear, flags | RENDER_FLAG_WEAVE, alpha);
  else if( m.presentmethod == PRESENT_METHOD_BLEND )
    PresentBlend(clear, flags, alpha);
  else
    PresentSingle(clear, flags, alpha);

  m_overlays.Render(m_presentsource);
}

/* simple present method */
void CXBMCRenderManager::PresentSingle(bool clear, DWORD flags, DWORD alpha)
{
  CSingleLock lock(g_graphicsContext);

  m_pRenderer->RenderUpdate(clear, flags, alpha);
}

/* new simpler method of handling interlaced material, *
 * we just render the two fields right after eachother */
void CXBMCRenderManager::PresentFields(bool clear, DWORD flags, DWORD alpha)
{
  CSingleLock lock(g_graphicsContext);
  SPresent& m = m_Queue[m_presentsource];

  if(m_presentstep == PRESENT_FRAME)
  {
    if( m.presentfield == FS_BOT)
      m_pRenderer->RenderUpdate(clear, flags | RENDER_FLAG_BOT | RENDER_FLAG_FIELD0, alpha);
    else
      m_pRenderer->RenderUpdate(clear, flags | RENDER_FLAG_TOP | RENDER_FLAG_FIELD0, alpha);
  }
  else
  {
    if( m.presentfield == FS_TOP)
      m_pRenderer->RenderUpdate(clear, flags | RENDER_FLAG_BOT | RENDER_FLAG_FIELD1, alpha);
    else
      m_pRenderer->RenderUpdate(clear, flags | RENDER_FLAG_TOP | RENDER_FLAG_FIELD1, alpha);
  }
}

void CXBMCRenderManager::PresentBlend(bool clear, DWORD flags, DWORD alpha)
{
  CSingleLock lock(g_graphicsContext);
  SPresent& m = m_Queue[m_presentsource];

  if( m.presentfield == FS_BOT )
  {
    m_pRenderer->RenderUpdate(clear, flags | RENDER_FLAG_BOT | RENDER_FLAG_NOOSD, alpha);
    m_pRenderer->RenderUpdate(false, flags | RENDER_FLAG_TOP, alpha / 2);
  }
  else
  {
    m_pRenderer->RenderUpdate(clear, flags | RENDER_FLAG_TOP | RENDER_FLAG_NOOSD, alpha);
    m_pRenderer->RenderUpdate(false, flags | RENDER_FLAG_BOT, alpha / 2);
  }
}

void CXBMCRenderManager::Recover()
{
#if defined(HAS_GL) && !defined(TARGET_DARWIN)
  glFlush(); // attempt to have gpu done with pixmap and vdpau
#endif

  UpdateDisplayLatency();
}

void CXBMCRenderManager::UpdateDisplayLatency()
{
  float refresh = g_graphicsContext.GetFPS();
  if (g_graphicsContext.GetVideoResolution() == RES_WINDOW)
    refresh = 0; // No idea about refresh rate when windowed, just get the default latency
  m_displayLatency = (double) g_advancedSettings.GetDisplayLatency(refresh);
  CLog::Log(LOGDEBUG, "CRenderManager::UpdateDisplayLatency - Latency set to %1.0f msec", m_displayLatency * 1000.0f);
}

void CXBMCRenderManager::UpdateResolution()
{
  if (m_bReconfigured)
  {
    CRetakeLock<CExclusiveLock> lock(m_sharedSection);
    if (g_graphicsContext.IsFullScreenVideo() && g_graphicsContext.IsFullScreenRoot())
    {
      RESOLUTION res = GetResolution();
      g_graphicsContext.SetVideoResolution(res);
    }
    m_bReconfigured = false;
  }
}


unsigned int CXBMCRenderManager::GetProcessorSize()
{
  CSharedLock lock(m_sharedSection);
  return std::max(4, NUM_BUFFERS);
}

// Supported pixel formats, can be called before configure
std::vector<ERenderFormat> CXBMCRenderManager::SupportedFormats()
{
  CSharedLock lock(m_sharedSection);
  if (m_pRenderer)
    return m_pRenderer->SupportedFormats();
  return std::vector<ERenderFormat>();
}

int CXBMCRenderManager::AddVideoPicture(DVDVideoPicture& pic)
{
  CSharedLock lock(m_sharedSection);
  if (!m_pRenderer)
    return -1;

  int index;
  {
    CSingleLock lock(m_presentlock);
    if (m_free.empty())
      return -1;
    index = m_free.front();
  }

  if(m_pRenderer->AddVideoPicture(&pic, index))
    return 1;

  YV12Image image;
  if (m_pRenderer->GetImage(&image, index) < 0)
    return -1;

  if(pic.format == RENDER_FMT_YUV420P
  || pic.format == RENDER_FMT_YUV420P10
  || pic.format == RENDER_FMT_YUV420P16)
  {
    CDVDCodecUtils::CopyPicture(&image, &pic);
  }
  else if(pic.format == RENDER_FMT_NV12)
  {
    CDVDCodecUtils::CopyNV12Picture(&image, &pic);
  }
  else if(pic.format == RENDER_FMT_YUYV422
       || pic.format == RENDER_FMT_UYVY422)
  {
    CDVDCodecUtils::CopyYUV422PackedPicture(&image, &pic);
  }
  else if(pic.format == RENDER_FMT_DXVA)
  {
    CDVDCodecUtils::CopyDXVA2Picture(&image, &pic);
  }
#ifdef HAVE_LIBVDPAU
  else if(pic.format == RENDER_FMT_VDPAU)
    m_pRenderer->AddProcessor(pic.vdpau, index);
#endif
#ifdef HAVE_LIBOPENMAX
  else if(pic.format == RENDER_FMT_OMXEGL)
    m_pRenderer->AddProcessor(pic.openMax, &pic, index);
#endif
#ifdef TARGET_DARWIN
  else if(pic.format == RENDER_FMT_CVBREF)
    m_pRenderer->AddProcessor(pic.cvBufferRef, index);
#endif
#ifdef HAVE_LIBVA
  else if(pic.format == RENDER_FMT_VAAPI)
    m_pRenderer->AddProcessor(*pic.vaapi, index);
#endif
  m_pRenderer->ReleaseImage(index, false);

  return index;
}

bool CXBMCRenderManager::Supports(ERENDERFEATURE feature)
{
  CSharedLock lock(m_sharedSection);
  if (m_pRenderer)
    return m_pRenderer->Supports(feature);
  else
    return false;
}

bool CXBMCRenderManager::Supports(EDEINTERLACEMODE method)
{
  CSharedLock lock(m_sharedSection);
  if (m_pRenderer)
    return m_pRenderer->Supports(method);
  else
    return false;
}

bool CXBMCRenderManager::Supports(EINTERLACEMETHOD method)
{
  CSharedLock lock(m_sharedSection);
  if (m_pRenderer)
    return m_pRenderer->Supports(method);
  else
    return false;
}

bool CXBMCRenderManager::Supports(ESCALINGMETHOD method)
{
  CSharedLock lock(m_sharedSection);
  if (m_pRenderer)
    return m_pRenderer->Supports(method);
  else
    return false;
}

EINTERLACEMETHOD CXBMCRenderManager::AutoInterlaceMethod(EINTERLACEMETHOD mInt)
{
  CSharedLock lock(m_sharedSection);
  return AutoInterlaceMethodInternal(mInt);
}

EINTERLACEMETHOD CXBMCRenderManager::AutoInterlaceMethodInternal(EINTERLACEMETHOD mInt)
{
  if (mInt == VS_INTERLACEMETHOD_NONE)
    return VS_INTERLACEMETHOD_NONE;

  if(!m_pRenderer->Supports(mInt))
    mInt = VS_INTERLACEMETHOD_AUTO;

  if (mInt == VS_INTERLACEMETHOD_AUTO)
    return m_pRenderer->AutoInterlaceMethod();

  return mInt;
}

int CXBMCRenderManager::WaitForBuffer(volatile bool& bStop, int timeout)
{
  CSingleLock lock2(m_presentlock);

  XbmcThreads::EndTime endtime(timeout);
  while(m_free.empty())
  {
    m_presentevent.wait(lock2, std::min(50, timeout));
    if(endtime.IsTimePast() || bStop)
    {
      if (timeout != 0 && !bStop)
        CLog::Log(LOGWARNING, "CRenderManager::WaitForBuffer - timeout waiting for buffer");
      return -1;
    }
  }

  // make sure overlay buffer is released, this won't happen on AddOverlay
  m_overlays.Release(m_free.front());

  // return buffer level
  return m_queued.size() + m_discard.size();;
}

void CXBMCRenderManager::PrepareNextRender()
{
  CSingleLock lock(m_presentlock);

  if (m_queued.empty())
  {
    CLog::Log(LOGERROR, "CRenderManager::PrepareNextRender - asked to prepare with nothing available");
    m_presentstep = PRESENT_IDLE;
    m_presentevent.notifyAll();
    return;
  }

  double clocktime = GetPresentTime();
  double frametime = 1.0 / GetMaximumFPS();

  /* see if any future queued frames are already due */
  std::deque<int>::reverse_iterator curr, prev;
  int idx;
  curr = prev = m_queued.rbegin();
  ++prev;
  while (prev != m_queued.rend())
  {
    if(clocktime > m_Queue[*prev].timestamp                 /* previous frame is late */
    && clocktime > m_Queue[*curr].timestamp - frametime)    /* selected frame is close to it's display time */
      break;
    ++curr;
    ++prev;
  }
  idx = *curr;

  /* in fullscreen we will block after render, but only for MAXPRESENTDELAY */
  bool next;
  if(g_graphicsContext.IsFullScreenVideo())
    next = (m_Queue[idx].timestamp <= clocktime + MAXPRESENTDELAY);
  else
    next = (m_Queue[idx].timestamp <= clocktime + frametime);

  if (next)
  {
    /* skip late frames */
    while(m_queued.front() != idx)
    {
      requeue(m_discard, m_queued);
      m_QueueSkip++;
    }

    m_presentstep   = PRESENT_FLIP;
    m_discard.push_back(m_presentsource);
    m_presentsource = idx;
    m_queued.pop_front();
    m_presentevent.notifyAll();
  }
}

void CXBMCRenderManager::DiscardBuffer()
{
  CSharedLock lock(m_sharedSection);
  CSingleLock lock2(m_presentlock);

  while(!m_queued.empty())
    requeue(m_discard, m_queued);

  if(m_presentstep == PRESENT_READY)
    m_presentstep   = PRESENT_IDLE;
  m_presentevent.notifyAll();
}
