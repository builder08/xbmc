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

#include "stdafx.h"
#include "LCDFactory.h"
#ifdef _XBOX
#include "lib/libXenium/XeniumLCD.h"
#include "lib/x3lcd/x3lcd.h"
#include "GUISettings.h"
#else
#include "../linux/XLCDproc.h"
#endif

ILCD* g_lcd = NULL;
CLCDFactory::CLCDFactory(void)
{}

CLCDFactory::~CLCDFactory(void)
{}

ILCD* CLCDFactory::Create()
{
#ifdef _XBOX
  switch (g_guiSettings.GetInt("lcd.modchip"))
  {
  case MODCHIP_XENIUM:
    return new CXeniumLCD();
    break;

  case MODCHIP_SMARTXX:
    return new CSmartXXLCD();
    break;

  case MODCHIP_XECUTER3:
    return new CX3LCD();
    break;
  }
  return new CSmartXXLCD();
#endif
#ifdef _LINUX
  return new XLCDproc();
#endif
}
