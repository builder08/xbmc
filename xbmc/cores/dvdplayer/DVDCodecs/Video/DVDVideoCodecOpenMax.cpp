/*
 *      Copyright (C) 2010 Team XBMC
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
#endif

#if defined(HAVE_LIBOPENMAX)
#include "DynamicDll.h"
#include "GUISettings.h"
#include "DVDClock.h"
#include "DVDStreamInfo.h"
#include "DVDVideoCodecOpenMax.h"
#include "utils/log.h"

#include <OpenMAX/il/OMX_Core.h>
#include <OpenMAX/il/OMX_Component.h>
#include <OpenMAX/il/OMX_Index.h>
#include <OpenMAX/il/OMX_Image.h>

// EGL extension functions
static PFNEGLCREATEIMAGEKHRPROC eglCreateImageKHR;
static PFNEGLDESTROYIMAGEKHRPROC eglDestroyImageKHR;

////////////////////////////////////////////////////////////////////////////////////////////
class DllLibOpenMaxInterface
{
public:
  virtual ~DllLibOpenMaxInterface() {}

  virtual OMX_ERRORTYPE OMX_Init(void) = 0;
  virtual OMX_ERRORTYPE OMX_Deinit(void) = 0;
  virtual OMX_ERRORTYPE OMX_GetHandle(
    OMX_HANDLETYPE *pHandle, OMX_STRING cComponentName, OMX_PTR pAppData, OMX_CALLBACKTYPE *pCallBacks) = 0;
  virtual OMX_ERRORTYPE OMX_FreeHandle(OMX_HANDLETYPE hComponent) = 0;
  virtual OMX_ERRORTYPE OMX_GetComponentsOfRole(OMX_STRING role, OMX_U32 *pNumComps, OMX_U8 **compNames) = 0;
  virtual OMX_ERRORTYPE OMX_GetRolesOfComponent(OMX_STRING compName, OMX_U32 *pNumRoles, OMX_U8 **roles) = 0;
  virtual OMX_ERRORTYPE OMX_ComponentNameEnum(OMX_STRING cComponentName, OMX_U32 nNameLength, OMX_U32 nIndex) = 0;
};

class DllLibOpenMax : public DllDynamic, DllLibOpenMaxInterface
{
  DECLARE_DLL_WRAPPER(DllLibOpenMax, "/usr/lib/libnvomx.so")

  DEFINE_METHOD0(OMX_ERRORTYPE, OMX_Init)
  DEFINE_METHOD0(OMX_ERRORTYPE, OMX_Deinit)
  DEFINE_METHOD4(OMX_ERRORTYPE, OMX_GetHandle, (OMX_HANDLETYPE *p1, OMX_STRING p2, OMX_PTR p3, OMX_CALLBACKTYPE *p4))
  DEFINE_METHOD1(OMX_ERRORTYPE, OMX_FreeHandle, (OMX_HANDLETYPE p1))
  DEFINE_METHOD3(OMX_ERRORTYPE, OMX_GetComponentsOfRole, (OMX_STRING p1, OMX_U32 *p2, OMX_U8 **p3))
  DEFINE_METHOD3(OMX_ERRORTYPE, OMX_GetRolesOfComponent, (OMX_STRING p1, OMX_U32 *p2, OMX_U8 **p3))
  DEFINE_METHOD3(OMX_ERRORTYPE, OMX_ComponentNameEnum, (OMX_STRING p1, OMX_U32 p2, OMX_U32 p3))
  BEGIN_METHOD_RESOLVE()
    RESOLVE_METHOD(OMX_Init)
    RESOLVE_METHOD(OMX_Deinit)
    RESOLVE_METHOD(OMX_GetHandle)
    RESOLVE_METHOD(OMX_FreeHandle)
    RESOLVE_METHOD(OMX_GetComponentsOfRole)
    RESOLVE_METHOD(OMX_GetRolesOfComponent)
    RESOLVE_METHOD(OMX_ComponentNameEnum)
  END_METHOD_RESOLVE()
};

////////////////////////////////////////////////////////////////////////////////////////////
// debug spew defines
#define OMX_DEBUG_VERBOSE
#define OMX_DEBUG_EVENTHANDLER
#define OMX_DEBUG_FILLBUFFERDONE
#define OMX_DEBUG_EMPTYBUFFERDONE

// TODO: These are Nvidia Tegra2 dependent, need to dynamiclly find the
// right codec matched to video format.
#define OMX_H264BASE_DECODER    "OMX.Nvidia.h264.decode"
// OMX.Nvidia.h264ext.decode segfaults, not sure why.
//#define OMX_H264MAIN_DECODER  "OMX.Nvidia.h264ext.decode"
#define OMX_H264MAIN_DECODER    "OMX.Nvidia.h264.decode"
#define OMX_H264HIGH_DECODER    "OMX.Nvidia.h264ext.decode"
#define OMX_MPEG4_DECODER       "OMX.Nvidia.mp4.decode"
#define OMX_MPEG4EXT_DECODER    "OMX.Nvidia.mp4ext.decode"
#define OMX_MPEG2V_DECODER      "OMX.Nvidia.mpeg2v.decode"
#define OMX_VC1_DECODER         "OMX.Nvidia.vc1.decode"

// quick and dirty scalers to calc image buffer sizes
#define FACTORFORMAT422 2
#define FACTORFORMAT420 1.5

#define OMX_INIT_STRUCTURE(a) \
  memset(&(a), 0, sizeof(a)); \
  (a).nSize = sizeof(a); \
  (a).nVersion.s.nVersionMajor = OMX_VERSION_MAJOR; \
  (a).nVersion.s.nVersionMinor = OMX_VERSION_MINOR; \
  (a).nVersion.s.nRevision = OMX_VERSION_REVISION; \
  (a).nVersion.s.nStep = OMX_VERSION_STEP

////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
CDVDVideoCodecOpenMax::CDVDVideoCodecOpenMax() : CDVDVideoCodec()
{
  m_dll = new DllLibOpenMax;
  m_dll->Load();

  m_omx_decoder = NULL;
  m_pFormatName = "omx-xxxx";

  pthread_mutex_init(&m_omx_ready_mutex, NULL);
  pthread_mutex_init(&m_omx_avaliable_mutex, NULL);

  m_omx_state_change = (sem_t*)malloc(sizeof(sem_t));
  sem_init(m_omx_state_change, 0, 0);
  /*
  m_omx_flush_input  = (sem_t*)malloc(sizeof(sem_t));
  sem_init(m_omx_flush_input, 0, 0);
  m_omx_flush_output = (sem_t*)malloc(sizeof(sem_t));
  sem_init(m_omx_flush_output, 0, 0);
  */

  m_convert_bitstream = false;
  m_videoplayback_done = false;
  memset(&m_videobuffer, 0, sizeof(DVDVideoPicture));
}

