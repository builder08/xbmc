/*
 *      Copyright (C) 2005-present Team Kodi
 *      This file is part of Kodi - https://kodi.tv
 *
 *  Kodi is free software: you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  Kodi is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Kodi. If not, see <https://www.gnu.org/licenses/>.
 *
 */

#pragma once

#include "X11/Xlib.h"
#include <string>

class CGLContext
{
public:
  explicit CGLContext(Display *dpy)
  {
    m_dpy = dpy;
  }
  virtual ~CGLContext() = default;
  virtual bool Refresh(bool force, int screen, Window glWindow, bool &newContext) = 0;
  virtual void Destroy() = 0;
  virtual void Detach() = 0;
  virtual void SetVSync(bool enable) = 0;
  virtual void SwapBuffers() = 0;
  virtual void QueryExtensions() = 0;
  bool IsExtSupported(const char* extension) const;

  std::string ExtPrefix(){ return m_extPrefix; };
  std::string m_extPrefix;
  std::string m_extensions;

  Display *m_dpy;
};
