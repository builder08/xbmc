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
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include "GUITexture.h"
#include "GraphicContext.h"
#include "TextureManager.h"
#include "GUILargeTextureManager.h"
#include "utils/MathUtils.h"
#include "Texture.h"
#include "guilib/GUIWindowManager.h"
#include "rendering/SceneGraph.h"

using namespace std;

CTextureInfo::CTextureInfo()
{
  orientation = 0;
  useLarge = false;
}

CTextureInfo::CTextureInfo(const CStdString &file)
{
  orientation = 0;
  useLarge = false;
  filename = file;
}

CTextureInfo& CTextureInfo::operator=(const CTextureInfo &right)
{
  border = right.border;
  orientation = right.orientation;
  diffuse = right.diffuse;
  filename = right.filename;
  useLarge = right.useLarge;

  return *this;
}

CGUITextureBase::CGUITextureBase(float posX, float posY, float width, float height, const CTextureInfo& texture)
{
  m_posX = posX;
  m_posY = posY;
  m_width = width;
  m_height = height;
  m_info = texture;

  // defaults
  m_visible = true;
  m_diffuseColor = 0xffffffff;
  m_alpha = 0xff;

  m_vertex.SetRect(m_posX, m_posY, m_posX + m_width, m_posY + m_height);

  m_frameWidth = 0;
  m_frameHeight = 0;

  m_texCoordsScaleU = 1.0f;
  m_texCoordsScaleV = 1.0f;
  m_diffuseU = 1.0f;
  m_diffuseV = 1.0f;
  m_diffuseScaleU = 1.0f;
  m_diffuseScaleV = 1.0f;

  // anim gifs
  m_currentFrame = 0;
  m_frameCounter = (unsigned int) -1;
  m_currentLoop = 0;

  m_allocateDynamically = false;
  m_isAllocated = NO;
  m_invalid = true;
  m_vertexIndex = 0;
}

CGUITextureBase::CGUITextureBase(const CGUITextureBase &right)
{
  m_posX = right.m_posX;
  m_posY = right.m_posY;
  m_width = right.m_width;
  m_height = right.m_height;
  m_info = right.m_info;

  m_visible = right.m_visible;
  m_diffuseColor = right.m_diffuseColor;
  m_alpha = right.m_alpha;
  m_aspect = right.m_aspect;

  m_allocateDynamically = right.m_allocateDynamically;
  // defaults
  m_vertex.SetRect(m_posX, m_posY, m_posX + m_width, m_posY + m_height);

  m_frameWidth = 0;
  m_frameHeight = 0;

  m_texCoordsScaleU = 1.0f;
  m_texCoordsScaleV = 1.0f;
  m_diffuseU = 1.0f;
  m_diffuseV = 1.0f;
  m_diffuseScaleU = 1.0f;
  m_diffuseScaleV = 1.0f;

  m_currentFrame = 0;
  m_frameCounter = (unsigned int) -1;
  m_currentLoop = 0;

  m_isAllocated = NO;
  m_invalid = true;
  m_vertexIndex = 0;
}

CGUITextureBase::~CGUITextureBase(void)
{
}

bool CGUITextureBase::AllocateOnDemand()
{
  if (m_visible)
  { // visible, so make sure we're allocated
    if (!IsAllocated() || (m_isAllocated == LARGE && !m_texture.size()))
      return AllocResources();
  }
  else
  { // hidden, so deallocate as applicable
    if (m_allocateDynamically && IsAllocated())
      FreeResources();
    // reset animated textures (animgifs)
    m_currentLoop = 0;
    m_currentFrame = 0;
    m_frameCounter = 0;
  }

  return false;
}

bool CGUITextureBase::Process(unsigned int currentTime)
{
  bool changed = false;
  // check if we need to allocate our resources
  changed |= AllocateOnDemand();

  if (m_texture.size() > 1)
    changed |= UpdateAnimFrame();

  if (m_invalid)
    changed |= CalculateSize();

  return changed;
}

