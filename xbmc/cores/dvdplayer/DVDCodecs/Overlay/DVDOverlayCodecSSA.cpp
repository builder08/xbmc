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

#ifndef OVERLAY_DVDOVERLAYCODECSSA_H_INCLUDED
#define OVERLAY_DVDOVERLAYCODECSSA_H_INCLUDED
#include "DVDOverlayCodecSSA.h"
#endif

#ifndef OVERLAY_DVDOVERLAYSSA_H_INCLUDED
#define OVERLAY_DVDOVERLAYSSA_H_INCLUDED
#include "DVDOverlaySSA.h"
#endif

#ifndef OVERLAY_DVDSTREAMINFO_H_INCLUDED
#define OVERLAY_DVDSTREAMINFO_H_INCLUDED
#include "DVDStreamInfo.h"
#endif

#ifndef OVERLAY_DVDCODECS_DVDCODECS_H_INCLUDED
#define OVERLAY_DVDCODECS_DVDCODECS_H_INCLUDED
#include "DVDCodecs/DVDCodecs.h"
#endif

#ifndef OVERLAY_DVDCLOCK_H_INCLUDED
#define OVERLAY_DVDCLOCK_H_INCLUDED
#include "DVDClock.h"
#endif

#ifndef OVERLAY_UTIL_H_INCLUDED
#define OVERLAY_UTIL_H_INCLUDED
#include "Util.h"
#endif

#ifndef OVERLAY_UTILS_AUTOPTRHANDLE_H_INCLUDED
#define OVERLAY_UTILS_AUTOPTRHANDLE_H_INCLUDED
#include "utils/AutoPtrHandle.h"
#endif

#ifndef OVERLAY_UTILS_STRINGUTILS_H_INCLUDED
#define OVERLAY_UTILS_STRINGUTILS_H_INCLUDED
#include "utils/StringUtils.h"
#endif


using namespace AUTOPTR;
using namespace std;

CDVDOverlayCodecSSA::CDVDOverlayCodecSSA() : CDVDOverlayCodec("SSA Subtitle Decoder")
{
  m_pOverlay = NULL;
  m_libass   = NULL;
  m_order    = 0;
  m_output   = false;
}

CDVDOverlayCodecSSA::~CDVDOverlayCodecSSA()
{
  Dispose();
}

bool CDVDOverlayCodecSSA::Open(CDVDStreamInfo &hints, CDVDCodecOptions &options)
{
  if(hints.codec != AV_CODEC_ID_SSA)
    return false;

  Dispose();

  m_hints  = hints;
  m_libass = new CDVDSubtitlesLibass();
  return m_libass->DecodeHeader((char *)hints.extradata, hints.extrasize);
}

void CDVDOverlayCodecSSA::Dispose()
{
  if(m_libass)
    SAFE_RELEASE(m_libass);

  if(m_pOverlay)
    SAFE_RELEASE(m_pOverlay);
}

int CDVDOverlayCodecSSA::Decode(DemuxPacket *pPacket)
{
  if(!pPacket)
    return OC_ERROR;
  
  double pts = pPacket->dts != DVD_NOPTS_VALUE ? pPacket->dts : pPacket->pts;
  uint8_t *data = pPacket->pData;
  int size = pPacket->iSize;
  double duration = pPacket->duration;
  if(duration == DVD_NOPTS_VALUE)
    duration = 0.0;

  if(strncmp((const char*)data, "Dialogue:", 9) == 0)
  {
    int    sh, sm, ss, sc, eh, em, es, ec;
    double beg, end;
    size_t pos;
    CStdString      line, line2;
    std::vector<std::string> lines;
    StringUtils::Tokenize((const char*)data, lines, "\r\n");
    for(size_t i=0; i<lines.size(); i++)
    {
      line = lines[i];
      StringUtils::Trim(line);
      auto_aptr<char> layer(new char[line.length()+1]);

      if(sscanf(line.c_str(), "%*[^:]:%[^,],%d:%d:%d%*c%d,%d:%d:%d%*c%d"
                            , layer.get(), &sh, &sm, &ss, &sc, &eh,&em, &es, &ec) != 9)
        continue;

      end = 10000 * ((eh*360000.0)+(em*6000.0)+(es*100.0)+ec);
      beg = 10000 * ((sh*360000.0)+(sm*6000.0)+(ss*100.0)+sc);

      pos = line.find_first_of(",", 0);
      pos = line.find_first_of(",", pos+1);
      pos = line.find_first_of(",", pos+1);
      if(pos == CStdString::npos)
        continue;

      line2 = StringUtils::Format("%d,%s,%s", m_order++, layer.get(), line.substr(pos+1).c_str());

      m_libass->DecodeDemuxPkt((char*)line2.c_str(), line2.length(), beg, end - beg);

      /* setup time spanning all dialogs */
      if(pts == DVD_NOPTS_VALUE || beg < pts)
        pts = beg;
      if(end - pts > duration)
        duration = end - pts;
    }
  }
  else
    m_libass->DecodeDemuxPkt((char*)data, size, pts, duration);

  if(m_pOverlay)
  {
    /* there will only ever be one active, so we 
     * must always make sure any new one overlap
     * include the full duration of the old one */
    if(m_pOverlay->iPTSStopTime > pts + duration)
      duration = m_pOverlay->iPTSStopTime - pts;
    SAFE_RELEASE(m_pOverlay);
  }

  m_pOverlay = new CDVDOverlaySSA(m_libass);
  m_pOverlay->iPTSStartTime = pts;
  m_pOverlay->iPTSStopTime  = pts + duration;
  m_output = true;
  return OC_OVERLAY;
}
void CDVDOverlayCodecSSA::Reset()
{
  Dispose();
  m_order  = 0;
  m_output = false;
  m_libass = new CDVDSubtitlesLibass();
  m_libass->DecodeHeader((char *)m_hints.extradata, m_hints.extrasize);
}

void CDVDOverlayCodecSSA::Flush()
{
  Reset();
}

CDVDOverlay* CDVDOverlayCodecSSA::GetOverlay()
{
  if(m_output)
  {
    m_output = false;
    return m_pOverlay->Acquire();
  }
  return NULL;
}


