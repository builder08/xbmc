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

#include "ModuleXbmcgui.h"
#include "LanguageHook.h"
#include "ServiceBroker.h"
#include "guilib/GUIComponent.h"
#include "windowing/GraphicContext.h"
#include "guilib/GUIWindowManager.h"

#define NOTIFICATION_INFO     "info"
#define NOTIFICATION_WARNING  "warning"
#define NOTIFICATION_ERROR    "error"

namespace XBMCAddon
{
  namespace xbmcgui
  {
    long getCurrentWindowId()
    {
      DelayedCallGuard dg;
      CSingleLock gl(CServiceBroker::GetWinSystem()->GetGfxContext());
      return CServiceBroker::GetGUI()->GetWindowManager().GetActiveWindow();
    }

    long getCurrentWindowDialogId()
    {
      DelayedCallGuard dg;
      CSingleLock gl(CServiceBroker::GetWinSystem()->GetGfxContext());
      return CServiceBroker::GetGUI()->GetWindowManager().GetTopmostModalDialog();
    }

    long getScreenHeight()
    {
      XBMC_TRACE;
      return CServiceBroker::GetWinSystem()->GetGfxContext().GetHeight();
    }

    long getScreenWidth()
    {
      XBMC_TRACE;
      return CServiceBroker::GetWinSystem()->GetGfxContext().GetWidth();
    }

    const char* getNOTIFICATION_INFO()    { return NOTIFICATION_INFO; }
    const char* getNOTIFICATION_WARNING() { return NOTIFICATION_WARNING; }
    const char* getNOTIFICATION_ERROR()   { return NOTIFICATION_ERROR; }

  }
}