void CGUITextureBase::Render()
{
  if (!m_visible || !m_texture.size())
    return;

  // see if we need to clip the image
  if (m_vertex.Width() > m_width || m_vertex.Height() > m_height)
  {
    if (!g_graphicsContext.SetClipRegion(m_posX, m_posY, m_width, m_height))
      return;
  }

  // set our draw color
  #define MIX_ALPHA(a,c) (((a * (c >> 24)) / 255) << 24) | (c & 0x00ffffff)
  color_t color = m_diffuseColor;
  if (m_alpha != 0xFF) color = MIX_ALPHA(m_alpha, m_diffuseColor);
  color = g_graphicsContext.MergeAlpha(color);
  // compute the texture coordinates
  float u1, u2, u3, v1, v2, v3;
  u1 = m_info.border.x1;
  u2 = m_frameWidth - m_info.border.x2;
  u3 = m_frameWidth;
  v1 = m_info.border.y1;
  v2 = m_frameHeight - m_info.border.y2;
  v3 = m_frameHeight;

  if (!m_texture.m_texCoordsArePixels)
  {
    u1 *= m_texCoordsScaleU;
    u2 *= m_texCoordsScaleU;
    u3 *= m_texCoordsScaleU;
    v1 *= m_texCoordsScaleV;
    v2 *= m_texCoordsScaleV;
    v3 *= m_texCoordsScaleV;
  }

  // TODO: The diffuse coloring applies to all vertices, which will
  //       look weird for stuff with borders, as will the -ve height/width
  //       for flipping


  int reserve = 1;
  if (m_info.border.x1)
  {
    reserve++;
    reserve += m_info.border.y1 != 0;
    reserve += m_info.border.y2 != 0;
  }

  if (m_info.border.x2)
  {
    reserve++;
    reserve += m_info.border.y1 != 0;
    reserve += m_info.border.y2 != 0;
  }
  reserve += m_info.border.y1 != 0;
  reserve += m_info.border.y2 != 0;
  PackedVerticesPtr batchVertices = PackedVerticesPtr(new PackedVertices(reserve*4));

  // left segment (0,0,u1,v3)
  m_vertexIndex = 0;
  if (m_info.border.x1)
  {
    if (m_info.border.y1)
      Render(batchVertices, m_vertex.x1, m_vertex.y1, m_vertex.x1 + m_info.border.x1, m_vertex.y1 + m_info.border.y1, 0, 0, u1, v1, u3, v3);
    Render(batchVertices, m_vertex.x1, m_vertex.y1 + m_info.border.y1, m_vertex.x1 + m_info.border.x1, m_vertex.y2 - m_info.border.y2, 0, v1, u1, v2, u3, v3);
    if (m_info.border.y2)
      Render(batchVertices, m_vertex.x1, m_vertex.y2 - m_info.border.y2, m_vertex.x1 + m_info.border.x1, m_vertex.y2, 0, v2, u1, v3, u3, v3);
  }
  // middle segment (u1,0,u2,v3)
  if (m_info.border.y1)
    Render(batchVertices, m_vertex.x1 + m_info.border.x1, m_vertex.y1, m_vertex.x2 - m_info.border.x2, m_vertex.y1 + m_info.border.y1, u1, 0, u2, v1, u3, v3);
  Render(batchVertices, m_vertex.x1 + m_info.border.x1, m_vertex.y1 + m_info.border.y1, m_vertex.x2 - m_info.border.x2, m_vertex.y2 - m_info.border.y2, u1, v1, u2, v2, u3, v3);
  if (m_info.border.y2)
    Render(batchVertices, m_vertex.x1 + m_info.border.x1, m_vertex.y2 - m_info.border.y2, m_vertex.x2 - m_info.border.x2, m_vertex.y2, u1, v2, u2, v3, u3, v3);
  // right segment
  if (m_info.border.x2)
  { // have a left border
    if (m_info.border.y1)
      Render(batchVertices, m_vertex.x2 - m_info.border.x2, m_vertex.y1, m_vertex.x2, m_vertex.y1 + m_info.border.y1, u2, 0, u3, v1, u3, v3);
    Render(batchVertices, m_vertex.x2 - m_info.border.x2, m_vertex.y1 + m_info.border.y1, m_vertex.x2, m_vertex.y2 - m_info.border.y2, u2, v1, u3, v2, u3, v3);
    if (m_info.border.y2)
      Render(batchVertices, m_vertex.x2 - m_info.border.x2, m_vertex.y2 - m_info.border.y2, m_vertex.x2, m_vertex.y2, u2, v2, u3, v3, u3, v3);
  }
  m_vertexIndex = 0;

  CBatchDrawPtr batchDraw = CBatchDrawPtr(new CBatchDraw);
  batchDraw->SetTexture(m_texture.m_textures[m_currentFrame]);
  batchDraw->SetColor(color);
  batchDraw->SetVertices(batchVertices);
  if (m_diffuse.size())
    batchDraw->SetDiffuseTexture(m_diffuse.m_textures[0]);

  CSceneGraph *sceneGraph = g_Windowing.GetSceneGraph();
  sceneGraph->Add(batchDraw);

  if (m_vertex.Width() > m_width || m_vertex.Height() > m_height)
    g_graphicsContext.RestoreClipRegion();
}

