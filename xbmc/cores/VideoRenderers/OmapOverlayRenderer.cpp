/*
 *      Copyright (C) 2005-2010 Team XBMC
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
 *  along with XBMC; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#include "system.h"
#if (defined HAVE_CONFIG_H) && (!defined WIN32)
  #include "config.h"
#endif

#ifdef HAS_OMAP_OVERLAY
#include "OmapOverlayRenderer.h"
#include "log.h"
#include <stdlib.h>
#include <malloc.h>

extern "C" void yuv420_to_yuv422(uint8_t *yuv, uint8_t *y, uint8_t *u, uint8_t *v, int w, int h, int yw, int cw, int dw);

COmapOverlayRenderer::COmapOverlayRenderer()
{
  m_bConfigured = false;

  m_yuvBuffers[0].plane[0] = NULL;
  m_yuvBuffers[0].plane[1] = NULL;
  m_yuvBuffers[0].plane[2] = NULL;

  m_yuvBuffers[1].plane[0] = NULL;
  m_yuvBuffers[1].plane[1] = NULL;
  m_yuvBuffers[1].plane[2] = NULL;

  m_overlayfd = -1;

  m_framebuffers[0].buf = NULL;
  m_framebuffers[1].buf = NULL;

  m_currentBackBuffer = 0;
  m_currentDisplayBuffer = 1;
}

COmapOverlayRenderer::~COmapOverlayRenderer()
{
  UnInit();
}

bool COmapOverlayRenderer::Configure(unsigned int width, unsigned int height, unsigned int d_width, unsigned int d_height, float fps, unsigned int flags)
{
  CLog::Log(LOGINFO, "OmapOverlay: Configure with [%i, %i] and [%i, %i] and fps %f and flags %i\n", width, height, d_width, d_height, fps, flags);

  if (CONF_FLAGS_FORMAT_MASK(flags) == CONF_FLAGS_FORMAT_NV12)
  {
    CLog::Log(LOGERROR, "OmapOverlay: Does not support NV12 format");
    return false;
  }
  else if (width != m_sourceWidth || height != m_sourceHeight)
  {
    m_sourceWidth = width;
    m_sourceHeight = height;

    // Open the framebuffer
    m_overlayfd = open("/dev/fb1", O_RDWR);
    if (m_overlayfd == -1)
    {
      CLog::Log(LOGERROR, "OmapOverlay: Failed to open framebuffer");
      return false;
    }

    if (ioctl(m_overlayfd, FBIOGET_VSCREENINFO, &m_overlayScreenInfo) == -1 ||
        ioctl(m_overlayfd, OMAPFB_QUERY_PLANE,  &m_overlayPlaneInfo)  == -1 ||
        ioctl(m_overlayfd, OMAPFB_QUERY_MEM,    &m_overlayMemInfo)    == -1)
    {
      CLog::Log(LOGERROR, "OmapOverlay: Failed to probe for screen info, plane info or memory info");
      return false;
    }

    // Enable the framebuffer
    m_overlayScreenInfo.xres = m_sourceWidth  & ~15;
    m_overlayScreenInfo.yres = m_sourceHeight & ~15;
    m_overlayScreenInfo.xoffset = 0;
    m_overlayScreenInfo.yoffset = 0;
    m_overlayScreenInfo.nonstd = OMAPFB_COLOR_YUY422;

    unsigned int frameSize = m_overlayScreenInfo.xres * m_overlayScreenInfo.yres * 2;
    unsigned int wantedMemSize = frameSize * 2;
    if (m_overlayMemInfo.size != wantedMemSize)
    {
      m_overlayMemInfo.size = wantedMemSize;
      if (ioctl(m_overlayfd, OMAPFB_SETUP_MEM, &m_overlayMemInfo))
      {
        CLog::Log(LOGERROR, "OmapOverlay: Failed to setup memory");
        return false;
      }
    }

    uint8_t *fbmem = (uint8_t *)mmap(NULL, wantedMemSize, PROT_READ|PROT_WRITE, MAP_SHARED, m_overlayfd, 0);
    if (fbmem == MAP_FAILED)
    {
      CLog::Log(LOGERROR, "OmapOverlay: Failed to map the framebuffer");
      return false;
    }

    for (unsigned int i = 0; i < wantedMemSize / 4; i++)
      ((uint32_t*)fbmem)[i] = 0x80008000;

    m_overlayScreenInfo.xres_virtual = m_overlayScreenInfo.xres;
    m_overlayScreenInfo.yres_virtual = m_overlayScreenInfo.yres * 2;

    m_framebuffers[0].x = 0;
    m_framebuffers[0].y = 0;
    m_framebuffers[0].buf = fbmem;

    m_framebuffers[1].x = 0;
    m_framebuffers[1].y = m_overlayScreenInfo.yres;
    m_framebuffers[1].buf = fbmem + frameSize;

    if (ioctl(m_overlayfd, FBIOPUT_VSCREENINFO, &m_overlayScreenInfo) == -1)
    {
      CLog::Log(LOGERROR, "OmapOverlay: Failed to set screen info");
      return false;
    }

    m_overlayPlaneInfo.enabled = 1;

    m_drawRegion.SetRect(0, 0, m_overlayScreenInfo.xres,  m_overlayScreenInfo.yres);

    m_overlayPlaneInfo.pos_x      = 0;
    m_overlayPlaneInfo.pos_y      = 0;
    m_overlayPlaneInfo.out_width  = m_overlayScreenInfo.xres;
    m_overlayPlaneInfo.out_height = m_overlayScreenInfo.yres;

    if (ioctl(m_overlayfd, OMAPFB_SETUP_PLANE, &m_overlayPlaneInfo) == -1)
    {
      CLog::Log(LOGERROR, "OmapOverlay: Failed to set plane info");
      return false;
    }

    m_bConfigured = true;

    for (unsigned int i = 0; i < 2; i++)
    {
      FreeYV12Image(i);
      m_bConfigured &= CreateYV12Image(i, m_overlayScreenInfo.xres, m_overlayScreenInfo.yres);
    }

    m_currentBackBuffer = 0;
  }

  m_iFlags = flags;

  return m_bConfigured;
}

int COmapOverlayRenderer::GetImage(YV12Image *image, int source, bool readonly)
{
  if (!m_bConfigured)
    return -1;

  /* take next available buffer */
  if( source == AUTOSOURCE || source > 1 || source < 0)
    source = m_currentBackBuffer;

  YV12Image &im = m_yuvBuffers[source];

  for (int p=0;p<MAX_PLANES;p++)
  {
    image->plane[p]  = im.plane[p];
    image->stride[p] = im.stride[p];
  }

  image->width    = im.width;
  image->height   = im.height;
  image->flags    = im.flags;
  image->cshift_x = im.cshift_x;
  image->cshift_y = im.cshift_y;

  return source;
}

