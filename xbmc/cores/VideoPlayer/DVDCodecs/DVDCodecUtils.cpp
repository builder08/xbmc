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

#include "DVDCodecUtils.h"
#include "TimingConstants.h"
#include "utils/log.h"
#include "cores/FFmpeg.h"
#include "cores/VideoPlayer/Process/VideoBuffer.h"
#include "Util.h"

extern "C" {
#include "libswscale/swscale.h"
}

bool CDVDCodecUtils::CopyPicture(YuvImage* pImage, VideoPicture *pSrc)
{
  uint8_t *s = pSrc->data[0];
  uint8_t *d = pImage->plane[0];
  int w = pImage->width * pImage->bpp;
  int h = pImage->height;
  if ((w == pSrc->iLineSize[0]) && ((unsigned int) pSrc->iLineSize[0] == pImage->stride[0]))
  {
    memcpy(d, s, w*h);
  }
  else
  {
    for (int y = 0; y < h; y++)
    {
      memcpy(d, s, w);
      s += pSrc->iLineSize[0];
      d += pImage->stride[0];
    }
  }
  s = pSrc->data[1];
  d = pImage->plane[1];
  w =(pImage->width  >> pImage->cshift_x) * pImage->bpp;
  h =(pImage->height >> pImage->cshift_y);
  if ((w==pSrc->iLineSize[1]) && ((unsigned int) pSrc->iLineSize[1]==pImage->stride[1]))
  {
    memcpy(d, s, w*h);
  }
  else
  {
    for (int y = 0; y < h; y++)
    {
      memcpy(d, s, w);
      s += pSrc->iLineSize[1];
      d += pImage->stride[1];
    }
  }
  s = pSrc->data[2];
  d = pImage->plane[2];
  if ((w==pSrc->iLineSize[2]) && ((unsigned int) pSrc->iLineSize[2]==pImage->stride[2]))
  {
    memcpy(d, s, w*h);
  }
  else
  {
    for (int y = 0; y < h; y++)
    {
      memcpy(d, s, w);
      s += pSrc->iLineSize[2];
      d += pImage->stride[2];
    }
  }
  return true;
}


bool CDVDCodecUtils::CopyNV12Picture(YuvImage* pImage, VideoPicture *pSrc)
{
  uint8_t *s = pSrc->data[0];
  uint8_t *d = pImage->plane[0];
  int w = pSrc->iWidth;
  int h = pSrc->iHeight;
  // Copy Y
  if ((w == pSrc->iLineSize[0]) && ((unsigned int) pSrc->iLineSize[0] == pImage->stride[0]))
  {
    memcpy(d, s, w*h);
  }
  else
  {
    for (int y = 0; y < h; y++)
    {
      memcpy(d, s, w);
      s += pSrc->iLineSize[0];
      d += pImage->stride[0];
    }
  }
  
  s = pSrc->data[1];
  d = pImage->plane[1];
  w = pSrc->iWidth;
  h = pSrc->iHeight >> 1;
  // Copy packed UV (width is same as for Y as it's both U and V components)
  if ((w==pSrc->iLineSize[1]) && ((unsigned int) pSrc->iLineSize[1]==pImage->stride[1]))
  {
    memcpy(d, s, w*h);
  }
  else
  {
    for (int y = 0; y < h; y++)
    {
      memcpy(d, s, w);
      s += pSrc->iLineSize[1];
      d += pImage->stride[1];
    }
  }

  return true;
}

bool CDVDCodecUtils::CopyYUV422PackedPicture(YuvImage* pImage, VideoPicture *pSrc)
{
  uint8_t *s = pSrc->data[0];
  uint8_t *d = pImage->plane[0];
  int w = pSrc->iWidth;
  int h = pSrc->iHeight;

  // Copy YUYV
  if ((w * 2 == pSrc->iLineSize[0]) && ((unsigned int) pSrc->iLineSize[0] == pImage->stride[0]))
  {
    memcpy(d, s, w*h*2);
  }
  else
  {
    for (int y = 0; y < h; y++)
    {
      memcpy(d, s, w*2);
      s += pSrc->iLineSize[0];
      d += pImage->stride[0];
    }
  }
  
  return true;
}