void CGUITextureBase::Render(PackedVerticesPtr batchVertices, float left, float top, float right, float bottom, float u1, float v1, float u2, float v2, float u3, float v3)
{
  CRect diffuse(u1, v1, u2, v2);
  CRect texture(u1, v1, u2, v2);
  CRect vertex(left, top, right, bottom);
  g_graphicsContext.ClipRect(vertex, texture, m_diffuse.size() ? &diffuse : NULL);

  if (vertex.IsEmpty())
    return; // nothing to render

  int orientation = GetOrientation();
  OrientateTexture(texture, u3, v3, orientation);

  if (m_diffuse.size())
  {
    // flip the texture as necessary.  Diffuse just gets flipped according to m_info.orientation.
    // Main texture gets flipped according to GetOrientation().
    diffuse.x1 *= m_diffuseScaleU / u3; diffuse.x2 *= m_diffuseScaleU / u3;
    diffuse.y1 *= m_diffuseScaleV / v3; diffuse.y2 *= m_diffuseScaleV / v3;
    diffuse += m_diffuseOffset;
    OrientateTexture(diffuse, m_diffuseU, m_diffuseV, m_info.orientation);
  }

#define ROUND_TO_PIXEL(x) (float)(MathUtils::round_int(x))

  batchVertices->at(m_vertexIndex+0).x = ROUND_TO_PIXEL(g_graphicsContext.ScaleFinalXCoord(vertex.x1, vertex.y1));
  batchVertices->at(m_vertexIndex+0).y = ROUND_TO_PIXEL(g_graphicsContext.ScaleFinalYCoord(vertex.x1, vertex.y1));
  batchVertices->at(m_vertexIndex+0).z = ROUND_TO_PIXEL(g_graphicsContext.ScaleFinalZCoord(vertex.x1, vertex.y1));
  batchVertices->at(m_vertexIndex+1).x = ROUND_TO_PIXEL(g_graphicsContext.ScaleFinalXCoord(vertex.x2, vertex.y1));
  batchVertices->at(m_vertexIndex+1).y = ROUND_TO_PIXEL(g_graphicsContext.ScaleFinalYCoord(vertex.x2, vertex.y1));
  batchVertices->at(m_vertexIndex+1).z = ROUND_TO_PIXEL(g_graphicsContext.ScaleFinalZCoord(vertex.x2, vertex.y1));
  batchVertices->at(m_vertexIndex+2).x = ROUND_TO_PIXEL(g_graphicsContext.ScaleFinalXCoord(vertex.x2, vertex.y2));
  batchVertices->at(m_vertexIndex+2).y = ROUND_TO_PIXEL(g_graphicsContext.ScaleFinalYCoord(vertex.x2, vertex.y2));
  batchVertices->at(m_vertexIndex+2).z = ROUND_TO_PIXEL(g_graphicsContext.ScaleFinalZCoord(vertex.x2, vertex.y2));
  batchVertices->at(m_vertexIndex+3).x = ROUND_TO_PIXEL(g_graphicsContext.ScaleFinalXCoord(vertex.x1, vertex.y2));
  batchVertices->at(m_vertexIndex+3).y = ROUND_TO_PIXEL(g_graphicsContext.ScaleFinalYCoord(vertex.x1, vertex.y2));
  batchVertices->at(m_vertexIndex+3).z = ROUND_TO_PIXEL(g_graphicsContext.ScaleFinalZCoord(vertex.x1, vertex.y2));

  if (batchVertices->at(m_vertexIndex+2).y == batchVertices->at(m_vertexIndex+0).y) batchVertices->at(m_vertexIndex+2).y += 1.0f;
  if (batchVertices->at(m_vertexIndex+3).x == batchVertices->at(m_vertexIndex+0).x) batchVertices->at(m_vertexIndex+2).x += 1.0f;
  if (batchVertices->at(m_vertexIndex+3).y == batchVertices->at(m_vertexIndex+1).y) batchVertices->at(m_vertexIndex+3).y += 1.0f;
  if (batchVertices->at(m_vertexIndex+3).x == batchVertices->at(m_vertexIndex+1).x) batchVertices->at(m_vertexIndex+3).x += 1.0f;
  // Setup texture coordinates
  //TopLeft
  batchVertices->at(m_vertexIndex+0).u1 = texture.x1;
  batchVertices->at(m_vertexIndex+0).v1 = texture.y1;
  //TopRight
  if (orientation & 4)
  {
    batchVertices->at(m_vertexIndex+1).u1 = texture.x1;
    batchVertices->at(m_vertexIndex+1).v1 = texture.y2;
  }
  else
  {
    batchVertices->at(m_vertexIndex+1).u1 = texture.x2;
    batchVertices->at(m_vertexIndex+1).v1 = texture.y1;
  }
  //BottomRight
  batchVertices->at(m_vertexIndex+2).u1 = texture.x2;
  batchVertices->at(m_vertexIndex+2).v1 = texture.y2;
  //BottomLeft
  if (orientation & 4)
  {
    batchVertices->at(m_vertexIndex+3).u1 = texture.x2;
    batchVertices->at(m_vertexIndex+3).v1 = texture.y1;
  }
  else
  {
    batchVertices->at(m_vertexIndex+3).u1 = texture.x1;
    batchVertices->at(m_vertexIndex+3).v1 = texture.y2;
  }

  if (m_diffuse.size())
  {
    //TopLeft
    batchVertices->at(m_vertexIndex+0).u2 = diffuse.x1;
    batchVertices->at(m_vertexIndex+0).v2 = diffuse.y1;
    //TopRight
    if (m_info.orientation & 4)
    {
      batchVertices->at(m_vertexIndex+1).u2 = diffuse.x1;
      batchVertices->at(m_vertexIndex+1).v2 = diffuse.y2;
    }
    else
    {
      batchVertices->at(m_vertexIndex+1).u2 = diffuse.x2;
      batchVertices->at(m_vertexIndex+1).v2 = diffuse.y1;
    }
    //BottomRight
    batchVertices->at(m_vertexIndex+2).u2 = diffuse.x2;
    batchVertices->at(m_vertexIndex+2).v2 = diffuse.y2;
    //BottomLeft
    if (m_info.orientation & 4)
    {
      batchVertices->at(m_vertexIndex+3).u2 = diffuse.x2;
      batchVertices->at(m_vertexIndex+3).v2 = diffuse.y1;
    }
    else
    {
      batchVertices->at(m_vertexIndex+3).u2 = diffuse.x1;
      batchVertices->at(m_vertexIndex+3).v2 = diffuse.y2;
    }
  }
  m_vertexIndex+=4;
}