CDVDVideoCodecOpenMax::~CDVDVideoCodecOpenMax()
{
  Dispose();
  pthread_mutex_destroy(&m_omx_ready_mutex);
  pthread_mutex_destroy(&m_omx_avaliable_mutex);
  sem_destroy(m_omx_state_change);
  free(m_omx_state_change);
  /*
  sem_destroy(m_omx_flush_input);
  free(m_omx_flush_input);
  sem_destroy(m_omx_flush_output);
  free(m_omx_flush_output);
  */
  delete m_dll;
}

bool CDVDVideoCodecOpenMax::Open(CDVDStreamInfo &hints, CDVDCodecOptions &options)
{
  // we always qualify even if DVDFactoryCodec does this too.
  if (g_guiSettings.GetBool("videoplayer.useomx") && !hints.software)
  {
    OMX_ERRORTYPE omx_err = OMX_ErrorNone;
    std::string decoder_name;

    m_decoded_width  = hints.width;
    m_decoded_height = hints.height;
    m_convert_bitstream = false;

    switch (hints.codec)
    {
      case CODEC_ID_H264:
      {
        switch(hints.profile)
        {
          case FF_PROFILE_H264_BASELINE:
            // (role name) video_decoder.avc
            // H.264 Baseline profile
            decoder_name = OMX_H264BASE_DECODER;
            m_pFormatName = "omx-h264(b)";
          break;
          case FF_PROFILE_H264_MAIN:
            // (role name) video_decoder.avc
            // H.264 Main profile
            decoder_name = OMX_H264MAIN_DECODER;
            m_pFormatName = "omx-h264(m)";
          break;
          case FF_PROFILE_H264_HIGH:
            // (role name) video_decoder.avc
            // H.264 Main profile
            decoder_name = OMX_H264HIGH_DECODER;
            m_pFormatName = "omx-h264(h)";
          break;
          default:
            return false;
          break;
        }
        if (hints.extrasize < 7 || hints.extradata == NULL)
        {
          CLog::Log(LOGNOTICE, "%s - avcC data too small or missing", __FUNCTION__);
          return false;
        }
        // valid avcC data (bitstream) always starts with the value 1 (version)
        if ( *(char*)hints.extradata == 1 )
          m_convert_bitstream = bitstream_convert_init(hints.extradata, hints.extrasize);
      }
      break;
      case CODEC_ID_MPEG4:
        // (role name) video_decoder.mpeg4
        // MPEG-4, DivX 4/5 and Xvid compatible
        decoder_name = OMX_MPEG4_DECODER;
        m_pFormatName = "omx-mpeg4";
      break;
      /*
      TODO: what mpeg4 formats are "ext" ????
      case NvxStreamType_MPEG4Ext:
        // (role name) video_decoder.mpeg4
        // MPEG-4, DivX 4/5 and Xvid compatible
        decoder_name = OMX_MPEG4EXT_DECODER;
        m_pFormatName = "omx-mpeg4";
      break;
      */
      case CODEC_ID_MPEG2VIDEO:
        // (role name) video_decoder.mpeg2
        // MPEG-2
        decoder_name = OMX_MPEG2V_DECODER;
        m_pFormatName = "omx-mpeg2";
      break;
      case CODEC_ID_VC1:
        // (role name) video_decoder.vc1
        // VC-1, WMV9
        decoder_name = OMX_VC1_DECODER;
        m_pFormatName = "omx-vc1";
      break;
      default:
        return false;
      break;
    }

    // initialize OpenMAX.
    omx_err = m_dll->OMX_Init();
    if (omx_err)
    {
      CLog::Log(LOGERROR, "%s - OpenMax failed to init, status(%d), codec(%d), profile(%d), level(%d)", 
        __FUNCTION__, omx_err, hints.codec, hints.profile, hints.level);
      return false;
    }

    // TODO: Find component from role name.

    // Get video decoder handle setting up callbacks, component is in loaded state on return.
    static OMX_CALLBACKTYPE decoder_callbacks = {
      &DecoderEventHandler, &DecoderEmptyBufferDone, &DecoderFillBufferDone };
    omx_err = m_dll->OMX_GetHandle(&m_omx_decoder, (char*)decoder_name.c_str(), this, &decoder_callbacks);
    if (omx_err)
    {
      CLog::Log(LOGERROR, "%s - could not get decoder handle\n", __FUNCTION__);
      m_dll->OMX_Deinit();
      return false;
    }

    // Get the port information. This will obtain information about the
    // number of ports and index of the first port.
    OMX_PORT_PARAM_TYPE port_param;
    OMX_INIT_STRUCTURE(port_param);
    omx_err = OMX_GetParameter(m_omx_decoder, OMX_IndexParamVideoInit, &port_param);
    if (omx_err)
    {
      CLog::Log(LOGERROR, "%s - failed to get component port parameter\n", __FUNCTION__);
      m_dll->OMX_FreeHandle(m_omx_decoder);
      m_omx_decoder = NULL;
      m_dll->OMX_Deinit();
      return false;
    }
    m_omx_input_port = port_param.nStartPortNumber;
    m_omx_output_port = m_omx_input_port + 1;
    #if defined(OMX_DEBUG_VERBOSE)
    CLog::Log(LOGDEBUG, "%s - decoder_component(0x%p), input_port(0x%x), output_port(0x%x)\n", 
      __FUNCTION__, m_omx_decoder, m_omx_input_port, m_omx_output_port);
    #endif

    // TODO: Set role for the component because components could have multiple roles.

    // Component will be in OMX_StateLoaded now so we can alloc omx input/output buffers.
    // we can only alloc them in OMX_StateLoaded state or if the component port is disabled 
    // Alloc buffers for the omx input port.
    omx_err = AllocOMXInputBuffers();
    if (omx_err)
    {
      m_dll->OMX_FreeHandle(m_omx_decoder);
      m_omx_decoder = NULL;
      m_dll->OMX_Deinit();
      return false;
    }
    // Alloc buffers for the omx output port.
    omx_err = AllocOMXOutputBuffers();
    if (omx_err)
    {
      FreeOMXInputBuffers(false);
      m_dll->OMX_FreeHandle(m_omx_decoder);
      m_omx_decoder = NULL;
      m_dll->OMX_Deinit();
      return false;
    }

    // allocate a YV12 DVDVideoPicture buffer.
    // first make sure all properties are reset.
    memset(&m_videobuffer, 0, sizeof(DVDVideoPicture));
    unsigned int luma_pixels = m_decoded_width * m_decoded_height;
    unsigned int chroma_pixels = luma_pixels/4;

    m_videobuffer.pts = DVD_NOPTS_VALUE;
    m_videobuffer.format = DVDVideoPicture::FMT_YUV420P;
    m_videobuffer.color_range  = 0;
    m_videobuffer.color_matrix = 4;
    m_videobuffer.iFlags  = DVP_FLAG_ALLOCATED;
    m_videobuffer.iWidth  = m_decoded_width;
    m_videobuffer.iHeight = m_decoded_height;
    m_videobuffer.iDisplayWidth  = m_decoded_width;
    m_videobuffer.iDisplayHeight = m_decoded_height;
    m_videobuffer.format = DVDVideoPicture::FMT_YUV420P;

    m_videobuffer.iLineSize[0] = m_decoded_width;   //Y
    m_videobuffer.iLineSize[1] = m_decoded_width/2; //U
    m_videobuffer.iLineSize[2] = m_decoded_width/2; //V
    m_videobuffer.iLineSize[3] = 0;

    m_videobuffer.data[0] = (BYTE*)_aligned_malloc(luma_pixels, 16);  //Y
    m_videobuffer.data[1] = (BYTE*)_aligned_malloc(chroma_pixels, 16);//U
    m_videobuffer.data[2] = (BYTE*)_aligned_malloc(chroma_pixels, 16);//V
    m_videobuffer.data[3] = NULL;

    // set all data to 0 for less artifacts.. hmm.. what is black in YUV??
    memset(m_videobuffer.data[0], 0, luma_pixels);
    memset(m_videobuffer.data[1], 0, chroma_pixels);
    memset(m_videobuffer.data[2], 0, chroma_pixels);

    m_drop_pictures = false;
    m_videoplayback_done = false;
    
    // crank it up.
    StartDecoder();

    return true;
  }

  return false;
}

