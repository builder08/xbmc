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
#include "DVDSubtitleParserSubrip.h"
#include "DVDCodecs/Overlay/DVDOverlayText.h"
#include "DVDClock.h"

using namespace std;

CDVDSubtitleParserSubrip::CDVDSubtitleParserSubrip(CDVDSubtitleStream* pStream, const string& strFile)
    : CDVDSubtitleParser(pStream, strFile)
{
}

CDVDSubtitleParserSubrip::~CDVDSubtitleParserSubrip()
{
  Dispose();
}

bool CDVDSubtitleParserSubrip::Open(CDVDStreamInfo &hints)
{
  if (m_pStream->Open(m_strFileName))
  {
    ParseFile();
    m_pStream->Close();

    return true;
  }

  return false;
}

void CDVDSubtitleParserSubrip::Dispose()
{
  //m_pStream->Close();
  m_collection.Clear();
}

void CDVDSubtitleParserSubrip::Reset()
{
  m_collection.Reset();
}

// parse exactly one subtitle
CDVDOverlay* CDVDSubtitleParserSubrip::Parse(double iPts)
{
  CDVDOverlay* pOverlay = m_collection.Get(iPts);
  return pOverlay;
}

int CDVDSubtitleParserSubrip::ParseFile()
{
  char line[1024];
  char* pLineStart;

  while (m_pStream->ReadLine(line, sizeof(line)))
  {
    pLineStart = line;

    // trim
    while (pLineStart[0] == ' ') pLineStart++;

    if (strlen(pLineStart) > 0)
    {
      char sep;
      int hh1, mm1, ss1, ms1, hh2, mm2, ss2, ms2;
      int c = sscanf(line, "%d%c%d%c%d%c%d --> %d%c%d%c%d%c%d\n",
                     &hh1, &sep, &mm1, &sep, &ss1, &sep, &ms1,
                     &hh2, &sep, &mm2, &sep, &ss2, &sep, &ms2);

      if (c == 1) 
      {
        // numbering, skip it
      }
      else if (c == 14) // time info
      {
        CDVDOverlayText* pOverlay = new CDVDOverlayText();
        pOverlay->Acquire(); // increase ref count with one so that we can hold a handle to this overlay

        pOverlay->iPTSStartTime = ((double)(((hh1 * 60 + mm1) * 60) + ss1) * 1000 + ms1) * (DVD_TIME_BASE / 1000);
        pOverlay->iPTSStopTime  = ((double)(((hh2 * 60 + mm2) * 60) + ss2) * 1000 + ms2) * (DVD_TIME_BASE / 1000);

        while (m_pStream->ReadLine(line, sizeof(line)))
        {
          pLineStart = line;
          // trim
          while (pLineStart[0] == ' ') pLineStart++;

          // empty line, next subtitle is about to start
          if (strlen(pLineStart) <= 0) break;

          CStdStringW strUTF16;
          CStdStringA strUTF8;

          if (g_charsetConverter.isValidUtf8(line))
            strUTF8 = line;
          else
          {  
            g_charsetConverter.subtitleCharsetToW(line, strUTF16);
            g_charsetConverter.wToUTF8(strUTF16,strUTF8);
          }

          // add a new text element to our container
          pOverlay->AddElement(new CDVDOverlayText::CElementText(strUTF8.c_str()));
        }

        m_collection.Add(pOverlay);
      }
    }
  }

  return m_collection.GetSize();
}

