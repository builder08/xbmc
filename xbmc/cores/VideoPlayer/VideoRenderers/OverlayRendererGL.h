/*
 *      Initial code sponsored by: Voddler Inc (voddler.com)
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
#include "system_gl.h"
#include "OverlayRenderer.h"

class CDVDOverlay;
class CDVDOverlayImage;
class CDVDOverlaySpu;
class CDVDOverlaySSA;
typedef struct ass_image ASS_Image;

namespace OVERLAY {

  class COverlayTextureGL : public COverlay
  {
  public:
     explicit COverlayTextureGL(CDVDOverlayImage* o);
     explicit COverlayTextureGL(CDVDOverlaySpu* o);
    ~COverlayTextureGL() override;

    void Render(SRenderState& state) override;

    GLuint m_texture;
    float  m_u;
    float  m_v;
    bool   m_pma; /*< is alpha in texture premultiplied in the values */
  };

  class COverlayGlyphGL : public COverlay
  {
  public:
   COverlayGlyphGL(ASS_Image* images, int width, int height);

   ~COverlayGlyphGL() override;

   void Render(SRenderState& state) override;

    struct VERTEX
    {
       GLfloat u, v;
       GLubyte r, g, b, a;
       GLfloat x, y, z;
    };

   VERTEX* m_vertex;
   int     m_count;

   GLuint m_texture;
   float  m_u;
   float  m_v;
  };

}