void CDVDVideoCodecOpenMax::Dispose()
{
  if (m_omx_decoder)
  {
    if (m_omx_state != OMX_StateLoaded)
      StopDecoder();
    m_dll->OMX_FreeHandle(m_omx_decoder);
    m_omx_decoder = NULL;
    m_dll->OMX_Deinit();
  }
  if (m_videobuffer.iFlags & DVP_FLAG_ALLOCATED)
  {
    _aligned_free(m_videobuffer.data[0]);
    _aligned_free(m_videobuffer.data[1]);
    _aligned_free(m_videobuffer.data[2]);
    m_videobuffer.iFlags = 0;
  }
  if (m_sps_pps_context.sps_pps_data)
  {
    free(m_sps_pps_context.sps_pps_data);
    m_sps_pps_context.sps_pps_data = NULL;
  }
}

void CDVDVideoCodecOpenMax::SetDropState(bool bDrop)
{
  m_drop_pictures = bDrop;
  
  if (m_drop_pictures)
  {
    //OMX_ERRORTYPE omx_err;
    //omx_err = OMX_SendCommand(m_omx_decoder, OMX_CommandFlush, m_omx_output_port, 0);
  }
}

int CDVDVideoCodecOpenMax::Decode(BYTE* pData, int iSize, double dts, double pts)
{
  if (pData)
  {
    int demuxer_bytes = iSize;
    uint8_t *demuxer_content = pData;
    bool bitstream_convered  = false;

    if (m_convert_bitstream)
    {
      // convert demuxer packet from bitstream to bytestream (AnnexB)
      int bytestream_size = 0;
      uint8_t *bytestream_buff = NULL;

      bitstream_convert(demuxer_content, demuxer_bytes, &bytestream_buff, &bytestream_size);
      if (bytestream_buff && (bytestream_size > 0))
      {
        bitstream_convered = true;
        demuxer_bytes = bytestream_size;
        demuxer_content = bytestream_buff;
      }
    }

    // we need to queue then de-queue the demux packet, seems silly but
    // omx might not have a omx input buffer avaliable when we are called
    // and we must store the demuxer packet and try again later.
    omx_demux_packet demux_packet;
    demux_packet.dts = dts;
    demux_packet.pts = pts;
    demux_packet.size = demuxer_bytes;
    demux_packet.buff = new OMX_U8[demuxer_bytes];
    memcpy(demux_packet.buff, demuxer_content, demuxer_bytes);
    m_demux_queue.push(demux_packet);
    if (bitstream_convered)
      free(demuxer_content);

    // we can look at m_omx_input_avaliable.empty without needing to lock/unlock
    // try to send any/all demux packets to omx decoder.  
    while (!m_omx_input_avaliable.empty() && !m_demux_queue.empty() )
    {
      OMX_ERRORTYPE omx_err;
      OMX_BUFFERHEADERTYPE* omx_buffer;

      demux_packet = m_demux_queue.front();
      m_demux_queue.pop();
      // need to lock here to retreve an input buffer and pop the queue
      pthread_mutex_lock(&m_omx_avaliable_mutex);
      omx_buffer = m_omx_input_avaliable.front();
      m_omx_input_avaliable.pop();
      pthread_mutex_unlock(&m_omx_avaliable_mutex);

      // delete the previous demuxer buffer
      delete [] omx_buffer->pBuffer;
      // setup a new omx_buffer.
      omx_buffer->nFlags  = m_omx_input_eos ? OMX_BUFFERFLAG_EOS : 0;
      omx_buffer->nOffset = 0;
      omx_buffer->pBuffer = demux_packet.buff;
      omx_buffer->nAllocLen  = demux_packet.size;
      omx_buffer->nFilledLen = demux_packet.size;
      omx_buffer->nTimeStamp = demux_packet.pts * 1000.0; // in microseconds;
      omx_buffer->pAppPrivate = this;
      omx_buffer->nInputPortIndex = m_omx_input_port;

      #if defined(OMX_DEBUG_EMPTYBUFFERDONE)
      CLog::Log(LOGDEBUG, "%s - feeding decoder, omx_buffer->pBuffer(0x%p), demuxer_bytes(%d)\n",
        __FUNCTION__, omx_buffer->pBuffer, demuxer_bytes);
      #endif
      // Give this omx_buffer to OpenMax to be decoded.
      omx_err = OMX_EmptyThisBuffer(m_omx_decoder, omx_buffer);
      if (omx_err)
      {
        CLog::Log(LOGDEBUG, "%s - OMX_EmptyThisBuffer() failed with result(0x%x)\n", __FUNCTION__, omx_err);
        return VC_ERROR;
      }
      // only push if we are successful with feeding OMX_EmptyThisBuffer
      m_dts_queue.push(demux_packet.dts);

      // if m_omx_input_avaliable and/or demux_queue are now empty,
      // wait up to 100ms for OpenMax to consume a demux packet
      if (m_omx_input_avaliable.empty() || m_demux_queue.empty())
        m_input_consumed_event.WaitMSec(100);
    }
    if (m_omx_input_avaliable.empty() && !m_demux_queue.empty())
      m_input_consumed_event.WaitMSec(100);

    #if defined(OMX_DEBUG_VERBOSE)
    if (m_omx_input_avaliable.empty())
      CLog::Log(LOGDEBUG, "%s - buffering demux, m_demux_queue_size(%d), demuxer_bytes(%d)\n",
        __FUNCTION__, m_demux_queue.size(), demuxer_bytes);
    #endif
  }

  if (m_omx_output_ready.empty())
    return VC_BUFFER;

  return VC_PICTURE | VC_BUFFER;
}