bool CGUITextureBase::AllocResources()
{
  if (m_info.filename.IsEmpty())
    return false;

  if (m_texture.size())
    return false; // already have our texture

  // reset our animstate
  m_frameCounter = 0;
  m_currentFrame = 0;
  m_currentLoop = 0;

  bool changed = false;
  bool useLarge = m_info.useLarge || !g_TextureManager.CanLoad(m_info.filename);
  if (useLarge)
  { // we want to use the large image loader, but we first check for bundled textures
    if (!IsAllocated())
    {
      int images = g_TextureManager.Load(m_info.filename, true);
      if (images)
      {
        m_isAllocated = NORMAL;
        m_texture = g_TextureManager.GetTexture(m_info.filename);
        changed = true;
      }
    }
    if (m_isAllocated != NORMAL)
    { // use our large image background loader
      CTextureArray texture;
      if (g_largeTextureManager.GetImage(m_info.filename, texture, !IsAllocated()))
      {
        m_isAllocated = LARGE;

        if (!texture.size()) // not ready as yet
          return false;

        m_texture = texture;

        changed = true;
      }
      else
        m_isAllocated = LARGE_FAILED;
    }
  }
  else if (!IsAllocated())
  {
    int images = g_TextureManager.Load(m_info.filename);

    // set allocated to true even if we couldn't load the image to save
    // us hitting the disk every frame
    m_isAllocated = images ? NORMAL : NORMAL_FAILED;
    if (!images)
      return false;

    m_texture = g_TextureManager.GetTexture(m_info.filename);
    changed = true;
  }
  m_frameWidth = (float)m_texture.m_width;
  m_frameHeight = (float)m_texture.m_height;

  // load the diffuse texture (if necessary)
  if (!m_info.diffuse.IsEmpty())
  {
    g_TextureManager.Load(m_info.diffuse);
    m_diffuse = g_TextureManager.GetTexture(m_info.diffuse);
  }

  CalculateSize();

  return changed;
}

