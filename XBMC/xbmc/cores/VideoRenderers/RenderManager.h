#pragma once

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

#if defined (HAS_GL)
#include "LinuxRendererGL.h"
#elif defined(HAS_GL)
#include "LinuxRenderer.h"
#elif defined(WIN32)
#include "WinRenderer.h"
#endif

#include "utils/SharedSection.h"
#include "utils/Thread.h"
#include "settings/VideoSettings.h"

class CXBoxRenderManager
{
public:
  CXBoxRenderManager();
  ~CXBoxRenderManager();

  // Functions called from the GUI
  void GetVideoRect(RECT &rs, RECT &rd) { CSharedLock lock(m_sharedSection); if (m_pRenderer) m_pRenderer->GetVideoRect(rs, rd); };
  float GetAspectRatio() { CSharedLock lock(m_sharedSection); if (m_pRenderer) return m_pRenderer->GetAspectRatio(); else return 1.0f; };
  void AutoCrop(bool bCrop = true) { CSharedLock lock(m_sharedSection); if (m_pRenderer) m_pRenderer->AutoCrop(bCrop); };
  void Update(bool bPauseDrawing);
  void RenderUpdate(bool clear, DWORD flags = 0, DWORD alpha = 255);
  void SetupScreenshot();

  void CreateThumbnail(XBMC::SurfacePtr surface, unsigned int width, unsigned int height);

  void SetViewMode(int iViewMode) { CSharedLock lock(m_sharedSection); if (m_pRenderer) m_pRenderer->SetViewMode(iViewMode); };

  // Functions called from mplayer
  bool Configure(unsigned int width, unsigned int height, unsigned int d_width, unsigned int d_height, float fps, unsigned flags);
  bool IsConfigured();

  // a call to GetImage must be followed by a call to releaseimage if getimage was successfull
  // failure to do so will result in deadlock
  inline int GetImage(YV12Image *image, int source = AUTOSOURCE, bool readonly = false)
  {
    CSharedLock lock(m_sharedSection);
    if (m_pRenderer)
      return m_pRenderer->GetImage(image, source, readonly);
    return -1;
  }
  inline void ReleaseImage(int source = AUTOSOURCE, bool preserve = false)
  {
    CSharedLock lock(m_sharedSection);
    if (m_pRenderer)
      m_pRenderer->ReleaseImage(source, preserve);
  }
  inline unsigned int DrawSlice(unsigned char *src[], int stride[], int w, int h, int x, int y)
  {
    CSharedLock lock(m_sharedSection);
    if (m_pRenderer)
      return m_pRenderer->DrawSlice(src, stride, w, h, x, y);
    return 0;
  }

  void FlipPage(volatile bool& bStop, double timestamp = 0.0, int source = -1, EFIELDSYNC sync = FS_NONE);
  unsigned int PreInit();
  void UnInit();

  inline void Reset()
  {
    CSharedLock lock(m_sharedSection);
    if (m_pRenderer)
      m_pRenderer->Reset();
  }
  RESOLUTION GetResolution()
  {
    CSharedLock lock(m_sharedSection);
    if (m_pRenderer)
      return m_pRenderer->GetResolution();
    else
      return INVALID;
  }

  float GetMaximumFPS();
  inline bool Paused() { return m_bPauseDrawing; };
  inline bool IsStarted() { return m_bIsStarted;}
  bool SupportsBrightness();
  bool SupportsContrast();
  bool SupportsGamma();

  double GetPresentTime();
  void  WaitPresentTime(double presenttime);

  CStdString GetVSyncState();

#ifdef HAS_GL
  CLinuxRendererGL *m_pRenderer;
#elif defined(HAS_GL)
  CLinuxRenderer *m_pRenderer;
#elif defined(HAS_XBOX_D3D)
  CXBoxRenderer *m_pRenderer;
#else
  CWinRenderer *m_pRenderer;
#endif

  void Present();
  void Recover(); // called after resolution switch if something special is needed

  CSharedSection& GetSection() { return m_sharedSection; };

protected:

  void PresentSingle();
  void PresentWeave();
  void PresentBob();
  void PresentBlend();

  bool m_bPauseDrawing;   // true if we should pause rendering

  bool m_bIsStarted;
  CSharedSection m_sharedSection;

  int m_rendermethod;

  double     m_presenttime;
  double     m_presentcorr;
  double     m_presenterr;
  EFIELDSYNC m_presentfield;
  EINTERLACEMETHOD m_presentmethod;
  int        m_presentstep;
  CEvent     m_presentevent;
};

extern CXBoxRenderManager g_renderManager;


