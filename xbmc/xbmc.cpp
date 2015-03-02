/*
 *      Copyright (C) 2005-2013 Team XBMC
 *      http://xbmc.org
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

#include "xbmc.h"
#include "Application.h"
#include "settings/AdvancedSettings.h"

#ifdef TARGET_RASPBERRY_PI
#include "linux/RBP.h"
#endif

CAppOptions::CAppOptions()
{
  fullscreen = false;
  standalone = false;
  portable = false;
  renderGUI = true;
#ifdef _DEBUG
  debug = true;
#else
  debug = false;
#endif
}

extern "C" int XBMC_Run(const CAppOptions &options)
{
  g_application.SetStandAlone(options.standalone);
  g_application.EnablePlatformDirectories(!options.portable);

  for (std::vector<std::string>::const_iterator itr = options.settings.begin(); itr != options.settings.end(); itr++)
    g_advancedSettings.AddSettingsFile(*itr);

  int status = -1;

  if (!g_advancedSettings.Initialized())
  {
    g_advancedSettings.m_logLevel     = options.debug ? LOG_LEVEL_DEBUG : LOG_LEVEL_NORMAL;
    g_advancedSettings.m_logLevelHint = options.debug ? LOG_LEVEL_DEBUG : LOG_LEVEL_NORMAL;

    g_advancedSettings.Initialize();
  }

  g_advancedSettings.m_startFullScreen = options.fullscreen;

  if (!g_application.Create())
  {
    fprintf(stderr, "ERROR: Unable to create application. Exiting\n");
    return status;
  }

#ifdef TARGET_RASPBERRY_PI
  if(!g_RBP.Initialize())
    return false;
  g_RBP.LogFirmwareVerison();
#endif

  if (options.renderGUI && !g_application.CreateGUI())
  {
    fprintf(stderr, "ERROR: Unable to create GUI. Exiting\n");
    return status;
  }
  if (!g_application.Initialize())
  {
    fprintf(stderr, "ERROR: Unable to Initialize. Exiting\n");
    return status;
  }

  try
  {
    status = g_application.Run();
  }
  catch(...)
  {
    fprintf(stderr, "ERROR: Exception caught on main loop. Exiting\n");
    status = -1;
  }

#ifdef TARGET_RASPBERRY_PI
  g_RBP.Deinitialize();
#endif

  return status;
}