bool CGUITextureBase::CalculateSize()
{
  if (m_currentFrame >= m_texture.size())
    return false;

  m_texCoordsScaleU = 1.0f / m_texture.m_texWidth;
  m_texCoordsScaleV = 1.0f / m_texture.m_texHeight;

  if (m_width == 0)
    m_width = m_frameWidth;
  if (m_height == 0)
    m_height = m_frameHeight;

  float newPosX = m_posX;
  float newPosY = m_posY;
  float newWidth = m_width;
  float newHeight = m_height;

  if (m_aspect.ratio != CAspectRatio::AR_STRETCH && m_frameWidth && m_frameHeight)
  {
    // to get the pixel ratio, we must use the SCALED output sizes
    float pixelRatio = g_graphicsContext.GetScalingPixelRatio();

    float fSourceFrameRatio = m_frameWidth / m_frameHeight;
    if (GetOrientation() & 4)
      fSourceFrameRatio = m_frameHeight / m_frameWidth;
    float fOutputFrameRatio = fSourceFrameRatio / pixelRatio;

    // maximize the width
    newHeight = m_width / fOutputFrameRatio;

    if ((m_aspect.ratio == CAspectRatio::AR_SCALE && newHeight < m_height) ||
        (m_aspect.ratio == CAspectRatio::AR_KEEP && newHeight > m_height))
    {
      newHeight = m_height;
      newWidth = newHeight * fOutputFrameRatio;
    }
    if (m_aspect.ratio == CAspectRatio::AR_CENTER)
    { // keep original size + center
      newWidth = m_frameWidth / sqrt(pixelRatio);
      newHeight = m_frameHeight * sqrt(pixelRatio);
    }

    if (m_aspect.align & ASPECT_ALIGN_LEFT)
      newPosX = m_posX;
    else if (m_aspect.align & ASPECT_ALIGN_RIGHT)
      newPosX = m_posX + m_width - newWidth;
    else
      newPosX = m_posX + (m_width - newWidth) * 0.5f;
    if (m_aspect.align & ASPECT_ALIGNY_TOP)
      newPosY = m_posY;
    else if (m_aspect.align & ASPECT_ALIGNY_BOTTOM)
      newPosY = m_posY + m_height - newHeight;
    else
      newPosY = m_posY + (m_height - newHeight) * 0.5f;
  }
  
  m_vertex.SetRect(newPosX, newPosY, newPosX + newWidth, newPosY + newHeight);

  // scale the diffuse coords as well
  if (m_diffuse.size())
  { // calculate scaling for the texcoords
    if (m_diffuse.m_texCoordsArePixels)
    {
      m_diffuseU = float(m_diffuse.m_width);
      m_diffuseV = float(m_diffuse.m_height);
    }
    else
    {
      m_diffuseU = float(m_diffuse.m_width) / float(m_diffuse.m_texWidth);
      m_diffuseV = float(m_diffuse.m_height) / float(m_diffuse.m_texHeight);
    }

    if (m_aspect.scaleDiffuse)
    {
      m_diffuseScaleU = m_diffuseU;
      m_diffuseScaleV = m_diffuseV;
      m_diffuseOffset = CPoint(0,0);
    }
    else // stretch'ing diffuse
    { // scale diffuse up or down to match output rect size, rather than image size
      //(m_fX, mfY) -> (m_fX + m_fNW, m_fY + m_fNH)
      //(0,0) -> (m_fU*m_diffuseScaleU, m_fV*m_diffuseScaleV)
      // x = u/(m_fU*m_diffuseScaleU)*m_fNW + m_fX
      // -> u = (m_posX - m_fX) * m_fU * m_diffuseScaleU / m_fNW
      m_diffuseScaleU = m_diffuseU * m_vertex.Width() / m_width;
      m_diffuseScaleV = m_diffuseV * m_vertex.Height() / m_height;
      m_diffuseOffset = CPoint((m_vertex.x1 - m_posX) / m_vertex.Width() * m_diffuseScaleU, (m_vertex.y1 - m_posY) / m_vertex.Height() * m_diffuseScaleV);
    }
  }

  m_invalid = false;
  return true;
}