void CDVDVideoCodecOpenMax::Reset(void)
{
  // only reset OpenMax decoder if it's running
  if (m_omx_state == OMX_StateExecuting)
  {
    StopDecoder();
    while (!m_omx_input_avaliable.empty())
      m_omx_input_avaliable.pop();
    while (!m_omx_output_ready.empty())
      m_omx_output_ready.pop();
    while (!m_demux_queue.empty())
      m_demux_queue.pop();
    while (!m_dts_queue.empty())
      m_dts_queue.pop();
    
    StartDecoder();
  }
}

bool CDVDVideoCodecOpenMax::GetPicture(DVDVideoPicture* pDvdVideoPicture)
{
  if (!m_omx_output_ready.empty())
  {
    // fetch a output buffer and pop it off the ready list
    pthread_mutex_lock(&m_omx_ready_mutex);
    OMX_BUFFERHEADERTYPE *omx_buffer = m_omx_output_ready.front();
    m_omx_output_ready.pop();
    pthread_mutex_unlock(&m_omx_ready_mutex);

    bool done = omx_buffer->nFlags & OMX_BUFFERFLAG_EOS;
    if (!done && (omx_buffer->nFilledLen > 0))
    {
      m_videobuffer.dts = DVD_NOPTS_VALUE;
      m_videobuffer.pts = DVD_NOPTS_VALUE;
      if (!m_dts_queue.empty())
      {
        m_videobuffer.dts = m_dts_queue.front();
        m_dts_queue.pop();
      }
      // nTimeStamp is in microseconds
      m_videobuffer.pts = (double)omx_buffer->nTimeStamp / 1000.0;

      // not sure yet about yv12 buffer layout coming from OpenMax decoder
      OMX_U32 filled_size = (m_decoded_width * m_decoded_height) * FACTORFORMAT420;
      if (filled_size == omx_buffer->nFilledLen)
      {
        int luma_pixels = m_decoded_width * m_decoded_height;
        int chroma_pixels = luma_pixels/4;
        uint8_t *image_buffer = omx_buffer->pBuffer;
        memcpy(m_videobuffer.data[0], image_buffer, luma_pixels);
        image_buffer += luma_pixels;
        memcpy(m_videobuffer.data[1], image_buffer, chroma_pixels);
        image_buffer += chroma_pixels;
        memcpy(m_videobuffer.data[2], image_buffer, chroma_pixels);
      }
      #if defined(OMX_DEBUG_VERBOSE)
      else
      {
        CLog::Log(LOGDEBUG, "%s - nAllocLen(%lu), nFilledLen(%lu) should be filled_size(%lu)\n",
          __FUNCTION__, omx_buffer->nAllocLen, omx_buffer->nFilledLen, filled_size);
      }
      #endif

      // release the omx buffer back to OpenMax to fill.
      OMX_ERRORTYPE omx_err = OMX_FillThisBuffer(m_omx_decoder, omx_buffer);
      if (omx_err)
        CLog::Log(LOGERROR, "%s - OMX_FillThisBuffer, omx_err(0x%x)\n", __FUNCTION__, omx_err);
    }
    #if defined(OMX_DEBUG_VERBOSE)
    else
    {
      CLog::Log(LOGDEBUG, "%s - called but m_omx_output_ready is empty\n", __FUNCTION__);
    }
    #endif
  }

  // copy out our current video buffer and update it's flags.
  *pDvdVideoPicture = m_videobuffer;
  pDvdVideoPicture->iFlags |= m_drop_pictures ? DVP_FLAG_DROPPED : 0;

  return VC_PICTURE | VC_BUFFER;
}

////////////////////////////////////////////////////////////////////////////////////////////
bool CDVDVideoCodecOpenMax::bitstream_convert_init(void *in_extradata, int in_extrasize)
{
  // based on h264_mp4toannexb_bsf.c (ffmpeg)
  // which is Copyright (c) 2007 Benoit Fouet <benoit.fouet@free.fr>
  // and Licensed GPL 2.1 or greater

  m_sps_pps_size = 0;
  m_sps_pps_context.sps_pps_data = NULL;
  
  // nothing to filter
  if (!in_extradata || in_extrasize < 6)
    return false;

  uint16_t unit_size;
  uint32_t total_size = 0;
  uint8_t *out = NULL, unit_nb, sps_done = 0;
  const uint8_t *extradata = (uint8_t*)in_extradata + 4;
  static const uint8_t nalu_header[4] = {0, 0, 0, 1};

  // retrieve length coded size
  m_sps_pps_context.length_size = (*extradata++ & 0x3) + 1;
  if (m_sps_pps_context.length_size == 3)
    return false;

  // retrieve sps and pps unit(s)
  unit_nb = *extradata++ & 0x1f;  // number of sps unit(s)
  if (!unit_nb)
  {
    unit_nb = *extradata++;       // number of pps unit(s)
    sps_done++;
  }
  while (unit_nb--)
  {
    unit_size = extradata[0] << 8 | extradata[1];
    total_size += unit_size + 4;
    if ( (extradata + 2 + unit_size) > ((uint8_t*)in_extradata + in_extrasize) )
    {
      free(out);
      return false;
    }
    out = (uint8_t*)realloc(out, total_size);
    if (!out)
      return false;

    memcpy(out + total_size - unit_size - 4, nalu_header, 4);
    memcpy(out + total_size - unit_size, extradata + 2, unit_size);
    extradata += 2 + unit_size;

    if (!unit_nb && !sps_done++)
      unit_nb = *extradata++;     // number of pps unit(s)
  }

  m_sps_pps_context.sps_pps_data = out;
  m_sps_pps_context.size = total_size;
  m_sps_pps_context.first_idr = 1;

  return true;
}

