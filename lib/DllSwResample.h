#pragma once
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
 *  along with XBMC; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#if (defined HAVE_CONFIG_H) && (!defined WIN32)
  #include "config.h"
#endif
#include "DynamicDll.h"
#include "utils/log.h"

extern "C" {
#ifndef HAVE_MMX
#define HAVE_MMX
#endif
#ifndef __STDC_CONSTANT_MACROS
#define __STDC_CONSTANT_MACROS
#endif
#ifndef __GNUC__
#pragma warning(disable:4244)
#endif
#if (defined USE_EXTERNAL_FFMPEG)
  #if HAVE_LIBSWRESAMPLE_SWRESAMPLE_H
    #include <libswresample/swresample.h>
  #elif HAVE_LIBAVRESAMPLE_AVRESAMPLE_H
    #include <libavresample/avresample.h>
    #include <libavutil/opt.h>
    #include <libavutil/samplefmt.h>
    #define SwrContext AVAudioResampleContext
  #else
    #error "Either libswresample or libavresample is needed!"
  #endif
#else
  #include "libswresample/swresample.h"
#endif
}

#ifdef TARGET_WINDOWS
#pragma comment(lib, "swresample.lib")
#endif