void CGUITextureBase::FreeResources(bool immediately /* = false */)
{
  if (m_isAllocated == LARGE || m_isAllocated == LARGE_FAILED)
    g_largeTextureManager.ReleaseImage(m_info.filename, immediately || (m_isAllocated == LARGE_FAILED));
  else if (m_isAllocated == NORMAL && m_texture.size())
    g_TextureManager.ReleaseTexture(m_info.filename);

  if (m_diffuse.size())
    g_TextureManager.ReleaseTexture(m_info.diffuse);
  m_diffuse.Reset();

  m_texture.Reset();

  m_currentFrame = 0;
  m_currentLoop = 0;
  m_texCoordsScaleU = 1.0f;
  m_texCoordsScaleV = 1.0f;

  m_isAllocated = NO;
}

void CGUITextureBase::DynamicResourceAlloc(bool allocateDynamically)
{
  m_allocateDynamically = allocateDynamically;
}

void CGUITextureBase::SetInvalid()
{
  m_invalid = true;
}

bool CGUITextureBase::UpdateAnimFrame()
{
  bool changed = false;

  m_frameCounter++;
  unsigned int delay = m_texture.m_delays[m_currentFrame];
  if (!delay) delay = 100;
  if (m_frameCounter * 40 >= delay)
  {
    m_frameCounter = 0;
    if (m_currentFrame + 1 >= m_texture.size())
    {
      if (m_texture.m_loops > 0)
      {
        if (m_currentLoop + 1 < m_texture.m_loops)
        {
          m_currentLoop++;
          m_currentFrame = 0;
          changed = true;
        }
      }
      else
      {
        // 0 == loop forever
        m_currentFrame = 0;
        changed = true;
      }
    }
    else
    {
      m_currentFrame++;
      changed = true;
    }
  }

  return changed;
}