bool CDVDVideoCodecOpenMax::bitstream_convert(BYTE* pData, int iSize, uint8_t **poutbuf, int *poutbuf_size)
{
  // based on h264_mp4toannexb_bsf.c (ffmpeg)
  // which is Copyright (c) 2007 Benoit Fouet <benoit.fouet@free.fr>
  // and Licensed GPL 2.1 or greater

  uint8_t *buf = pData;
  uint32_t buf_size = iSize;
  uint8_t  unit_type;
  int32_t  nal_size;
  uint32_t cumul_size = 0;
  const uint8_t *buf_end = buf + buf_size;

  do
  {
    if (buf + m_sps_pps_context.length_size > buf_end)
      goto fail;

    if (m_sps_pps_context.length_size == 1)
      nal_size = buf[0];
    else if (m_sps_pps_context.length_size == 2)
      nal_size = buf[0] << 8 | buf[1];
    else
      nal_size = buf[0] << 24 | buf[1] << 16 | buf[2] << 8 | buf[3];

    buf += m_sps_pps_context.length_size;
    unit_type = *buf & 0x1f;

    if (buf + nal_size > buf_end || nal_size < 0)
      goto fail;

    // prepend only to the first type 5 NAL unit of an IDR picture
    if (m_sps_pps_context.first_idr && unit_type == 5)
    {
      bitstream_alloc_and_copy(poutbuf, poutbuf_size,
        m_sps_pps_context.sps_pps_data, m_sps_pps_context.size, buf, nal_size);
      m_sps_pps_context.first_idr = 0;
    }
    else
    {
      bitstream_alloc_and_copy(poutbuf, poutbuf_size, NULL, 0, buf, nal_size);
      if (!m_sps_pps_context.first_idr && unit_type == 1)
          m_sps_pps_context.first_idr = 1;
    }

    buf += nal_size;
    cumul_size += nal_size + m_sps_pps_context.length_size;
  } while (cumul_size < buf_size);

  return true;

fail:
  free(*poutbuf);
  *poutbuf = NULL;
  *poutbuf_size = 0;
  return false;
}

