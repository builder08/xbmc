/*
 *  Copyright (C) 2005-2018 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#pragma once

#include "WinSystemX11.h"

#include "EGL/egl.h"
#include "rendering/gl/RenderSystemGL.h"
#include "platform/linux/OptionalsReg.h"
#include <memory>

class CGLContext;
class CVaapiProxy;

class CWinSystemX11GLContext : public CWinSystemX11, public CRenderSystemGL
{
public:
  CWinSystemX11GLContext();
  ~CWinSystemX11GLContext() override;

  // Implementation of CWinSystem via CWinSystemX11
  CRenderSystemBase *GetRenderSystem() override { return this; }
  bool CreateNewWindow(const std::string& name, bool fullScreen, const RESOLUTION_INFO& res) override;
  bool ResizeWindow(int newWidth, int newHeight, int newLeft, int newTop) override;
  void FinishWindowResize(int newWidth, int newHeight) override;
  bool SetFullScreen(bool fullScreen, const RESOLUTION_INFO& res, bool blankOtherDisplays) override;
  bool DestroyWindowSystem() override;
  bool DestroyWindow() override;

  bool IsExtSupported(const char* extension) const override;

  // videosync
  std::unique_ptr<CVideoSync> GetVideoSync(void *clock) override;
  float GetFrameLatencyAdjustment() override;
  uint64_t GetVblankTiming(uint64_t &msc, uint64_t &interval);

  XID GetWindow() const;
  void* GetGlxContext() const;
  EGLDisplay GetEGLDisplay() const;
  EGLSurface GetEGLSurface() const;
  EGLContext GetEGLContext() const;
  EGLConfig GetEGLConfig() const;

protected:
  bool SetWindow(int width, int height, bool fullscreen, const std::string &output, int *winstate = NULL) override;
  void PresentRenderImpl(bool rendered) override;
  void SetVSyncImpl(bool enable) override;
  bool RefreshGLContext(bool force);
  XVisualInfo* GetVisual() override;

  CGLContext *m_pGLContext = nullptr;
  bool m_newGlContext;

  struct delete_CVaapiProxy
  {
    void operator()(CVaapiProxy *p) const;
  };
  std::unique_ptr<CVaapiProxy, delete_CVaapiProxy> m_vaapiProxy;

  std::unique_ptr<OPTIONALS::CLircContainer, OPTIONALS::delete_CLircContainer> m_lirc;
};