void COmapOverlayRenderer::ReleaseImage(int source, bool preserve)
{
  if (!m_bConfigured)
    return;

  YV12Image *image = &m_yuvBuffers[source];

  yuv420_to_yuv422(m_framebuffers[source].buf, image->plane[0], image->plane[1], image->plane[2], image->width, image->height, image->stride[0], image->stride[1], m_overlayScreenInfo.xres * 2);
}

void COmapOverlayRenderer::FlipPage(int source)
{
  if (!m_bConfigured)
    return;

  m_currentDisplayBuffer = m_currentBackBuffer;
  m_currentBackBuffer = NextYV12Image();
}

void COmapOverlayRenderer::Reset()
{
}

void COmapOverlayRenderer::Update(bool bPauseDrawing)
{
}

void COmapOverlayRenderer::AddProcessor(void *processor)
{
}

void COmapOverlayRenderer::RenderUpdate(bool clear, DWORD flags, DWORD alpha)
{
  if (!m_bConfigured)
    return;

  static unsigned int currentDisplayedBuffered = -1;

  if (m_currentDisplayBuffer == currentDisplayedBuffered)
    return;

  YV12Image *image = &m_yuvBuffers[m_currentDisplayBuffer];
  CRect drawRegion(0, 0, image->width, image->height);
  drawRegion = g_graphicsContext.generateAABB(drawRegion);

  if (m_drawRegion != drawRegion)
  {
    m_drawRegion = drawRegion;

    m_overlayPlaneInfo.pos_x      = m_drawRegion.x1;
    m_overlayPlaneInfo.pos_y      = m_drawRegion.y1;
    m_overlayPlaneInfo.out_width  = m_drawRegion.Width();
    m_overlayPlaneInfo.out_height = m_drawRegion.Height();

    if (ioctl(m_overlayfd, OMAPFB_SETUP_PLANE, &m_overlayPlaneInfo) == -1)
      CLog::Log(LOGERROR, "OmapOverlay: Failed to set plane info");
  }

  m_overlayScreenInfo.xoffset = m_framebuffers[m_currentDisplayBuffer].x;
  m_overlayScreenInfo.yoffset = m_framebuffers[m_currentDisplayBuffer].y;

  ioctl(m_overlayfd, FBIOPAN_DISPLAY, &m_overlayScreenInfo);
  ioctl(m_overlayfd, OMAPFB_WAITFORGO);

  currentDisplayedBuffered = m_currentDisplayBuffer;
}