bool CDVDCodecUtils::IsVP3CompatibleWidth(int width)
{
  // known hardware limitation of purevideo 3 (VP3). (the Nvidia 9400 is a purevideo 3 chip)
  // from nvidia's linux vdpau README: All current third generation PureVideo hardware
  // (G98, MCP77, MCP78, MCP79, MCP7A) cannot decode H.264 for the following horizontal resolutions:
  // 769-784, 849-864, 929-944, 1009–1024, 1793–1808, 1873–1888, 1953–1968 and 2033-2048 pixel.
  // This relates to the following macroblock sizes.
  int unsupported[] = {49, 54, 59, 64, 113, 118, 123, 128};
  for (unsigned int i = 0; i < sizeof(unsupported) / sizeof(int); i++)
  {
    if (unsupported[i] == (width + 15) / 16)
      return false;
  }
  return true;
}

double CDVDCodecUtils::NormalizeFrameduration(double frameduration, bool *match)
{
  //if the duration is within 20 microseconds of a common duration, use that
  const double durations[] = {DVD_TIME_BASE * 1.001 / 24.0, DVD_TIME_BASE / 24.0, DVD_TIME_BASE / 25.0,
                              DVD_TIME_BASE * 1.001 / 30.0, DVD_TIME_BASE / 30.0, DVD_TIME_BASE / 50.0,
                              DVD_TIME_BASE * 1.001 / 60.0, DVD_TIME_BASE / 60.0};

  double lowestdiff = DVD_TIME_BASE;
  int    selected   = -1;
  for (size_t i = 0; i < ARRAY_SIZE(durations); i++)
  {
    double diff = fabs(frameduration - durations[i]);
    if (diff < DVD_MSEC_TO_TIME(0.02) && diff < lowestdiff)
    {
      selected = i;
      lowestdiff = diff;
    }
  }

  if (selected != -1)
  {
    if (match)
      *match = true;
    return durations[selected];
  }
  else
  {
    if (match)
      *match = false;
    return frameduration;
  }
}

struct EFormatMap {
  AVPixelFormat   pix_fmt;
  ERenderFormat format;
};

static const EFormatMap g_format_map[] = {
   { AV_PIX_FMT_YUV420P,     RENDER_FMT_YUV420P    }
,  { AV_PIX_FMT_YUVJ420P,    RENDER_FMT_YUV420P    }
,  { AV_PIX_FMT_YUV420P10,   RENDER_FMT_YUV420P10  }
,  { AV_PIX_FMT_YUV420P16,   RENDER_FMT_YUV420P16  }
,  { AV_PIX_FMT_UYVY422,     RENDER_FMT_UYVY422    }
,  { AV_PIX_FMT_YUYV422,     RENDER_FMT_YUYV422    }
,  { AV_PIX_FMT_VAAPI_VLD,   RENDER_FMT_VAAPI      }
,  { AV_PIX_FMT_D3D11VA_VLD, RENDER_FMT_DXVA       }
,  { AV_PIX_FMT_NONE     ,   RENDER_FMT_NONE       }
};

ERenderFormat CDVDCodecUtils::EFormatFromPixfmt(int fmt)
{
  for(const EFormatMap *p = g_format_map; p->pix_fmt != AV_PIX_FMT_NONE; ++p)
  {
    if(p->pix_fmt == fmt)
      return p->format;
  }
  return RENDER_FMT_NONE;
}

AVPixelFormat CDVDCodecUtils::PixfmtFromEFormat(ERenderFormat fmt)
{
  for(const EFormatMap *p = g_format_map; p->pix_fmt != AV_PIX_FMT_NONE; ++p)
  {
    if(p->format == fmt)
      return p->pix_fmt;
  }
  return AV_PIX_FMT_NONE;
}