void CDVDVideoCodecOpenMax::bitstream_alloc_and_copy(
  uint8_t **poutbuf,      int *poutbuf_size,
  const uint8_t *sps_pps, uint32_t sps_pps_size,
  const uint8_t *in,      uint32_t in_size)
{
  // based on h264_mp4toannexb_bsf.c (ffmpeg)
  // which is Copyright (c) 2007 Benoit Fouet <benoit.fouet@free.fr>
  // and Licensed GPL 2.1 or greater

  #define CHD_WB32(p, d) { \
    ((uint8_t*)(p))[3] = (d); \
    ((uint8_t*)(p))[2] = (d) >> 8; \
    ((uint8_t*)(p))[1] = (d) >> 16; \
    ((uint8_t*)(p))[0] = (d) >> 24; }

  uint32_t offset = *poutbuf_size;
  uint8_t nal_header_size = offset ? 3 : 4;

  *poutbuf_size += sps_pps_size + in_size + nal_header_size;
  *poutbuf = (uint8_t*)realloc(*poutbuf, *poutbuf_size);
  if (sps_pps)
    memcpy(*poutbuf + offset, sps_pps, sps_pps_size);

  memcpy(*poutbuf + sps_pps_size + nal_header_size + offset, in, in_size);
  if (!offset)
  {
    CHD_WB32(*poutbuf + sps_pps_size, 1);
  }
  else
  {
    (*poutbuf + offset + sps_pps_size)[0] = 0;
    (*poutbuf + offset + sps_pps_size)[1] = 0;
    (*poutbuf + offset + sps_pps_size)[2] = 1;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////
// DecoderEventHandler -- OMX event callback
OMX_ERRORTYPE CDVDVideoCodecOpenMax::DecoderEventHandler(
  OMX_HANDLETYPE hComponent,
  OMX_PTR pAppData,
  OMX_EVENTTYPE eEvent,
  OMX_U32 nData1,
  OMX_U32 nData2,
  OMX_PTR pEventData)
{
  OMX_ERRORTYPE omx_err;
  CDVDVideoCodecOpenMax *ctx = (CDVDVideoCodecOpenMax*)pAppData;

/*
  #if defined(OMX_DEBUG_VERBOSE)
  CLog::Log(LOGDEBUG, "%s - hComponent(0x%p), eEvent(0x%x), nData1(0x%lx), nData2(0x%lx), pEventData(0x%p)\n",
    __FUNCTION__, hComponent, eEvent, nData1, nData2, pEventData);
  #endif
*/

  switch (eEvent)
  {
    case OMX_EventCmdComplete:
      switch(nData1)
      {
        case OMX_CommandStateSet:
          ctx->m_omx_state = (int)nData2;
          sem_post(ctx->m_omx_state_change);
          #if defined(OMX_DEBUG_EVENTHANDLER)
          switch (ctx->m_omx_state)
          {
            case OMX_StateInvalid:
              CLog::Log(LOGDEBUG, "%s - OMX_StateInvalid\n", __FUNCTION__);
              break;
            case OMX_StateLoaded:
              CLog::Log(LOGDEBUG, "%s - OMX_StateLoaded\n", __FUNCTION__);
              break;
            case OMX_StateIdle:
              CLog::Log(LOGDEBUG, "%s - OMX_StateIdle\n", __FUNCTION__);
              break;
            case OMX_StateExecuting:
              CLog::Log(LOGDEBUG, "%s - OMX_StateExecuting\n", __FUNCTION__);
              break;
            case OMX_StatePause:
              CLog::Log(LOGDEBUG, "%s - OMX_StatePause\n", __FUNCTION__);
              break;
            case OMX_StateWaitForResources:
              CLog::Log(LOGDEBUG, "%s - OMX_StateWaitForResources\n", __FUNCTION__);
              break;
          }
          #endif
        break;
        case OMX_CommandFlush:
          /*
          if (OMX_ALL == (int)nData2)
          {
            sem_post(ctx->m_omx_flush_input);
            sem_post(ctx->m_omx_flush_output);
            CLog::Log(LOGDEBUG, "%s - OMX_CommandFlush input/output\n",__FUNCTION__);
          }
          else if (ctx->m_omx_input_port == (int)nData2)
          {
            sem_post(ctx->m_omx_flush_input);
            CLog::Log(LOGDEBUG, "%s - OMX_CommandFlush input\n",__FUNCTION__);
          }
          else if (ctx->m_omx_output_port == (int)nData2)
          {
            sem_post(ctx->m_omx_flush_output);
            CLog::Log(LOGDEBUG, "%s - OMX_CommandFlush ouput\n",__FUNCTION__);
          }
          else
          */
          {
            #if defined(OMX_DEBUG_EVENTHANDLER)
            CLog::Log(LOGDEBUG, "%s - OMX_CommandFlush, nData2(0x%lx)\n",__FUNCTION__, nData2);
            #endif
          }
        break;
        case OMX_CommandPortDisable:
          #if defined(OMX_DEBUG_EVENTHANDLER)
          CLog::Log(LOGDEBUG, "%s - OMX_CommandPortDisable, nData1(0x%lx), nData2(0x%lx)\n",
            __FUNCTION__, nData1, nData2);
          #endif
          if (ctx->m_omx_output_port == (int)nData2)
          {
            // Got OMX_CommandPortDisable event, alloc new buffers for the output port.
            omx_err = ctx->AllocOMXOutputBuffers();
            omx_err = OMX_SendCommand(ctx->m_omx_decoder, OMX_CommandPortEnable, ctx->m_omx_output_port, NULL);
          }
        break;
        case OMX_CommandPortEnable:
          #if defined(OMX_DEBUG_EVENTHANDLER)
          CLog::Log(LOGDEBUG, "%s - OMX_CommandPortEnable, nData1(0x%lx), nData2(0x%lx)\n",
            __FUNCTION__, nData1, nData2);
          #endif
          if (ctx->m_omx_output_port == (int)nData2)
          {
            // Got OMX_CommandPortEnable event.
            // OMX_CommandPortDisable will have re-alloced new ones so re-prime
            ctx->PrimeFillBuffers();
          }
        break;
        #if defined(OMX_DEBUG_EVENTHANDLER)
        case OMX_CommandMarkBuffer:
          CLog::Log(LOGDEBUG, "%s - OMX_CommandMarkBuffer, nData1(0x%lx), nData2(0x%lx)\n",
            __FUNCTION__, nData1, nData2);
        break;
        #endif
      }
    break;
    case OMX_EventBufferFlag:
      if (ctx->m_omx_decoder == hComponent && (nData2 & OMX_BUFFERFLAG_EOS)) {
        #if defined(OMX_DEBUG_EVENTHANDLER)
        if(ctx->m_omx_input_port  == (int)nData1)
            CLog::Log(LOGDEBUG, "%s - OMX_EventBufferFlag(input)\n" , __FUNCTION__);
        #endif
        if(ctx->m_omx_output_port == (int)nData1)
        {
            ctx->m_videoplayback_done = true;
            #if defined(OMX_DEBUG_EVENTHANDLER)
            CLog::Log(LOGDEBUG, "%s - OMX_EventBufferFlag(output)\n", __FUNCTION__);
            #endif
        }
      }
    break;
    case OMX_EventPortSettingsChanged:
      #if defined(OMX_DEBUG_EVENTHANDLER)
      CLog::Log(LOGDEBUG, "%s - OMX_EventPortSettingsChanged(output)\n", __FUNCTION__);
      #endif
      // not sure nData2 is the input/output ports in this call, docs don't say
      if (ctx->m_omx_output_port == (int)nData2)
      {
        // free the current OpenMax output buffers, you must do this before sending
        // OMX_CommandPortDisable to component as it expects output buffers
        // to be freed before it will issue a OMX_CommandPortDisable event.
        omx_err = ctx->FreeOMXOutputBuffers(false);
        omx_err = OMX_SendCommand(ctx->m_omx_decoder, OMX_CommandPortDisable, ctx->m_omx_output_port, NULL);
      }
    break;
    #if defined(OMX_DEBUG_EVENTHANDLER)
    case OMX_EventMark:
      CLog::Log(LOGDEBUG, "%s - OMX_EventMark\n", __FUNCTION__);
    break;
    case OMX_EventResourcesAcquired:
      CLog::Log(LOGDEBUG, "%s - OMX_EventResourcesAcquired\n", __FUNCTION__);
    break;
    #endif
    case OMX_EventError:
      switch((OMX_S32)nData1)
      {
        case OMX_ErrorInsufficientResources:
          CLog::Log(LOGERROR, "%s - OMX_EventError, insufficient resources\n", __FUNCTION__);
          // we are so frack'ed
          //exit(0);
        break;
        case OMX_ErrorFormatNotDetected:
          CLog::Log(LOGERROR, "%s - OMX_EventError, cannot parse input stream\n", __FUNCTION__);
        break;
        case OMX_ErrorPortUnpopulated:
          // silently ignore these. We can get them when setting OMX_CommandPortDisable
          // on the output port and the component flushes the output buffers.
        break;
        case OMX_ErrorStreamCorrupt:
          CLog::Log(LOGERROR, "%s - OMX_EventError, Bitstream corrupt\n", __FUNCTION__);
          ctx->m_videoplayback_done = true;
        break;
        default:
          CLog::Log(LOGERROR, "%s - OMX_EventError detected, nData1(0x%lx), nData2(0x%lx)\n",
            __FUNCTION__, nData1, nData2);
        break;
      }
      // do this so we don't hang on errors
      /*
      sem_post(ctx->m_omx_flush_input);
      sem_post(ctx->m_omx_flush_output);
      */
      sem_post(ctx->m_omx_state_change);
    break;
    default:
      CLog::Log(LOGWARNING, "%s - Unknown eEvent(0x%x), nData1(0x%lx), nData2(0x%lx)\n",
        __FUNCTION__, eEvent, nData1, nData2);
    break;
  }

  return OMX_ErrorNone;
}

// DecoderEmptyBufferDone -- OpenMax input buffer has been emptied
OMX_ERRORTYPE CDVDVideoCodecOpenMax::DecoderEmptyBufferDone(
  OMX_HANDLETYPE hComponent,
  OMX_PTR pAppData,
  OMX_BUFFERHEADERTYPE* pBuffer)
{
  CDVDVideoCodecOpenMax *ctx = (CDVDVideoCodecOpenMax*)pAppData;

  // queue free input buffer to avaliable list.
  pthread_mutex_lock(&ctx->m_omx_avaliable_mutex);
  ctx->m_omx_input_avaliable.push(pBuffer);
  ctx->m_input_consumed_event.Set();
  pthread_mutex_unlock(&ctx->m_omx_avaliable_mutex);

  return OMX_ErrorNone;
}

// DecoderFillBufferDone -- OpenMax output buffer has been filled
OMX_ERRORTYPE CDVDVideoCodecOpenMax::DecoderFillBufferDone(
  OMX_HANDLETYPE hComponent,
  OMX_PTR pAppData,
  OMX_BUFFERHEADERTYPE* pBufferHeader)
{
  #if defined(OMX_DEBUG_FILLBUFFERDONE)
  CLog::Log(LOGDEBUG, "%s - buffer_size(%lu), timestamp(%f)\n",
    __FUNCTION__, pBufferHeader->nFilledLen, (double)pBufferHeader->nTimeStamp / 1000.0);
  #endif
  CDVDVideoCodecOpenMax *ctx = (CDVDVideoCodecOpenMax*)pAppData;

  // queue output omx buffer to ready list.
  pthread_mutex_lock(&ctx->m_omx_ready_mutex);
  ctx->m_omx_output_ready.push(pBufferHeader);
  pthread_mutex_unlock(&ctx->m_omx_ready_mutex);

  return OMX_ErrorNone;
}

OMX_ERRORTYPE CDVDVideoCodecOpenMax::PrimeFillBuffers(void)
{
  OMX_ERRORTYPE omx_err = OMX_ErrorNone;
  OMX_BUFFERHEADERTYPE *buffer;

  // tell OpenMax to start filling output buffers
  for(size_t i = 0; i < m_omx_output_buffers.size(); i++)
  {
    buffer = m_omx_output_buffers[i];
    // always set the port index.
    buffer->nOutputPortIndex = m_omx_output_port;
    // Need to clear the EOS flag.
    buffer->nFlags &= ~OMX_BUFFERFLAG_EOS;
    buffer->pAppPrivate = this;

    omx_err = OMX_FillThisBuffer(m_omx_decoder, buffer);
    if (omx_err)
      CLog::Log(LOGERROR, "%s - OMX_FillThisBuffer failed with omx_err(0x%x)\n",
        __FUNCTION__, omx_err);
  }
  
  return omx_err;
}

OMX_ERRORTYPE CDVDVideoCodecOpenMax::AllocOMXInputBuffers(void)
{
  OMX_ERRORTYPE omx_err = OMX_ErrorNone;

  // Obtain the information about the decoder input port.
  OMX_PARAM_PORTDEFINITIONTYPE port_format;
  OMX_INIT_STRUCTURE(port_format);
  port_format.nPortIndex = m_omx_input_port;
  OMX_GetParameter(m_omx_decoder, OMX_IndexParamPortDefinition, &port_format);

  #if defined(OMX_DEBUG_VERBOSE)
  CLog::Log(LOGDEBUG, "%s - iport(%d), nBufferCountMin(%lu), nBufferSize(%lu)\n", 
    __FUNCTION__, m_omx_input_port, port_format.nBufferCountMin, port_format.nBufferSize);
  #endif
  for (size_t i = 0; i < port_format.nBufferCountMin; i++)
  {
    OMX_BUFFERHEADERTYPE *buffer = NULL;
    // use an external buffer that's sized according to actual demux
    // packet size, start at internal's buffer size, will get deleted when
    // we start pulling demuxer packets and using demux packet sized buffers.
    OMX_U8* data = new OMX_U8[port_format.nBufferSize];
    omx_err = OMX_UseBuffer(m_omx_decoder, &buffer, m_omx_input_port, NULL, port_format.nBufferSize, data);
    if (omx_err)
    {
      CLog::Log(LOGERROR, "%s - OMX_UseBuffer failed with omx_err(0x%x)\n", __FUNCTION__, omx_err);
      return(omx_err);
    }
    m_omx_input_buffers.push_back(buffer);
    // don't have to lock/unlock here, we are not decoding
    m_omx_input_avaliable.push(buffer);
  }
  m_omx_input_eos = false;

  return(omx_err);
}
OMX_ERRORTYPE CDVDVideoCodecOpenMax::FreeOMXInputBuffers(bool wait)
{
  OMX_ERRORTYPE omx_err = OMX_ErrorNone;

  /*
  omx_err = OMX_SendCommand(m_omx_decoder, OMX_CommandFlush, m_omx_input_port, 0);
  if (omx_err)
    CLog::Log(LOGERROR, "%s - OMX_CommandFlush failed with omx_err(0x%x)\n",
      __FUNCTION__, omx_err);
  else if (wait)
    sem_wait(m_omx_flush_input);
  */

  // free omx input port buffers.
  for (size_t i = 0; i < m_omx_input_buffers.size(); i++)
  {
    // using external buffers (OMX_UseBuffer), free our external buffers
    //  before calling OMX_FreeBuffer which frees the omx buffer.
    delete [] m_omx_input_buffers[i]->pBuffer;
    m_omx_input_buffers[i]->pBuffer = NULL;
    omx_err = OMX_FreeBuffer(m_omx_decoder, m_omx_input_port, m_omx_input_buffers[i]);
  }
  m_omx_input_buffers.clear();

  // empty input buffer queue. not decoding so don't need lock/unlock.
  while (!m_omx_input_avaliable.empty())
    m_omx_input_avaliable.pop();
  while (!m_demux_queue.empty())
    m_demux_queue.pop();
  while (!m_dts_queue.empty())
    m_dts_queue.pop();

  return(omx_err);
}

OMX_ERRORTYPE CDVDVideoCodecOpenMax::AllocOMXOutputBuffers(void)
{
  OMX_ERRORTYPE omx_err = OMX_ErrorNone;
  int buffer_size;

  // Obtain the information about the output port.
  OMX_PARAM_PORTDEFINITIONTYPE port_format;
  OMX_INIT_STRUCTURE(port_format);
  port_format.nPortIndex = m_omx_output_port;
  omx_err = OMX_GetParameter(m_omx_decoder, OMX_IndexParamPortDefinition, &port_format);

  #if defined(OMX_DEBUG_VERBOSE)
  CLog::Log(LOGDEBUG, 
    "%s (1) - oport(%d), nFrameWidth(%lu), nFrameHeight(%lu), nStride(%lx), nBufferCountMin(%lu), nBufferSize(%lu)\n",
    __FUNCTION__, m_omx_output_port, 
    port_format.format.video.nFrameWidth, port_format.format.video.nFrameHeight,port_format.format.video.nStride,
    port_format.nBufferCountMin, port_format.nBufferSize);
  #endif

  buffer_size = m_decoded_width * m_decoded_height;
  if (port_format.format.video.eColorFormat == OMX_COLOR_FormatCbYCrY)
    buffer_size *= FACTORFORMAT422;
  else if (port_format.format.video.eColorFormat == OMX_COLOR_FormatYUV420Planar)
    buffer_size *= FACTORFORMAT420;
 
  for (size_t i = 0; i < port_format.nBufferCountMin; i++)
  {
    OMX_BUFFERHEADERTYPE *buffer = NULL;
    omx_err = OMX_AllocateBuffer(m_omx_decoder, &buffer, m_omx_output_port, NULL, buffer_size);
    if (omx_err)
    {
      CLog::Log(LOGERROR, "%s - OMX_AllocateBuffer failed with omx_err(0x%x)\n", __FUNCTION__, omx_err);
      return(omx_err);
    }
    m_omx_output_buffers.push_back(buffer);
  }
  m_omx_output_eos = false;

  return(omx_err);
}

OMX_ERRORTYPE CDVDVideoCodecOpenMax::FreeOMXOutputBuffers(bool wait)
{
  OMX_ERRORTYPE omx_err = OMX_ErrorNone;

  /*
  omx_err = OMX_SendCommand(m_omx_decoder, OMX_CommandFlush, m_omx_output_port, 0);
  if (omx_err)
    CLog::Log(LOGERROR, "%s - OMX_CommandFlush failed with omx_err(0x%x)\n",
      __FUNCTION__, omx_err);
  else if (wait)
    sem_wait(m_omx_flush_output);
  */

  // free omx output port buffers.
  for (size_t i = 0; i < m_omx_output_buffers.size(); i++)
    omx_err = OMX_FreeBuffer(m_omx_decoder, m_omx_output_port, m_omx_output_buffers[i]);
  m_omx_output_buffers.clear();

  // empty available output buffer queue. not decoding so don't need lock/unlock.
  while (!m_omx_output_ready.empty())
    m_omx_output_ready.pop();

  return(omx_err);
}

OMX_ERRORTYPE CDVDVideoCodecOpenMax::AllocOMXOutputEGLTextures(void)
{
  OMX_ERRORTYPE omx_err;
  EGLint attrib = EGL_NONE;
  omx_egl_buffer *egl_buffer;

  // Obtain the information about the output port.
  OMX_PARAM_PORTDEFINITIONTYPE port_format;
  OMX_INIT_STRUCTURE(port_format);
  port_format.nPortIndex = m_omx_output_port;
  omx_err = OMX_GetParameter(m_omx_decoder, OMX_IndexParamPortDefinition, &port_format);

  #if defined(OMX_DEBUG_VERBOSE)
  CLog::Log(LOGDEBUG, 
    "%s (1) - oport(%d), nFrameWidth(%lu), nFrameHeight(%lu), nStride(%lx), nBufferCountMin(%lu), nBufferSize(%lu)\n",
    __FUNCTION__, m_omx_output_port, 
    port_format.format.video.nFrameWidth, port_format.format.video.nFrameHeight,port_format.format.video.nStride,
    port_format.nBufferCountMin, port_format.nBufferSize);
  #endif

  glActiveTexture(GL_TEXTURE0);

  for (size_t i = 0; i < port_format.nBufferCountMin; i++)
  {
    egl_buffer = new omx_egl_buffer;
    memset(egl_buffer, 0, sizeof(*egl_buffer));
    egl_buffer->width  = m_decoded_width;
    egl_buffer->height = m_decoded_height;

    glGenTextures(1, &egl_buffer->texture_id);
    glBindTexture(GL_TEXTURE_2D, egl_buffer->texture_id);

    // create space for buffer with a texture
    glTexImage2D(
      GL_TEXTURE_2D,      // target
      0,                  // level
      GL_RGBA,            // internal format
      m_decoded_width,    // width
      m_decoded_height,   // height
      0,                  // border
      GL_RGBA,            // format
      GL_UNSIGNED_BYTE,   // type
      NULL);              // pixels -- will be provided later
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // create EGLImage from texture
    egl_buffer->egl_image = eglCreateImageKHR(
      m_egl_display,
      m_egl_context,
      EGL_GL_TEXTURE_2D_KHR,
      (EGLClientBuffer)(egl_buffer->texture_id),
      &attrib);
    if (!egl_buffer->egl_image)
    {
      CLog::Log(LOGERROR, "%s - ERROR creating EglImage\n", __FUNCTION__);
      return(OMX_ErrorUndefined);
    }
    egl_buffer->index = i;

    // tell decoder output port that it will be using EGLImage
    omx_err = OMX_UseEGLImage(
      m_omx_decoder, &egl_buffer->omx_buffer, m_omx_output_port, this, egl_buffer->egl_image);
    if (omx_err)
    {
      CLog::Log(LOGERROR, "%s - OMX_UseEGLImage failed with omx_err(0x%x)\n", __FUNCTION__, omx_err);
      return(omx_err);
    }
    m_omx_egl_output_buffers.push_back(egl_buffer);

    CLog::Log(LOGDEBUG, "%s - Texture %p Width %d Height %d\n",
      __FUNCTION__, egl_buffer->egl_image, egl_buffer->width, egl_buffer->height);
  }
  m_omx_output_eos = false;
  while (!m_omx_egl_output_ready.empty())
    m_omx_egl_output_ready.pop();

  return omx_err;
}

OMX_ERRORTYPE CDVDVideoCodecOpenMax::FreeOMXOutputEGLTextures(bool wait)
{
  OMX_ERRORTYPE omx_err = OMX_ErrorNone;
  omx_egl_buffer *egl_buffer;

  for (size_t i = 0; i < m_omx_egl_output_buffers.size(); i++)
  {
    egl_buffer = m_omx_egl_output_buffers[i];

    // tell decoder output port to stop using the EGLImage
    omx_err = OMX_FreeBuffer(m_omx_decoder, m_omx_output_port, egl_buffer->omx_buffer);
    // destroy egl_image
    eglDestroyImageKHR(m_egl_display, egl_buffer->egl_image);
    // free texture
    glDeleteTextures(1, &egl_buffer->texture_id);
    delete egl_buffer;
  }
  m_omx_egl_output_buffers.clear();

  return omx_err;
}

// SetStateForAllComponents
// Blocks until all state changes have completed
OMX_ERRORTYPE CDVDVideoCodecOpenMax::SetStateForComponent(OMX_STATETYPE state)
{
  OMX_ERRORTYPE omx_err;

  omx_err = OMX_SendCommand(m_omx_decoder, OMX_CommandStateSet, state, 0);
  if (omx_err)
    CLog::Log(LOGERROR, "%s - OMX_CommandStateSet failed with omx_err(0x%x)\n",
      __FUNCTION__, omx_err);
  else
    sem_wait(m_omx_state_change);  

  return omx_err;
}

// StartPlayback -- Kick off video playback.
OMX_ERRORTYPE CDVDVideoCodecOpenMax::StartDecoder(void)
{
  OMX_ERRORTYPE omx_err;

  // transition decoder component to IDLE state
  omx_err = SetStateForComponent(OMX_StateIdle);
  if (omx_err)
  {
    CLog::Log(LOGERROR, "%s - setting OMX_StateIdle failed with omx_err(0x%x)\n",
      __FUNCTION__, omx_err);
    return omx_err;
  }

  // transition decoder component to executing state
  omx_err = SetStateForComponent(OMX_StateExecuting);
  if (omx_err)
  {
    CLog::Log(LOGERROR, "%s - setting OMX_StateExecuting failed with omx_err(0x%x)\n",
      __FUNCTION__, omx_err);
    return omx_err;
  }

  //prime the omx output buffers.
  PrimeFillBuffers();

  return omx_err;
}

// StopPlayback -- Stop video playback
OMX_ERRORTYPE CDVDVideoCodecOpenMax::StopDecoder(void)
{
  OMX_ERRORTYPE omx_err;

  // transition decoder component from executing to idle
  omx_err = SetStateForComponent(OMX_StateIdle);
  if (omx_err)
  {
    CLog::Log(LOGERROR, "%s - setting OMX_StateIdle failed with omx_err(0x%x)\n",
      __FUNCTION__, omx_err);
    return omx_err;
  }

  // transition decoder component from idle to loaded
  omx_err = SetStateForComponent(OMX_StateLoaded);
  if (omx_err)
    CLog::Log(LOGERROR, "%s - setting OMX_StateLoaded failed with omx_err(0x%x)\n",
      __FUNCTION__, omx_err);

  // we can free port buffers in OMX_StateLoaded state.
  // free OpenMax input buffers.
  omx_err = FreeOMXInputBuffers(true);
  // free OpenMax output buffers.
  omx_err = FreeOMXOutputBuffers(true);

  return omx_err;
}

#endif
