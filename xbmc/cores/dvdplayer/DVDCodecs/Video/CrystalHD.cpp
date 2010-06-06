/*
 *      Copyright (C) 2005-2009 Team XBMC
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

#if (defined HAVE_CONFIG_H) && (!defined WIN32)
  #include "config.h"
#elif defined(_WIN32)
  #include "system.h"
  #include "WIN32Util.h"
  #include "util.h"
#endif

#if defined(HAVE_LIBCRYSTALHD)
#include "CrystalHD.h"

#include "DVDClock.h"
#include "DynamicDll.h"
#include "utils/Atomics.h"
#include "utils/Thread.h"
#include "utils/log.h"
#include "utils/fastmemcpy.h"

namespace BCM
{
  #if defined(WIN32)
    typedef void		*HANDLE;
  #else
    #ifndef __LINUX_USER__
      #define __LINUX_USER__
    #endif
  #endif

  #include <libcrystalhd/bc_dts_types.h>
  #include <libcrystalhd/bc_dts_defs.h>
  #include <libcrystalhd/libcrystalhd_if.h>
};

#define __MODULE_NAME__ "CrystalHD"
//#define USE_CHD_SINGLE_THREADED_API

class DllLibCrystalHDInterface
{
public:
  virtual ~DllLibCrystalHDInterface() {}
  virtual BCM::BC_STATUS DtsDeviceOpen(void *hDevice, uint32_t mode)=0;
  virtual BCM::BC_STATUS DtsDeviceClose(void *hDevice)=0;
  virtual BCM::BC_STATUS DtsOpenDecoder(void *hDevice, uint32_t StreamType)=0;
  virtual BCM::BC_STATUS DtsCloseDecoder(void *hDevice)=0;
  virtual BCM::BC_STATUS DtsStartDecoder(void *hDevice)=0;
  virtual BCM::BC_STATUS DtsSetVideoParams(void *hDevice, uint32_t videoAlg, int FGTEnable, int MetaDataEnable, int Progressive, uint32_t OptFlags)=0;
  virtual BCM::BC_STATUS DtsStartCapture(void *hDevice)=0;
  virtual BCM::BC_STATUS DtsFlushRxCapture(void *hDevice, int bDiscardOnly)=0;
  virtual BCM::BC_STATUS DtsSetFFRate(void *hDevice, uint32_t rate)=0;
  virtual BCM::BC_STATUS DtsGetDriverStatus(void *hDevice, BCM::BC_DTS_STATUS *pStatus)=0;
  virtual BCM::BC_STATUS DtsProcInput(void *hDevice, uint8_t *pUserData, uint32_t ulSizeInBytes, uint64_t timeStamp, int encrypted)=0;
  virtual BCM::BC_STATUS DtsProcOutput(void *hDevice, uint32_t milliSecWait, BCM::BC_DTS_PROC_OUT *pOut)=0;
  virtual BCM::BC_STATUS DtsProcOutputNoCopy(void *hDevice, uint32_t milliSecWait, BCM::BC_DTS_PROC_OUT *pOut)=0;
  virtual BCM::BC_STATUS DtsReleaseOutputBuffs(void *hDevice, void *Reserved, int fChange)=0;
  virtual BCM::BC_STATUS DtsSetSkipPictureMode(void *hDevice, uint32_t Mode)=0;
  virtual BCM::BC_STATUS DtsFlushInput(void *hDevice, uint32_t SkipMode)=0;
  
};

class DllLibCrystalHD : public DllDynamic, DllLibCrystalHDInterface
{
  DECLARE_DLL_WRAPPER(DllLibCrystalHD, DLL_PATH_LIBCRYSTALHD)

  DEFINE_METHOD2(BCM::BC_STATUS, DtsDeviceOpen,      (void *p1, uint32_t p2))
  DEFINE_METHOD1(BCM::BC_STATUS, DtsDeviceClose,     (void *p1))
  DEFINE_METHOD2(BCM::BC_STATUS, DtsOpenDecoder,     (void *p1, uint32_t p2))
  DEFINE_METHOD1(BCM::BC_STATUS, DtsCloseDecoder,    (void *p1))
  DEFINE_METHOD1(BCM::BC_STATUS, DtsStartDecoder,    (void *p1))
  DEFINE_METHOD1(BCM::BC_STATUS, DtsStopDecoder,     (void *p1))
  DEFINE_METHOD6(BCM::BC_STATUS, DtsSetVideoParams,  (void *p1, uint32_t p2, int p3, int p4, int p5, uint32_t p6))
  DEFINE_METHOD1(BCM::BC_STATUS, DtsStartCapture,    (void *p1))
  DEFINE_METHOD2(BCM::BC_STATUS, DtsFlushRxCapture,  (void *p1, int p2))
  DEFINE_METHOD2(BCM::BC_STATUS, DtsSetFFRate,       (void *p1, uint32_t p2))
  DEFINE_METHOD2(BCM::BC_STATUS, DtsGetDriverStatus, (void *p1, BCM::BC_DTS_STATUS *p2))
  DEFINE_METHOD5(BCM::BC_STATUS, DtsProcInput,       (void *p1, uint8_t *p2, uint32_t p3, uint64_t p4, int p5))
  DEFINE_METHOD3(BCM::BC_STATUS, DtsProcOutput,      (void *p1, uint32_t p2, BCM::BC_DTS_PROC_OUT *p3))
  DEFINE_METHOD3(BCM::BC_STATUS, DtsProcOutputNoCopy,(void *p1, uint32_t p2, BCM::BC_DTS_PROC_OUT *p3))
  DEFINE_METHOD3(BCM::BC_STATUS, DtsReleaseOutputBuffs,(void *p1, void *p2, int p3))
  DEFINE_METHOD2(BCM::BC_STATUS, DtsSetSkipPictureMode,(void *p1, uint32_t p2))
  DEFINE_METHOD2(BCM::BC_STATUS, DtsFlushInput,      (void *p1, uint32_t p2))

  BEGIN_METHOD_RESOLVE()
    RESOLVE_METHOD_RENAME(DtsDeviceOpen,      DtsDeviceOpen)
    RESOLVE_METHOD_RENAME(DtsDeviceClose,     DtsDeviceClose)
    RESOLVE_METHOD_RENAME(DtsOpenDecoder,     DtsOpenDecoder)
    RESOLVE_METHOD_RENAME(DtsCloseDecoder,    DtsCloseDecoder)
    RESOLVE_METHOD_RENAME(DtsStartDecoder,    DtsStartDecoder)
    RESOLVE_METHOD_RENAME(DtsStopDecoder,     DtsStopDecoder)
    RESOLVE_METHOD_RENAME(DtsSetVideoParams,  DtsSetVideoParams)
    RESOLVE_METHOD_RENAME(DtsStartCapture,    DtsStartCapture)
    RESOLVE_METHOD_RENAME(DtsFlushRxCapture,  DtsFlushRxCapture)
    RESOLVE_METHOD_RENAME(DtsSetFFRate,       DtsSetFFRate)
    RESOLVE_METHOD_RENAME(DtsGetDriverStatus, DtsGetDriverStatus)
    RESOLVE_METHOD_RENAME(DtsProcInput,       DtsProcInput)
    RESOLVE_METHOD_RENAME(DtsProcOutput,      DtsProcOutput)
    RESOLVE_METHOD_RENAME(DtsProcOutputNoCopy,DtsProcOutputNoCopy)
    RESOLVE_METHOD_RENAME(DtsReleaseOutputBuffs,DtsReleaseOutputBuffs)
    RESOLVE_METHOD_RENAME(DtsSetSkipPictureMode,DtsSetSkipPictureMode)
    RESOLVE_METHOD_RENAME(DtsFlushInput,      DtsFlushInput)
  END_METHOD_RESOLVE()
};

void PrintFormat(BCM::BC_PIC_INFO_BLOCK &pib);
void BcmDebugLog( BCM::BC_STATUS lResult, CStdString strFuncName="");

const char* g_DtsStatusText[] = {
	"BC_STS_SUCCESS",
	"BC_STS_INV_ARG",
	"BC_STS_BUSY",		
	"BC_STS_NOT_IMPL",		
	"BC_STS_PGM_QUIT",		
	"BC_STS_NO_ACCESS",	
	"BC_STS_INSUFF_RES",	
	"BC_STS_IO_ERROR",		
	"BC_STS_NO_DATA",		
	"BC_STS_VER_MISMATCH",
	"BC_STS_TIMEOUT",		
	"BC_STS_FW_CMD_ERR",	
	"BC_STS_DEC_NOT_OPEN",
	"BC_STS_ERR_USAGE",
	"BC_STS_IO_USER_ABORT",
	"BC_STS_IO_XFR_ERROR",
	"BC_STS_DEC_NOT_STARTED",
	"BC_STS_FWHEX_NOT_FOUND",
	"BC_STS_FMT_CHANGE",
	"BC_STS_HIF_ACCESS",
	"BC_STS_CMD_CANCELLED",
	"BC_STS_FW_AUTH_FAILED",
	"BC_STS_BOOTLOADER_FAILED",
	"BC_STS_CERT_VERIFY_ERROR",
	"BC_STS_DEC_EXIST_OPEN",
	"BC_STS_PENDING",
	"BC_STS_CLK_NOCHG"
};

union pts_union
{
  double  pts_d;
  int64_t pts_i;
};

static int64_t pts_dtoi(double pts)
{
  pts_union u;
  u.pts_d = pts;
  return u.pts_i;
}

static double pts_itod(int64_t pts)
{
  pts_union u;
  u.pts_i = pts;
  return u.pts_d;
}

////////////////////////////////////////////////////////////////////////////////////////////
class CMPCOutputThread : public CThread
{
public:
  CMPCOutputThread(void *device, DllLibCrystalHD *dll);
  virtual ~CMPCOutputThread();

  unsigned int        GetReadyCount(void);
  unsigned int        GetFreeCount(void);
  CPictureBuffer*     ReadyListPop(void);
  void                FreeListPush(CPictureBuffer* pBuffer);
  bool                WaitOutput(unsigned int msec);

protected:
  void                DoFrameRateTracking(double timestamp);
  void                SetFrameRate(uint32_t resolution);
  void                SetAspectRatio(BCM::BC_PIC_INFO_BLOCK *pic_info);
  void                CopyOutAsNV12(CPictureBuffer *pBuffer, BCM::BC_DTS_PROC_OUT *procOut, int w, int h, int stride);
  void                CopyOutAsYV12(CPictureBuffer *pBuffer, BCM::BC_DTS_PROC_OUT *procOut, int w, int h, int stride);
  bool                GetDecoderOutput(void);
  virtual void        Process(void);

  CSyncPtrQueue<CPictureBuffer> m_FreeList;
  CSyncPtrQueue<CPictureBuffer> m_ReadyList;

  DllLibCrystalHD     *m_dll;
  void                *m_device;
  unsigned int        m_timeout;
  int                 m_width;
  int                 m_height;
  uint64_t            m_timestamp;
  uint64_t            m_PictureNumber;
  unsigned int        m_color_range;
  unsigned int        m_color_matrix;
  int                 m_interlace;
  bool                m_framerate_tracking;
  uint64_t            m_framerate_cnt;
  double              m_framerate_timestamp;
  double              m_framerate;
  int                 m_aspectratio_x;
  int                 m_aspectratio_y;
  CPictureBuffer      *m_interlace_buf;
  CEvent              m_ready_event;
};

////////////////////////////////////////////////////////////////////////////////////////////
#if defined(__APPLE__)
#pragma mark -
#endif
CPictureBuffer::CPictureBuffer(DVDVideoPicture::EFormat format, int width, int height)
{
  m_width = width;
  m_height = height;
  m_field = CRYSTALHD_FIELD_FULL;
  m_interlace = false;
  m_timestamp = DVD_NOPTS_VALUE;
  m_PictureNumber = 0;
  m_color_range = 0;
  m_color_matrix = 4;
  m_format = format;
  
  // setup y plane
  m_y_buffer_size = m_width * m_height;
  m_y_buffer_ptr = (unsigned char*)_aligned_malloc(m_y_buffer_size, 16);
  
  switch(m_format)
  {
    default:
    case DVDVideoPicture::FMT_NV12:
      m_u_buffer_size = 0;
      m_v_buffer_size = 0;
      m_u_buffer_ptr = NULL;
      m_v_buffer_ptr = NULL;
      m_uv_buffer_size = m_y_buffer_size / 2;
      m_uv_buffer_ptr = (unsigned char*)_aligned_malloc(m_uv_buffer_size, 16);
    break;
    case DVDVideoPicture::FMT_YUV420P:
      m_uv_buffer_size = 0;
      m_uv_buffer_ptr = NULL;
      m_u_buffer_size = m_y_buffer_size / 4;
      m_v_buffer_size = m_y_buffer_size / 4;
      m_u_buffer_ptr = (unsigned char*)_aligned_malloc(m_u_buffer_size, 16);
      m_v_buffer_ptr = (unsigned char*)_aligned_malloc(m_v_buffer_size, 16);
    break;
  }
}

CPictureBuffer::~CPictureBuffer()
{
  if (m_y_buffer_ptr) _aligned_free(m_y_buffer_ptr);
  if (m_u_buffer_ptr) _aligned_free(m_u_buffer_ptr);
  if (m_v_buffer_ptr) _aligned_free(m_v_buffer_ptr);
  if (m_uv_buffer_ptr) _aligned_free(m_uv_buffer_ptr);
}

/////////////////////////////////////////////////////////////////////////////////////////////
#if defined(__APPLE__)
#pragma mark -
#endif
CMPCOutputThread::CMPCOutputThread(void *device, DllLibCrystalHD *dll) :
  CThread(),
  m_dll(dll),
  m_device(device),
  m_timeout(20),
  m_interlace_buf(NULL),
  m_framerate_tracking(false),
  m_framerate_cnt(0),
  m_framerate_timestamp(0.0),
  m_framerate(0.0)
{
}

CMPCOutputThread::~CMPCOutputThread()
{
  while(m_ReadyList.Count())
    delete m_ReadyList.Pop();
  while(m_FreeList.Count())
    delete m_FreeList.Pop();
    
  if (m_interlace_buf)
    delete m_interlace_buf;
}

unsigned int CMPCOutputThread::GetReadyCount(void)
{
  return m_ReadyList.Count();
}

unsigned int CMPCOutputThread::GetFreeCount(void)
{
  return m_FreeList.Count();
}

CPictureBuffer* CMPCOutputThread::ReadyListPop(void)
{
  CPictureBuffer *pBuffer = m_ReadyList.Pop();
  return pBuffer;
}

void CMPCOutputThread::FreeListPush(CPictureBuffer* pBuffer)
{
  m_FreeList.Push(pBuffer);
}

bool CMPCOutputThread::WaitOutput(unsigned int msec)
{
  return m_ready_event.WaitMSec(msec);
}

void CMPCOutputThread::DoFrameRateTracking(double timestamp)
{
  if (timestamp != DVD_NOPTS_VALUE)
  {
    double duration;
    duration = timestamp - m_framerate_timestamp;
    if (duration > 0.0)
    {
      double framerate;

      framerate = DVD_TIME_BASE / duration;
      // qualify framerate, we don't care about absolute value, just
      // want to to verify range. Timestamp could be borked so ignore
      // anything that does not verify.
      // 60, 59.94 -> 60
      // 50, 49.95 -> 50
      // 30, 29.97 -> 30
      // 25, 24.975 -> 25
      // 24, 23.976 -> 24
      switch ((int)(0.5 + framerate))
      {
        case 60:
        case 50:
        case 30:
        case 25:
        case 24:
          m_framerate_timestamp += duration;
          m_framerate_cnt++;
          m_framerate = DVD_TIME_BASE / (m_framerate_timestamp/m_framerate_cnt);
        break;
      }
    }
  }
}

void CMPCOutputThread::SetFrameRate(uint32_t resolution)
{
  m_interlace = FALSE;

  switch (resolution)
  {
    case BCM::vdecRESOLUTION_1080p30:
      m_framerate = 30.0;
    break;
    case BCM::vdecRESOLUTION_1080p29_97:
      m_framerate = 30.0 * 1000.0 / 1001.0;
    break;
    case BCM::vdecRESOLUTION_1080p25 :
      m_framerate = 25.0;
    break;
    case BCM::vdecRESOLUTION_1080p24:
      m_framerate = 24.0;
    break;
    case BCM::vdecRESOLUTION_1080p23_976:
      m_framerate = 24.0 * 1000.0 / 1001.0;
    break;
    case BCM::vdecRESOLUTION_1080p0:
      // 1080p0 is ambiguious, could be 23.976 or 29.97 fps, decoder
      // just does not know. 1080p@23_976 is more common but this
      // will mess up 1080p@29_97 playback. We really need to verify
      // which framerate with duration tracking.
      m_framerate_tracking = true;
      m_framerate = 24.0 * 1000.0 / 1001.0;
    break;
    
    case BCM::vdecRESOLUTION_1080i29_97:
      m_framerate = 30.0 * 1000.0 / 1001.0;
      m_interlace = TRUE;
    break;
    case BCM::vdecRESOLUTION_1080i0:
      m_framerate = 30.0 * 1000.0 / 1001.0;
      m_interlace = TRUE;
    break;
    case BCM::vdecRESOLUTION_1080i:
      m_framerate = 30.0 * 1000.0 / 1001.0;
      m_interlace = TRUE;
    break;
    case BCM::vdecRESOLUTION_1080i25:
      m_framerate = 25.0 * 1000.0 / 1001.0;
      m_interlace = TRUE;
    break;
    
    case BCM::vdecRESOLUTION_720p59_94:
      m_framerate = 60.0 * 1000.0 / 1001.0;
    break;
    case BCM::vdecRESOLUTION_720p:
      m_framerate = 60.0 * 1000.0 / 1001.0;
    break;
    case BCM::vdecRESOLUTION_720p50:
      m_framerate = 50.0 * 1000.0 / 1001.0;
    break;
    case BCM::vdecRESOLUTION_720p29_97:
      m_framerate = 30.0 * 1000.0 / 1001.0;
    break;
    case BCM::vdecRESOLUTION_720p24:
      m_framerate = 24.0;
    break;
    case BCM::vdecRESOLUTION_720p23_976:
      // some 720p/25 will be identifed as this, enable tracking.
      m_framerate_tracking = true;
      m_framerate = 24.0 * 1000.0 / 1001.0;
    break;
    case BCM::vdecRESOLUTION_720p0:
      // 720p0 is ambiguious, could be 23.976, 29.97 or 59.97 fps, decoder
      // just does not know. 720p@23_976 is more common but this
      // will mess up other playback. We really need to verify
      // which framerate with duration tracking.
      m_framerate_tracking = true;
      m_framerate = 24.0 * 1000.0 / 1001.0;
    break;
    
    case BCM::vdecRESOLUTION_576p25:
      m_framerate = 25.0;
    break;
    case BCM::vdecRESOLUTION_576p0:
      m_framerate = 25.0;
    break;
    case BCM::vdecRESOLUTION_PAL1:
      m_framerate = 25.0 * 1000.0 / 1001.0;
      m_interlace = TRUE;
    break;
    
    case BCM::vdecRESOLUTION_480p0:
      m_framerate = 60.0;
    break;
    case BCM::vdecRESOLUTION_480p:
      m_framerate = 60.0 * 1000.0 / 1001.0;
    break;
    case BCM::vdecRESOLUTION_480p29_97:
      m_framerate = 30.0 * 1000.0 / 1001.0;
    break;
    case BCM::vdecRESOLUTION_480p23_976:
      m_framerate = 24.0 * 1000.0 / 1001.0;
    break;
    
    case BCM::vdecRESOLUTION_480i0:
      m_framerate = 30.0 * 1000.0 / 1001.0;
      m_interlace = TRUE;
    break;
    case BCM::vdecRESOLUTION_480i:
      m_framerate = 30.0 * 1000.0 / 1001.0;
      m_interlace = TRUE;
    break;
    case BCM::vdecRESOLUTION_NTSC:
      m_framerate = 30.0 * 1000.0 / 1001.0;
      m_interlace = TRUE;
    break;
    
    default:
      m_framerate_tracking = true;
      m_framerate = 24.0 * 1000.0 / 1001.0;
    break;
  }

  CLog::Log(LOGDEBUG, "%s: resolution = %x  interlace = %d", __MODULE_NAME__, resolution, m_interlace);
}

void CMPCOutputThread::SetAspectRatio(BCM::BC_PIC_INFO_BLOCK *pic_info)
{
	switch(pic_info->aspect_ratio)
  {
    case BCM::vdecAspectRatioSquare:
      m_aspectratio_x = 1;
      m_aspectratio_y = 1;
    break;
    case BCM::vdecAspectRatio12_11:
      m_aspectratio_x = 12;
      m_aspectratio_y = 11;
    break;
    case BCM::vdecAspectRatio10_11:
      m_aspectratio_x = 10;
      m_aspectratio_y = 11;
    break;
    case BCM::vdecAspectRatio16_11:
      m_aspectratio_x = 16;
      m_aspectratio_y = 11;
    break;
    case BCM::vdecAspectRatio40_33:
      m_aspectratio_x = 40;
      m_aspectratio_y = 33;
    break;
    case BCM::vdecAspectRatio24_11:
      m_aspectratio_x = 24;
      m_aspectratio_y = 11;
    break;
    case BCM::vdecAspectRatio20_11:
      m_aspectratio_x = 20;
      m_aspectratio_y = 11;
    break;
    case BCM::vdecAspectRatio32_11:
      m_aspectratio_x = 32;
      m_aspectratio_y = 11;
    break;
    case BCM::vdecAspectRatio80_33:
      m_aspectratio_x = 80;
      m_aspectratio_y = 33;
    break;
    case BCM::vdecAspectRatio18_11:
      m_aspectratio_x = 18;
      m_aspectratio_y = 11;
    break;
    case BCM::vdecAspectRatio15_11:
      m_aspectratio_x = 15;
      m_aspectratio_y = 11;
    break;
    case BCM::vdecAspectRatio64_33:
      m_aspectratio_x = 64;
      m_aspectratio_y = 33;
    break;
    case BCM::vdecAspectRatio160_99:
      m_aspectratio_x = 160;
      m_aspectratio_y = 99;
    break;
    case BCM::vdecAspectRatio4_3:
      m_aspectratio_x = 4;
      m_aspectratio_y = 3;
    break;
    case BCM::vdecAspectRatio16_9:
      m_aspectratio_x = 16;
      m_aspectratio_y = 9;
    break;
    case BCM::vdecAspectRatio221_1:
      m_aspectratio_x = 221;
      m_aspectratio_y = 1;
    break;
    case BCM::vdecAspectRatioUnknown:
      m_aspectratio_x = 0;
      m_aspectratio_y = 0;
    break;

    case BCM::vdecAspectRatioOther:
      m_aspectratio_x = pic_info->custom_aspect_ratio_width_height & 0x0000ffff;
      m_aspectratio_y = pic_info->custom_aspect_ratio_width_height >> 16;
    break;
  }
  if(m_aspectratio_x == 0)
  {
    m_aspectratio_x = 1;
    m_aspectratio_y = 1;
  }

  CLog::Log(LOGDEBUG, "%s: dec_par x = %d, dec_par y = %d", __MODULE_NAME__, m_aspectratio_x, m_aspectratio_y);
}

void CMPCOutputThread::CopyOutAsYV12(CPictureBuffer *pBuffer, BCM::BC_DTS_PROC_OUT *procOut, int w, int h, int stride)
{
  // copy y
  if (w == stride)
  {
    fast_memcpy(pBuffer->m_y_buffer_ptr, procOut->Ybuff, w * h);
  }
  else
  {
    uint8_t *s_y = procOut->Ybuff;
    uint8_t *d_y = pBuffer->m_y_buffer_ptr;
    for (int y = 0; y < h; y++, s_y += stride, d_y += w)
      fast_memcpy(d_y, s_y, w);
  }

  //copy uv packed to u,v planes (1/2 the width and 1/2 the height of y)
  uint8_t *s_uv;
  uint8_t *d_u = pBuffer->m_u_buffer_ptr;
  uint8_t *d_v = pBuffer->m_v_buffer_ptr;
  for (int y = 0; y < h/2; y++)
  {
    s_uv = procOut->UVbuff + (y * stride);
    for (int x = 0; x < w/2; x++)
    {
      *d_u++ = *s_uv++;
      *d_v++ = *s_uv++;
    }
  }
}

void CMPCOutputThread::CopyOutAsNV12(CPictureBuffer *pBuffer, BCM::BC_DTS_PROC_OUT *procOut, int w, int h, int stride)
{
  if (w == stride)
  {
    int bytes = w * h;
    // copy y
    fast_memcpy(pBuffer->m_y_buffer_ptr, procOut->Ybuff, bytes);
    // copy uv
    fast_memcpy(pBuffer->m_uv_buffer_ptr, procOut->UVbuff, bytes/2 );
  }
  else
  {
    // copy y
    uint8_t *s = procOut->Ybuff;
    uint8_t *d = pBuffer->m_y_buffer_ptr;
    for (int y = 0; y < h; y++, s += stride, d += w)
      fast_memcpy(d, s, w);
    // copy uv
    s = procOut->UVbuff;
    d = pBuffer->m_uv_buffer_ptr;
    for (int y = 0; y < h/2; y++, s += stride, d += w)
      fast_memcpy(d, s, w);
  }
}

bool CMPCOutputThread::GetDecoderOutput(void)
{
  BCM::BC_STATUS ret;
  BCM::BC_DTS_PROC_OUT procOut;
  CPictureBuffer *pBuffer = NULL;
  bool got_picture = false;

  // Setup output struct
  memset(&procOut, 0, sizeof(BCM::BC_DTS_PROC_OUT));

  // Fetch data from the decoder
  ret = m_dll->DtsProcOutputNoCopy(m_device, m_timeout, &procOut);

  switch (ret)
  {
    case BCM::BC_STS_SUCCESS:
      if (procOut.PoutFlags & BCM::BC_POUT_FLAGS_PIB_VALID)
      {
        if (procOut.PicInfo.timeStamp)
        {
          m_timestamp = procOut.PicInfo.timeStamp;
          m_PictureNumber = procOut.PicInfo.picture_number;

          if (m_framerate_tracking)
            DoFrameRateTracking(pts_itod(m_timestamp));

          // Get next output buffer from the free list
          pBuffer = m_FreeList.Pop();
          if (!pBuffer)
          {
#ifdef _WIN32
            // force Windows to use YV12 until DX renderer gets fixed.
            if (TRUE)
#else
            // No free pre-allocated buffers so make one
            if (m_interlace)
#endif
              // Something wrong with NV12 rendering of interlaced so copy out as YV12.
              // Setting the picture format will determine the copy out method.
              pBuffer = new CPictureBuffer(DVDVideoPicture::FMT_YUV420P, m_width, m_height);
            else
              pBuffer = new CPictureBuffer(DVDVideoPicture::FMT_NV12, m_width, m_height);
              
            CLog::Log(LOGDEBUG, "%s: Added a new Buffer, ReadyListCount: %d", __MODULE_NAME__, m_ReadyList.Count());
          }

          pBuffer->m_width = m_width;
          pBuffer->m_height = m_height;
          pBuffer->m_field = CRYSTALHD_FIELD_FULL;
          pBuffer->m_interlace = m_interlace > 0 ? true : false;
          pBuffer->m_framerate = m_framerate;
          pBuffer->m_timestamp = m_timestamp;
          pBuffer->m_color_range = m_color_range;
          pBuffer->m_color_matrix = m_color_matrix;
          pBuffer->m_PictureNumber = m_PictureNumber;

          int w = procOut.PicInfo.width;
          int h = procOut.PicInfo.height;
          // frame that are not equal in width to 720, 1280 or 1920
          // need to be copied by a quantized stride (possible lib/driver bug) so force it.
          int stride;
          if (w <= 720)
            stride = 720;
          else if (w <= 1280)
            stride = 1280;
          else
            stride = 1920;
            
          if (pBuffer->m_format == DVDVideoPicture::FMT_NV12)
          {
            CopyOutAsNV12(pBuffer, &procOut, w, h, stride);
          }
          else
          {
            if (pBuffer->m_interlace)
            {
              // we get a 1/2 height frame (field) from hw, not seeing the odd/even flags so
              // can't tell which frames are odd, which are even so use picture number.
              int line_width = w*2;

              if (pBuffer->m_PictureNumber & 1)
                m_interlace_buf->m_field = CRYSTALHD_FIELD_ODD;
              else
                m_interlace_buf->m_field = CRYSTALHD_FIELD_EVEN;

              // copy luma
              uint8_t *s_y = procOut.Ybuff;
              uint8_t *d_y = m_interlace_buf->m_y_buffer_ptr;
              if (m_interlace_buf->m_field == CRYSTALHD_FIELD_ODD)
                d_y += line_width;
              for (int y = 0; y < h/2; y++, s_y += stride, d_y += line_width)
              {
                fast_memcpy(d_y, s_y, w);
              }

              //copy chroma
              line_width = w/2;
              uint8_t *s_uv;
              uint8_t *d_u = m_interlace_buf->m_u_buffer_ptr;
              uint8_t *d_v = m_interlace_buf->m_v_buffer_ptr;
              if (m_interlace_buf->m_field == CRYSTALHD_FIELD_ODD)
              {
                d_u += line_width;
                d_v += line_width;
              }
              for (int y = 0; y < h/4; y++, d_u += line_width, d_v += line_width) {
                s_uv = procOut.UVbuff + (y * stride);
                for (int x = 0; x < w/2; x++) {
                  *d_u++ = *s_uv++;
                  *d_v++ = *s_uv++;
                }
              }

              pBuffer->m_field = m_interlace_buf->m_field;
              // copy y
              fast_memcpy(pBuffer->m_y_buffer_ptr,  m_interlace_buf->m_y_buffer_ptr,  pBuffer->m_y_buffer_size);
              // copy u
              fast_memcpy(pBuffer->m_u_buffer_ptr, m_interlace_buf->m_u_buffer_ptr, pBuffer->m_u_buffer_size);
              // copy v
              fast_memcpy(pBuffer->m_v_buffer_ptr, m_interlace_buf->m_v_buffer_ptr, pBuffer->m_v_buffer_size);
            }
            else
            {
              CopyOutAsYV12(pBuffer, &procOut, w, h, stride);
            }
          }

          m_ReadyList.Push(pBuffer);
          m_ready_event.Set();                                                                                                                                                                                                       
          got_picture = true;
        }
        else
        {
          if (m_PictureNumber != procOut.PicInfo.picture_number)
            CLog::Log(LOGDEBUG, "%s: No timestamp detected: %llu", __MODULE_NAME__, procOut.PicInfo.timeStamp);
          m_PictureNumber = procOut.PicInfo.picture_number;
        }
      }

      m_dll->DtsReleaseOutputBuffs(m_device, NULL, FALSE);
    break;

    case BCM::BC_STS_NO_DATA:
    break;

    case BCM::BC_STS_FMT_CHANGE:
      CLog::Log(LOGDEBUG, "%s: Format Change Detected. Flags: 0x%08x", __MODULE_NAME__, procOut.PoutFlags);
      if ((procOut.PoutFlags & BCM::BC_POUT_FLAGS_PIB_VALID) && (procOut.PoutFlags & BCM::BC_POUT_FLAGS_FMT_CHANGE))
      {
        PrintFormat(procOut.PicInfo);

        if (procOut.PicInfo.height == 1088) {
          procOut.PicInfo.height = 1080;
        }
        m_width = procOut.PicInfo.width;
        m_height = procOut.PicInfo.height;
        m_color_range = 0;
        m_color_matrix = procOut.PicInfo.colour_primaries;
        SetAspectRatio(&procOut.PicInfo);
        SetFrameRate(procOut.PicInfo.frame_rate);
        if (procOut.PicInfo.flags & VDEC_FLAG_INTERLACED_SRC)
        {
          m_interlace = true;
          m_interlace_buf = new CPictureBuffer(DVDVideoPicture::FMT_YUV420P, m_width, m_height);
        }
        m_timeout = 2000;
        m_ready_event.Set();                                                                                                                                                                                                       
      }
    break;

    case BCM::BC_STS_TIMEOUT:
    break;

    default:
      if (ret > 26)
        CLog::Log(LOGDEBUG, "%s: DtsProcOutput returned %d.", __MODULE_NAME__, ret);
      else
        CLog::Log(LOGDEBUG, "%s: DtsProcOutput returned %s.", __MODULE_NAME__, g_DtsStatusText[ret]);
    break;
  }
  
  return got_picture;
}

void CMPCOutputThread::Process(void)
{
  BCM::BC_STATUS ret;
  BCM::BC_DTS_STATUS decoder_status;

  m_PictureNumber = 0;

  CLog::Log(LOGDEBUG, "%s: Output Thread Started...", __MODULE_NAME__);

  // wait for decoder startup, calls into DtsProcOutputXXCopy will
  // return immediately until decoder starts getting input packets. 
  while (!m_bStop)
  {
    ret = m_dll->DtsGetDriverStatus(m_device, &decoder_status);
    if (ret == BCM::BC_STS_SUCCESS && decoder_status.ReadyListCount)
    {
      GetDecoderOutput();
      break;
    }
    Sleep(10);
  }

  // decoder is primed so now calls in DtsProcOutputXXCopy will block
  while (!m_bStop)
  {
    if (!GetDecoderOutput())
      Sleep(1);

#ifdef USE_CHD_SINGLE_THREADED_API
    while (!m_bStop)
    {
      ret = m_dll->DtsGetDriverStatus(m_device, &decoder_status);
      if (ret == BCM::BC_STS_SUCCESS && decoder_status.ReadyListCount != 0)
      {
        double pts = pts_itod(decoder_status.NextTimeStamp);
        fprintf(stdout, "cpbEmptySize(%d), NextTimeStamp(%f)\n", decoder_status.cpbEmptySize, pts);
        break;
      }
      Sleep(10);
    }
#endif
  }
  CLog::Log(LOGDEBUG, "%s: Output Thread Stopped...", __MODULE_NAME__);
}

////////////////////////////////////////////////////////////////////////////////////////////
#if defined(__APPLE__)
#pragma mark -
#endif
CCrystalHD* CCrystalHD::m_pInstance = NULL;

CCrystalHD::CCrystalHD() :
  m_device(NULL),
  m_decoder_open(false),
  m_drop_state(false),
  m_pOutputThread(NULL)
{

  m_dll = new DllLibCrystalHD;
#ifdef _WIN32
  CStdString  strDll;
  if(CWIN32Util::GetCrystalHDLibraryPath(strDll) && m_dll->SetFile(strDll) && m_dll->Load() && m_dll->IsLoaded() )
#else
  if (m_dll->Load() && m_dll->IsLoaded() )
#endif
    OpenDevice();

  // delete dll if device open fails, minimizes ram footprint
  if (!m_device)
  {
    delete m_dll;
    m_dll = NULL;
    CLog::Log(LOGINFO, "%s: broadcom crystal hd not found", __MODULE_NAME__);
  }
}


CCrystalHD::~CCrystalHD()
{
  if (m_decoder_open)
    CloseDecoder();

  if (m_device)
    CloseDevice();

  if (m_dll)
    delete m_dll;
}


bool CCrystalHD::DevicePresent(void)
{
  return m_device != NULL;
}

bool CCrystalHD::Wake(void)
{
  return true;
}

bool CCrystalHD::Sleep(void)
{
  return true;
}

void CCrystalHD::RemoveInstance(void)
{
  if (m_pInstance)
  {
    delete m_pInstance;
    m_pInstance = NULL;
  }
}

CCrystalHD* CCrystalHD::GetInstance(void)
{
  if (!m_pInstance)
  {
    m_pInstance = new CCrystalHD();
  }
  return m_pInstance;
}

void CCrystalHD::OpenDevice()
{
  uint32_t mode = BCM::DTS_PLAYBACK_MODE          |
                  BCM::DTS_LOAD_FILE_PLAY_FW      |
#ifdef USE_CHD_SINGLE_THREADED_API
                  BCM::DTS_SINGLE_THREADED_MODE   |
#endif
                  BCM::DTS_PLAYBACK_DROP_RPT_MODE |
                  DTS_DFLT_RESOLUTION(BCM::vdecRESOLUTION_720p23_976);

  BCM::BC_STATUS res= m_dll->DtsDeviceOpen(&m_device, mode);
  if (res != BCM::BC_STS_SUCCESS)
  {
    m_device = NULL;
    if( res == BCM::BC_STS_DEC_EXIST_OPEN )
      CLog::Log(LOGERROR, "%s: device owned by another application", __MODULE_NAME__);
    else
      CLog::Log(LOGERROR, "%s: device open failed", __MODULE_NAME__);
  }
  else
  {
    CLog::Log(LOGINFO, "%s: device opened", __MODULE_NAME__);
  }
}

void CCrystalHD::CloseDevice()
{
  if (m_device)
  {
    m_dll->DtsDeviceClose(m_device);
    m_device = NULL;
    CLog::Log(LOGINFO, "%s: device closed", __MODULE_NAME__);
  }
}

bool CCrystalHD::OpenDecoder(CRYSTALHD_CODEC_TYPE codec_type, int extradata_size, void *extradata)
{
  BCM::BC_STATUS res;
  uint32_t StreamType;

  if (!m_device)
    return false;

  if (m_decoder_open)
    CloseDecoder();

  uint32_t videoAlg = 0;
  switch (codec_type)
  {
    case CRYSTALHD_CODEC_ID_VC1:
      videoAlg = BCM::BC_VID_ALGO_VC1;
      StreamType = BCM::BC_STREAM_TYPE_ES;
    break;
    case CRYSTALHD_CODEC_ID_WMV3:
      videoAlg = BCM::BC_VID_ALGO_VC1MP;
      StreamType = BCM::BC_STREAM_TYPE_PES;
    break;
    case CRYSTALHD_CODEC_ID_H264:
      videoAlg = BCM::BC_VID_ALGO_H264;
      StreamType = BCM::BC_STREAM_TYPE_ES;
    break;
    case CRYSTALHD_CODEC_ID_MPEG2:
      videoAlg = BCM::BC_VID_ALGO_MPEG2;
      StreamType = BCM::BC_STREAM_TYPE_ES;
    break;
    default:
      return false;
    break;
  }

  do
  {
    res = m_dll->DtsOpenDecoder(m_device, StreamType);
    if (res != BCM::BC_STS_SUCCESS)
    {
      CLog::Log(LOGERROR, "%s: open decoder failed", __MODULE_NAME__);
      break;
    }
#ifdef USE_CHD_SINGLE_THREADED_API
    res = m_dll->DtsSetVideoParams(m_device, videoAlg, FALSE, FALSE, TRUE, 0x80 | 0x80000000 | BCM::vdecFrameRate23_97);
#else
    res = m_dll->DtsSetVideoParams(m_device, videoAlg, FALSE, FALSE, TRUE, 0x80000000 | BCM::vdecFrameRate23_97);
#endif
    if (res != BCM::BC_STS_SUCCESS)
    {
      CLog::Log(LOGDEBUG, "%s: set video params failed", __MODULE_NAME__);
      break;
    }
    res = m_dll->DtsStartDecoder(m_device);
    if (res != BCM::BC_STS_SUCCESS)
    {
      CLog::Log(LOGDEBUG, "%s: start decoder failed", __MODULE_NAME__);
      break;
    }
    res = m_dll->DtsStartCapture(m_device);
    if (res != BCM::BC_STS_SUCCESS)
    {
      CLog::Log(LOGDEBUG, "%s: start capture failed", __MODULE_NAME__);
      break;
    }

    m_pOutputThread = new CMPCOutputThread(m_device, m_dll);
    m_pOutputThread->Create();

    m_drop_state = false;
    m_decoder_open = true;

    CLog::Log(LOGDEBUG, "%s: codec opened", __MODULE_NAME__);
  } while(false);

  return m_decoder_open;
}

void CCrystalHD::CloseDecoder(void)
{
  if (m_pOutputThread)
  {
    while(m_BusyList.Count())
      m_pOutputThread->FreeListPush( m_BusyList.Pop() );

    m_pOutputThread->StopThread();
    delete m_pOutputThread;
    m_pOutputThread = NULL;
  }

  if (m_device)
  {
    m_dll->DtsFlushRxCapture(m_device, true);
    m_dll->DtsStopDecoder(m_device);
    m_dll->DtsCloseDecoder(m_device);
  }
  m_decoder_open = false;

  CLog::Log(LOGDEBUG, "%s: codec closed", __MODULE_NAME__);
}

void CCrystalHD::Reset(void)
{
  // Calling for non-error flush, flush all 
  m_dll->DtsFlushInput(m_device, 1);
  m_dll->DtsFlushRxCapture(m_device, true);

  while (m_pOutputThread->GetReadyCount())
    m_pOutputThread->FreeListPush( m_pOutputThread->ReadyListPop() );

  while (m_BusyList.Count())
    m_pOutputThread->FreeListPush( m_BusyList.Pop() );

  m_timestamps.clear();

  CLog::Log(LOGDEBUG, "%s: codec flushed", __MODULE_NAME__);
}

bool CCrystalHD::AddInput(unsigned char *pData, size_t size, double dts, double pts)
{
  if (pData)
  {
    BCM::BC_STATUS ret;

    do
    {
      ret = m_dll->DtsProcInput(m_device, pData, size, pts_dtoi(pts), 0);
      if (ret == BCM::BC_STS_SUCCESS)
      {
        CHD_TIMESTAMP timestamp;

        m_last_pts = pts;
        timestamp.dts = dts;
        timestamp.pts = pts;
        m_timestamps.push_back(timestamp);
      }
      else if (ret == BCM::BC_STS_BUSY)
      {
        CLog::Log(LOGDEBUG, "%s: DtsProcInput returned BC_STS_BUSY", __MODULE_NAME__);
        ::Sleep(1); // Buffer is full, sleep it empty
      }
    } while (ret != BCM::BC_STS_SUCCESS);

    bool wait_state;
    if (!m_drop_state)
      wait_state = m_pOutputThread->WaitOutput(10);
  }

  return true;
}

int CCrystalHD::GetReadyCount(void)
{
  if (m_pOutputThread)
    return m_pOutputThread->GetReadyCount();
  else
    return 0;
}

void CCrystalHD::BusyListFlush(void)
{
  if (m_pOutputThread)
  {
    // leave one around, DVDPlayer expects it
    while( m_BusyList.Count() > 1)
      m_pOutputThread->FreeListPush( m_BusyList.Pop() );
  }
}

bool CCrystalHD::GetPicture(DVDVideoPicture *pDvdVideoPicture)
{
  CPictureBuffer* pBuffer = m_pOutputThread->ReadyListPop();

  if (!pBuffer)
    return false;

  if (pBuffer->m_timestamp == 0)
  {
    // All timestamps that we pass to hardware have a value != 0
    // so this a picture frame came from a demxer packet with more than one
    // picture frames encoded inside. Set DVD_NOPTS_VALUE both dts/pts and
    // let DVDPlayerVideo sort it out.
    pDvdVideoPicture->dts = DVD_NOPTS_VALUE;
    pDvdVideoPicture->pts = DVD_NOPTS_VALUE;
  }
  else
  {
    if (!m_timestamps.empty())
    {
      CHD_TIMESTAMP timestamp;
      
      timestamp = m_timestamps.front();
      m_timestamps.pop_front();
      pDvdVideoPicture->dts = timestamp.dts;
      pDvdVideoPicture->pts = pts_itod(pBuffer->m_timestamp);
    }
    else
    {
      pDvdVideoPicture->dts = DVD_NOPTS_VALUE;
      pDvdVideoPicture->pts = pts_itod(pBuffer->m_timestamp);
    }
  }

  pDvdVideoPicture->iWidth = pBuffer->m_width;
  pDvdVideoPicture->iHeight = pBuffer->m_height;
  pDvdVideoPicture->iDisplayWidth = pBuffer->m_width;
  pDvdVideoPicture->iDisplayHeight = pBuffer->m_height;

  // Y plane
  pDvdVideoPicture->data[0] = (BYTE*)pBuffer->m_y_buffer_ptr;
  pDvdVideoPicture->iLineSize[0] = pBuffer->m_width;
  switch(pBuffer->m_format)
  {
    default:
    case DVDVideoPicture::FMT_NV12:
      // UV packed plane
      pDvdVideoPicture->data[1] = (BYTE*)pBuffer->m_uv_buffer_ptr;
      pDvdVideoPicture->iLineSize[1] = pBuffer->m_width;
      // unused
      pDvdVideoPicture->data[2] = NULL;
      pDvdVideoPicture->iLineSize[2] = 0;
    break;
    case DVDVideoPicture::FMT_YUV420P:
      // U plane
      pDvdVideoPicture->data[1] = (BYTE*)pBuffer->m_u_buffer_ptr;
      pDvdVideoPicture->iLineSize[1] = pBuffer->m_width / 2;
      // V plane
      pDvdVideoPicture->data[2] = (BYTE*)pBuffer->m_v_buffer_ptr;
      pDvdVideoPicture->iLineSize[2] = pBuffer->m_width / 2;
    break;
  }

  pDvdVideoPicture->iRepeatPicture = 0;
  pDvdVideoPicture->iDuration = (DVD_TIME_BASE / pBuffer->m_framerate);
  pDvdVideoPicture->color_range = pBuffer->m_color_range;
  pDvdVideoPicture->color_matrix = pBuffer->m_color_matrix;
  pDvdVideoPicture->iFlags = DVP_FLAG_ALLOCATED;
  pDvdVideoPicture->iFlags |= pBuffer->m_interlace ? DVP_FLAG_INTERLACED : 0;
  //if (pBuffer->m_interlace)
  //  pDvdVideoPicture->iFlags |= DVP_FLAG_TOP_FIELD_FIRST;
  pDvdVideoPicture->iFlags |= m_drop_state ? DVP_FLAG_DROPPED : 0;
  pDvdVideoPicture->format = pBuffer->m_format;

  while( m_BusyList.Count())
    m_pOutputThread->FreeListPush( m_BusyList.Pop() );

  m_BusyList.Push(pBuffer);
  return true;
}

void CCrystalHD::SetDropState(bool bDrop)
{
  if (m_drop_state != bDrop)
  {
    m_drop_state = bDrop;
  }
  if (m_drop_state)
    CLog::Log(LOGDEBUG, "%s: SetDropState... %d, , GetFreeCount(%d), GetReadyCount(%d), BusyListCount(%d)", __MODULE_NAME__, 
      m_drop_state, m_pOutputThread->GetFreeCount(), m_pOutputThread->GetReadyCount(), m_BusyList.Count());
}

////////////////////////////////////////////////////////////////////////////////////////////
void PrintFormat(BCM::BC_PIC_INFO_BLOCK &pib)
{
  CLog::Log(LOGDEBUG, "----------------------------------\n%s","");
  CLog::Log(LOGDEBUG, "\tTimeStamp: %llu\n", pib.timeStamp);
  CLog::Log(LOGDEBUG, "\tPicture Number: %d\n", pib.picture_number);
  CLog::Log(LOGDEBUG, "\tWidth: %d\n", pib.width);
  CLog::Log(LOGDEBUG, "\tHeight: %d\n", pib.height);
  CLog::Log(LOGDEBUG, "\tChroma: 0x%03x\n", pib.chroma_format);
  CLog::Log(LOGDEBUG, "\tPulldown: %d\n", pib.pulldown);
  CLog::Log(LOGDEBUG, "\tFlags: 0x%08x\n", pib.flags);
  CLog::Log(LOGDEBUG, "\tFrame Rate/Res: %d\n", pib.frame_rate);
  CLog::Log(LOGDEBUG, "\tAspect Ratio: %d\n", pib.aspect_ratio);
  CLog::Log(LOGDEBUG, "\tColor Primaries: %d\n", pib.colour_primaries);
  CLog::Log(LOGDEBUG, "\tMetaData: %d\n", pib.picture_meta_payload);
  CLog::Log(LOGDEBUG, "\tSession Number: %d\n", pib.sess_num);
  CLog::Log(LOGDEBUG, "\tTimeStamp: %d\n", pib.ycom);
  CLog::Log(LOGDEBUG, "\tCustom Aspect: %d\n", pib.custom_aspect_ratio_width_height);
  CLog::Log(LOGDEBUG, "\tFrames to Drop: %d\n", pib.n_drop);
  CLog::Log(LOGDEBUG, "\tH264 Valid Fields: 0x%08x\n", pib.other.h264.valid);
}

#endif