bool CGUITextureBase::SetVisible(bool visible)
{
  bool changed = m_visible != visible;
  m_visible = visible;
  return changed;
}

bool CGUITextureBase::SetAlpha(unsigned char alpha)
{
  bool changed = m_alpha != alpha;
  m_alpha = alpha;
  return changed;
}

bool CGUITextureBase::SetDiffuseColor(color_t color)
{
  bool changed = m_diffuseColor != color;
  m_diffuseColor = color;
  return changed;
}

bool CGUITextureBase::ReadyToRender() const
{
  return m_texture.size() > 0;
}

void CGUITextureBase::OrientateTexture(CRect &rect, float width, float height, int orientation)
{
  switch (orientation & 3)
  {
  case 0:
    // default
    break;
  case 1:
    // flip in X direction
    rect.x1 = width - rect.x1;
    rect.x2 = width - rect.x2;
    break;
  case 2:
    // rotate 180 degrees
    rect.x1 = width - rect.x1;
    rect.x2 = width - rect.x2;
    rect.y1 = height - rect.y1;
    rect.y2 = height - rect.y2;
    break;
  case 3:
    // flip in Y direction
    rect.y1 = height - rect.y1;
    rect.y2 = height - rect.y2;
    break;
  }
  if (orientation & 4)
  {
    // we need to swap x and y coordinates but only within the width,height block
    float temp = rect.x1;
    rect.x1 = rect.y1 * width/height;
    rect.y1 = temp * height/width;
    temp = rect.x2;
    rect.x2 = rect.y2 * width/height;
    rect.y2 = temp * height/width;
  }
}

bool CGUITextureBase::SetWidth(float width)
{
  if (width < m_info.border.x1 + m_info.border.x2)
    width = m_info.border.x1 + m_info.border.x2;
  if (m_width != width)
  {
    m_width = width;
    m_invalid = true;
    return true;
  }
  else
    return false;
}

bool CGUITextureBase::SetHeight(float height)
{
  if (height < m_info.border.y1 + m_info.border.y2)
    height = m_info.border.y1 + m_info.border.y2;
  if (m_height != height)
  {
    m_height = height;
    m_invalid = true;
    return true;
  }
  else
    return false;
}

bool CGUITextureBase::SetPosition(float posX, float posY)
{
  if (m_posX != posX || m_posY != posY)
  {
    m_posX = posX;
    m_posY = posY;
    m_invalid = true;
    return true;
  }
  else
    return false;
}

bool CGUITextureBase::SetAspectRatio(const CAspectRatio &aspect)
{
  if (m_aspect != aspect)
  {
    m_aspect = aspect;
    m_invalid = true;
    return true;
  }
  else
    return false;
}

bool CGUITextureBase::SetFileName(const CStdString& filename)
{
  if (m_info.filename.Equals(filename)) return false;
  // Don't completely free resources here - we may be just changing
  // filenames mid-animation
  FreeResources();
  m_info.filename = filename;
  // Don't allocate resources here as this is done at render time
  return true;
}

int CGUITextureBase::GetOrientation() const
{
  // multiply our orientations
  static char orient_table[] = { 0, 1, 2, 3, 4, 5, 6, 7,
                                 1, 0, 3, 2, 5, 4, 7, 6,
                                 2, 3, 0, 1, 6, 7, 4, 5,
                                 3, 2, 1, 0, 7, 6, 5, 4,
                                 4, 7, 6, 5, 0, 3, 2, 1,
                                 5, 6, 7, 4, 1, 2, 3, 0,
                                 6, 5, 4, 7, 2, 1, 0, 3,
                                 7, 4, 5, 6, 3, 0, 1, 2 };
  return (int)orient_table[8 * m_info.orientation + m_texture.m_orientation];
}
