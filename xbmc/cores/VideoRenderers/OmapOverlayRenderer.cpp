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
#include "../dvdplayer/Codecs/DllSwScale.h"
#include "log.h"

COmapOverlayRenderer::COmapOverlayRenderer()
{
  m_dllSwScale = new DllSwScale;

  m_yuvBuffers[0].plane[0] = NULL;
  m_yuvBuffers[0].plane[1] = NULL;
  m_yuvBuffers[0].plane[2] = NULL;

  m_yuvBuffers[1].plane[0] = NULL;
  m_yuvBuffers[1].plane[1] = NULL;
  m_yuvBuffers[1].plane[2] = NULL;

  m_overlayfd = -1;

  m_framebuffers[0].buf = NULL;
  m_framebuffers[1].buf = NULL;

  UnInit();
}

COmapOverlayRenderer::~COmapOverlayRenderer()
{
  UnInit();

  delete m_dllSwScale;
}

bool COmapOverlayRenderer::Configure(unsigned int width, unsigned int height, unsigned int d_width, unsigned int d_height, float fps, unsigned int flags)
{
  printf("Configure with [%i, %i] and [%i, %i] and fps %f and flags %i\n", width, height, d_width, d_height, fps, flags);

  if (CONF_FLAGS_FORMAT_MASK(flags) == CONF_FLAGS_FORMAT_NV12)
  {
    printf("Bad format\n");
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
    m_overlayScreenInfo.xres = m_sourceWidth;
    m_overlayScreenInfo.yres = m_sourceHeight;
    m_overlayScreenInfo.xoffset = 0;
    m_overlayScreenInfo.yoffset = 0;
    m_overlayScreenInfo.nonstd = OMAPFB_COLOR_YUY422;

    unsigned int frameSize = m_overlayScreenInfo.xres * m_overlayScreenInfo.yres * 2;
    unsigned int memSize = 0;//m_overlayScreenInfo.size;

    if (!memSize)
    {
      struct omapfb_mem_info mi = m_overlayMemInfo;
      mi.size = frameSize * 2;
      if (ioctl(m_overlayfd, OMAPFB_SETUP_MEM, &mi))
      {
        CLog::Log(LOGERROR, "OmapOverlay: Failed to setup memory");
        return false;
      }
      memSize = mi.size;
    }

    uint8_t *fbmem = (uint8_t *)mmap(NULL, memSize, PROT_READ|PROT_WRITE, MAP_SHARED, m_overlayfd, 0);
    if (fbmem == MAP_FAILED)
    {
      CLog::Log(LOGERROR, "OmapOverlay: Failed to map the framebuffer");
      return false;
    }

    for (unsigned int i = 0; i < memSize / 4; i++)
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

    m_overlayPlaneInfo.pos_x      = 0;
    m_overlayPlaneInfo.pos_y      = 0;
    m_overlayPlaneInfo.out_width  = width;
    m_overlayPlaneInfo.out_height = height;

    if (ioctl(m_overlayfd, OMAPFB_SETUP_PLANE, &m_overlayPlaneInfo) == -1)
    {
      CLog::Log(LOGERROR, "OmapOverlay: Failed to set plane info");
      return false;
    }

    for (unsigned int i = 0; i < 2; i++)
    {
      FreeYV12Image(i);
      CreateYV12Image(i, m_sourceWidth, m_sourceHeight);
    }

    m_currentBuffer = 0;

    printf("Proper format, continuing\n");
  }

  m_iFlags = flags;
  m_bConfigured = true;

  return m_bConfigured;
}

int COmapOverlayRenderer::GetImage(YV12Image *image, int source, bool readonly)
{
  if (!m_bConfigured)
    return -1;

  /* take next available buffer */
  if( source == AUTOSOURCE )
    source = NextYV12Image();

  printf("image [%i, %i]\n", image->width, image->height);

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

  struct SwsContext *context = m_dllSwScale->sws_getContext(image->width, image->height, PIX_FMT_YUV420P,
                                                            image->width, image->height, PIX_FMT_YUV422P,
                                                            SWS_FAST_BILINEAR, NULL, NULL, NULL);

  if (context)
  {
    uint8_t *src[]  = { image->plane[0],  image->plane[1],  image->plane[2],  0 };
    int srcStride[] = { image->stride[0], image->stride[1], image->stride[2], 0 };

    // TODO here we could probably convert straight into the framebuffer if its memory mapped?
    uint8_t *dst[]  = { m_framebuffers[source].buf, 0, 0, 0 };
    int dstStride[] = { image->width * 4, 0, 0, 0 };

    m_dllSwScale->sws_scale(context, src, srcStride, 0, image->height, dst, dstStride);
    m_dllSwScale->sws_freeContext(context);
  }
}

void COmapOverlayRenderer::FlipPage(int source)
{
  if (!m_bConfigured)
    return;

  m_overlayScreenInfo.xoffset = m_framebuffers[m_currentBuffer].x;
  m_overlayScreenInfo.yoffset = m_framebuffers[m_currentBuffer].y;
  ioctl(m_overlayfd, FBIOPAN_DISPLAY, &m_overlayScreenInfo);
  ioctl(m_overlayfd, OMAPFB_WAITFORGO);

  m_currentBuffer = NextYV12Image();
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
}

unsigned int COmapOverlayRenderer::DrawSlice(unsigned char *src[], int stride[], int w, int h, int x, int y)
{
  return 0;
}

unsigned int COmapOverlayRenderer::PreInit()
{
  UnInit();

  printf("PreInit\n");

  if (!m_dllSwScale->Load())
  {
    CLog::Log(LOGERROR, "COmapOverlayRenderer::PreInit - failed to load rescale libraries!");
    return false;
  }

  return true;
}

void COmapOverlayRenderer::UnInit()
{
  CLog::Log(LOGINFO, "COmapOverlayRenderer::UnInit");
  printf("UnInit\n");
  m_bConfigured = false;
  m_iFlags = 0;
  m_currentBuffer = 0;

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
  return 1 - m_currentBuffer;
}

bool COmapOverlayRenderer::CreateYV12Image(unsigned int index, unsigned int width, unsigned int height)
{
  YV12Image &im = m_yuvBuffers[index];

  im.width  = width;
  im.height = height;
  im.cshift_x = 1;
  im.cshift_y = 1;

  im.stride[0] = im.width;
  im.stride[1] = im.width >> im.cshift_x;
  im.stride[2] = im.width >> im.cshift_x;

  im.planesize[0] = im.stride[0] * im.height;
  im.planesize[1] = im.stride[1] * ( im.height >> im.cshift_y );
  im.planesize[2] = im.stride[2] * ( im.height >> im.cshift_y );

  for (int i = 0; i < 3; i++)
    im.plane[i] = new BYTE[im.planesize[i]];

  return true;
}

bool COmapOverlayRenderer::FreeYV12Image(unsigned int index)
{
  YV12Image &im = m_yuvBuffers[index];
  for (int i = 0; i < 3; i++)
  {
    delete[] im.plane[i];
    im.plane[i] = NULL;
  }

  memset(&im , 0, sizeof(YV12Image));

  return true;
}

#endif
