#pragma once
/*
 *      Copyright (C) 2013 Team XBMC
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

#include <queue>
#include <vector>
#include <boost/shared_ptr.hpp>

#ifndef VIDEO_DVDVIDEOCODEC_H_INCLUDED
#define VIDEO_DVDVIDEOCODEC_H_INCLUDED
#include "DVDVideoCodec.h"
#endif

#ifndef VIDEO_DVDSTREAMINFO_H_INCLUDED
#define VIDEO_DVDSTREAMINFO_H_INCLUDED
#include "DVDStreamInfo.h"
#endif

#ifndef VIDEO_THREADS_THREAD_H_INCLUDED
#define VIDEO_THREADS_THREAD_H_INCLUDED
#include "threads/Thread.h"
#endif

#ifndef VIDEO_THREADS_SINGLELOCK_H_INCLUDED
#define VIDEO_THREADS_SINGLELOCK_H_INCLUDED
#include "threads/SingleLock.h"
#endif


class CJNISurface;
class CJNISurfaceTexture;
class CJNIMediaCodec;
class CJNIMediaFormat;
class CDVDMediaCodecOnFrameAvailable;
class CJNIByteBuffer;
class CBitstreamConverter;

typedef struct amc_demux {
  uint8_t  *pData;
  int       iSize;
  double    dts;
  double    pts;
} amc_demux;

class CDVDMediaCodecInfo
{
public:
  CDVDMediaCodecInfo( int index,
                      unsigned int texture,
                      boost::shared_ptr<CJNIMediaCodec> &codec,
                      boost::shared_ptr<CJNISurfaceTexture> &surfacetexture,
                      boost::shared_ptr<CDVDMediaCodecOnFrameAvailable> &frameready);

  // reference counting
  CDVDMediaCodecInfo* Retain();
  long                Release();

  // meat and potatos
  void                Validate(bool state);
  // MediaCodec related
  void                ReleaseOutputBuffer(bool render);
  // SurfaceTexture released
  int                 GetIndex() const;
  int                 GetTextureID() const;
  void                GetTransformMatrix(float *textureMatrix);
  void                UpdateTexImage();

private:
  // private because we are reference counted
  virtual            ~CDVDMediaCodecInfo();

  long                m_refs;
  bool                m_valid;
  bool                m_isReleased;
  int                 m_index;
  unsigned int        m_texture;
  int64_t             m_timestamp;
  CCriticalSection    m_section;
  // shared_ptr bits, shared between
  // CDVDVideoCodecAndroidMediaCodec and LinuxRenderGLES.
  boost::shared_ptr<CJNIMediaCodec> m_codec;
  boost::shared_ptr<CJNISurfaceTexture> m_surfacetexture;
  boost::shared_ptr<CDVDMediaCodecOnFrameAvailable> m_frameready;
};

class CDVDVideoCodecAndroidMediaCodec : public CDVDVideoCodec
{
public:
  CDVDVideoCodecAndroidMediaCodec();
  virtual ~CDVDVideoCodecAndroidMediaCodec();

  // required overrides
  virtual bool    Open(CDVDStreamInfo &hints, CDVDCodecOptions &options);
  virtual void    Dispose();
  virtual int     Decode(uint8_t *pData, int iSize, double dts, double pts);
  virtual void    Reset();
  virtual bool    GetPicture(DVDVideoPicture *pDvdVideoPicture);
  virtual bool    ClearPicture(DVDVideoPicture* pDvdVideoPicture);
  virtual void    SetDropState(bool bDrop);
  virtual int     GetDataSize(void);
  virtual double  GetTimeSize(void);
  virtual const char* GetName(void) { return m_formatname; }
  virtual unsigned GetAllowedReferences();

protected:
  void            FlushInternal(void);
  bool            ConfigureMediaCodec(void);
  int             GetOutputPicture(void);
  void            ConfigureOutputFormat(CJNIMediaFormat* mediaformat);

  // surface handling functions
  static void     CallbackInitSurfaceTexture(void*);
  void            InitSurfaceTexture(void);
  void            ReleaseSurfaceTexture(void);

  CDVDStreamInfo  m_hints;
  std::string     m_mime;
  std::string     m_codecname;
  int             m_colorFormat;
  const char     *m_formatname;
  bool            m_opened;
  bool            m_drop;

  CJNISurface    *m_surface;
  unsigned int    m_textureId;
  // we need these as shared_ptr because CDVDVideoCodecAndroidMediaCodec
  // will get deleted before CLinuxRendererGLES is shut down and
  // CLinuxRendererGLES refs them via CDVDMediaCodecInfo.
  boost::shared_ptr<CJNIMediaCodec> m_codec;
  boost::shared_ptr<CJNISurfaceTexture> m_surfaceTexture;
  boost::shared_ptr<CDVDMediaCodecOnFrameAvailable> m_frameAvailable;

  std::queue<amc_demux> m_demux;
  std::vector<CJNIByteBuffer> m_input;
  std::vector<CJNIByteBuffer> m_output;
  std::vector<CDVDMediaCodecInfo*> m_inflight;

  CBitstreamConverter *m_bitstream;
  DVDVideoPicture m_videobuffer;

  bool            m_render_sw;
  int             m_src_offset[4];
  int             m_src_stride[4];
};