unsigned int COmapOverlayRenderer::DrawSlice(unsigned char *src[], int stride[], int w, int h, int x, int y)
{
  yuv420_to_yuv422(m_framebuffers[m_currentBackBuffer].buf, src[0], src[1], src[2], w, h, stride[0], stride[1], m_overlayScreenInfo.xres * 2);

  return 0;
}

unsigned int COmapOverlayRenderer::PreInit()
{
  UnInit();

  return true;
}

void COmapOverlayRenderer::UnInit()
{
  if (!m_bConfigured)
    return;

  CLog::Log(LOGINFO, "OmapOverlay: UnInit");
  m_bConfigured = false;
  m_iFlags = 0;
  m_currentBackBuffer = 0;

  for (unsigned int i = 0; i < 2; i++)
    FreeYV12Image(i);

  m_overlayPlaneInfo.enabled = 0;
  ioctl(m_overlayfd, OMAPFB_SETUP_PLANE, &m_overlayPlaneInfo);
  ioctl(m_overlayfd, OMAPFB_SETUP_MEM,   &m_overlayMemInfo);

  if (m_overlayfd > 0)
  {
    close(m_overlayfd);
    m_overlayfd = -1;
  }
}

void COmapOverlayRenderer::CreateThumbnail(CBaseTexture* texture, unsigned int width, unsigned int height)
{
}

bool COmapOverlayRenderer::Supports(ERENDERFEATURE feature)
{
  return false;
}

bool COmapOverlayRenderer::SupportsMultiPassRendering()
{
  return false;
}

bool COmapOverlayRenderer::Supports(EINTERLACEMETHOD method)
{
  return false;
}

bool COmapOverlayRenderer::Supports(ESCALINGMETHOD method)
{
  if(method == VS_SCALINGMETHOD_NEAREST
  || method == VS_SCALINGMETHOD_LINEAR)
    return true;

  return false;
}

unsigned int COmapOverlayRenderer::NextYV12Image()
{
  return 1 - m_currentBackBuffer;
}

bool COmapOverlayRenderer::CreateYV12Image(unsigned int index, unsigned int width, unsigned int height)
{
  YV12Image &im = m_yuvBuffers[index];

  im.width  = width;
  im.height = height;
  im.cshift_x = 1;
  im.cshift_y = 1;

  im.stride[0] = width;
  im.stride[1] = width >> im.cshift_x;
  im.stride[2] = width >> im.cshift_x;

  im.planesize[0] = im.stride[0] * im.height;
  im.planesize[1] = im.stride[1] * ( im.height >> im.cshift_y );
  im.planesize[2] = im.stride[2] * ( im.height >> im.cshift_y );

  for (int i = 0; i < 3; i++)
    im.plane[i] = (BYTE *)memalign(16, im.planesize[i]);

  return im.plane[0] != NULL && im.plane[1] != NULL && im.plane[2] != NULL;
}

bool COmapOverlayRenderer::FreeYV12Image(unsigned int index)
{
  YV12Image &im = m_yuvBuffers[index];
  for (int i = 0; i < 3; i++)
    free(im.plane[i]);

  memset(&im , 0, sizeof(YV12Image));
  im.plane[0] = im.plane[1] = im.plane[2] = NULL;

  return true;
}

#endif
