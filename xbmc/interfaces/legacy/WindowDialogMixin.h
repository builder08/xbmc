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

#include "Window.h"

// These messages are a side effect of the way dialogs work through the
// main ApplicationMessenger. At some point it would be nice to remove
// the messenger and have direct (or even drive) communications.
#define HACK_CUSTOM_ACTION_CLOSING -3
#define HACK_CUSTOM_ACTION_OPENING -4

namespace XBMCAddon
{
  namespace xbmcgui
  {
    class WindowDialogMixin
    {
    private:
      Window* w;

    protected:
      inline explicit WindowDialogMixin(Window* window) : w(window) {}

    public:
      virtual ~WindowDialogMixin() = default;

      SWIGHIDDENVIRTUAL void show();
      SWIGHIDDENVIRTUAL void close();

#ifndef SWIG
      SWIGHIDDENVIRTUAL bool IsDialogRunning() const;
      SWIGHIDDENVIRTUAL bool OnAction(const CAction &action);
#endif
    };
  }
}
