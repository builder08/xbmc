#pragma once

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

#ifndef DVDSUBTITLES_DLLLIBASS_H_INCLUDED
#define DVDSUBTITLES_DLLLIBASS_H_INCLUDED
#include "DllLibass.h"
#endif

#ifndef DVDSUBTITLES_DVDRESOURCE_H_INCLUDED
#define DVDSUBTITLES_DVDRESOURCE_H_INCLUDED
#include "DVDResource.h"
#endif

#ifndef DVDSUBTITLES_THREADS_CRITICALSECTION_H_INCLUDED
#define DVDSUBTITLES_THREADS_CRITICALSECTION_H_INCLUDED
#include "threads/CriticalSection.h"
#endif


/** Wrapper for Libass **/

class CDVDSubtitlesLibass : public IDVDResourceCounted<CDVDSubtitlesLibass>
{
public:
  CDVDSubtitlesLibass();
  virtual ~CDVDSubtitlesLibass();

  ASS_Image* RenderImage(int imageWidth, int imageHeight, double pts, int* changes = NULL);
  ASS_Event* GetEvents();

  int GetNrOfEvents();

  bool DecodeHeader(char* data, int size);
  bool DecodeDemuxPkt(char* data, int size, double start, double duration);
  bool CreateTrack(char* buf, size_t size);

private:
  DllLibass m_dll;
  long m_references;
  ASS_Library* m_library;
  ASS_Track* m_track;
  ASS_Renderer* m_renderer;
  CCriticalSection m_section;
};

