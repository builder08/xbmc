/*
 *      Copyright (C) 2005-2009 Team XBMC
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


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include "addon_local.h"
#include "xbmc_pvr_lib.h"

AddonCB *m_pvr_cb = NULL;

#ifdef __cplusplus
extern "C" {
#endif


void PVR_register_me(ADDON_HANDLE hdl)
{
  m_pvr_cb = (AddonCB*) hdl;

  return;
}

void PVR_event_callback(const PVR_EVENT event, const char* msg)
{
  if (m_pvr_cb == NULL)
    return;

  m_pvr_cb->PVR.EventCallback(m_pvr_cb->userData, event, msg);
}

void PVR_transfer_channel_entry(const PVRHANDLE handle, const PVR_CHANNEL *chan)
{
  if (m_pvr_cb == NULL)
    return;

  m_pvr_cb->PVR.TransferChannelEntry(m_pvr_cb->userData, handle, chan);
}

void PVR_transfer_timer_entry(const PVRHANDLE handle, const PVR_TIMERINFO *timer)
{
  if (m_pvr_cb == NULL)
    return;

  m_pvr_cb->PVR.TransferTimerEntry(m_pvr_cb->userData, handle, timer);
}

#ifdef __cplusplus
}
#endif


