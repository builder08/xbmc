#pragma once

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

#include "system.h" // for SAFE_RELEASE
#include "DVDOverlay.h"
#include "../../DVDSubtitles/DVDSubtitlesLibass.h"

class CDVDOverlaySSA : public CDVDOverlay
{
public:

  CDVDSubtitlesLibass* m_libass;

  explicit CDVDOverlaySSA(CDVDSubtitlesLibass* libass) : CDVDOverlay(DVDOVERLAY_TYPE_SSA)
  {
    replace = true;
    m_libass = libass;
    libass->Acquire();
  }

  CDVDOverlaySSA(CDVDOverlaySSA& src)
    : CDVDOverlay(src)
    , m_libass(src.m_libass)
  {
    m_libass->Acquire();
  }

  ~CDVDOverlaySSA() override
  {
    if(m_libass)
      SAFE_RELEASE(m_libass);
  }

  CDVDOverlaySSA* Clone() override
  {
    return new CDVDOverlaySSA(*this);
  }
};
