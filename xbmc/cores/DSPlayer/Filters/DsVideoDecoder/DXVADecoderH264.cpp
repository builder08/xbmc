/* 
 *  Copyright (C) 2003-2009 Gabest
 *  http://www.gabest.org
 *
 *  Copyright (C) 2010 Team XBMC
 *  http://www.xbmc.org
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
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. 
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#ifdef HAS_DS_PLAYER

#include "dshowutil/dshowutil.h"
#include "DXVADecoderH264.h"
#include "XBMCVideoDecFilter.h"
#include "VideoDecDXVAAllocator.h"
#include "PODtypes.h"
#include "libavcodec/avcodec.h"


CDXVADecoderH264::CDXVADecoderH264 (CXBMCVideoDecFilter* pFilter, IAMVideoAccelerator*  pAMVideoAccelerator, DXVAMode nMode, int nPicEntryNumber)
        : CDXVADecoder (pFilter, pAMVideoAccelerator, nMode, nPicEntryNumber)
{
  m_bUseLongSlice = (GetDXVA1Config()->bConfigBitstreamRaw != 2);
  Init();
}

CDXVADecoderH264::CDXVADecoderH264 (CXBMCVideoDecFilter* pFilter, IDirectXVideoDecoder* pDirectXVideoDec, DXVAMode nMode, int nPicEntryNumber, DXVA2_ConfigPictureDecode* pDXVA2Config)
        : CDXVADecoder (pFilter, pDirectXVideoDec, nMode, nPicEntryNumber, pDXVA2Config)
{
  m_bUseLongSlice = (m_pFilter->GetDXVA2Config()->ConfigBitstreamRaw != 2);
  Init();
}

CDXVADecoderH264::~CDXVADecoderH264()
{
}

void CDXVADecoderH264::Init()
{
  m_pFilter->m_dllAvCodec.Load();
  memset (&m_DXVAPicParams,  0, sizeof (m_DXVAPicParams));
  memset (&m_DXVAPicParams,   0, sizeof (DXVA_PicParams_H264));
  memset (&m_pSliceLong,      0, sizeof (DXVA_Slice_H264_Long) *MAX_SLICES);
  memset (&m_pSliceShort,      0, sizeof (DXVA_Slice_H264_Short)*MAX_SLICES);
  
  m_DXVAPicParams.MbsConsecutiveFlag = 1;
  
  if(m_pFilter->GetPCIVendor() == PCIV_Intel) 
    m_DXVAPicParams.Reserved16Bits          = 0x534c;
  else
    m_DXVAPicParams.Reserved16Bits          = 0;
	
  m_DXVAPicParams.ContinuationFlag          = 1;
  m_DXVAPicParams.Reserved8BitsA            = 0;
  m_DXVAPicParams.Reserved8BitsB            = 0;
  m_DXVAPicParams.MinLumaBipredSize8x8Flag      = 1;  // Improve accelerator performances
  m_DXVAPicParams.StatusReportFeedbackNumber      = 0;  // Use to report status

  for (int i =0; i<16; i++)
  {
    m_DXVAPicParams.RefFrameList[i].AssociatedFlag  = 1;
    m_DXVAPicParams.RefFrameList[i].bPicEntry    = 255;
    m_DXVAPicParams.RefFrameList[i].Index7Bits    = 127;
  }


  m_nNALLength    = 4;
  m_nMaxSlices    = 0;

  switch (GetMode())
  {
    case H264_VLD :
      AllocExecuteParams (3);
      break;
    default :
      ASSERT(FALSE);
  }
}


void CDXVADecoderH264::CopyBitstream(BYTE* pDXVABuffer, BYTE* pBuffer, UINT& nSize)
{
  CH264Nalu    Nalu;
  int        nDummy;
  int        nSlices = 0;
  int        nDxvaNalLength;

  Nalu.SetBuffer (pBuffer, nSize, m_nNALLength);
  nSize = 0;
  while (Nalu.ReadNext())
  {
    switch (Nalu.GetType())
    {
      case NALU_TYPE_SLICE:
      case NALU_TYPE_IDR:
      // For AVC1, put startcode 0x000001
      pDXVABuffer[0]=pDXVABuffer[1]=0;pDXVABuffer[2]=1;
      
      // Copy NALU
      memcpy (pDXVABuffer+3, Nalu.GetDataBuffer(), Nalu.GetDataLength());
      
      // Update slice control buffer
      nDxvaNalLength          = Nalu.GetDataLength()+3;
      m_pSliceShort[nSlices].BSNALunitDataLocation  = nSize;
      m_pSliceShort[nSlices].SliceBytesInBuffer  = nDxvaNalLength;

      nSize += nDxvaNalLength;
      pDXVABuffer += nDxvaNalLength;
      nSlices++;
      break;
    }
  }

  // Complete with zero padding (buffer size should be a multiple of 128)
  nDummy  = 128 - (nSize %128);

  memset (pDXVABuffer, 0, nDummy);
  m_pSliceShort[nSlices-1].SliceBytesInBuffer  += nDummy;
  nSize            += nDummy;
  }


void CDXVADecoderH264::Flush()
{
  ClearRefFramesList();
  m_DXVAPicParams.UsedForReferenceFlags  = 0;
  m_nOutPOC                = -1;
  m_rtLastFrameDisplayed          = 0;

  __super::Flush();
}

HRESULT CDXVADecoderH264::DecodeFrame (BYTE* pDataIn, UINT nSize, REFERENCE_TIME rtStart, REFERENCE_TIME rtStop)
{

  HRESULT            hr      = S_FALSE;
  CH264Nalu          Nalu;
  UINT            nSlices    = 0;
  /*int              nSurfaceIndex;*/

  Com::SmartPtr<IMediaSample>    pSampleToDeliver;
  Com::SmartQIPtr<IMPCDXVA2Sample>  pDXVA2Sample;
  int              nDXIndex  = 0;
  UINT            nNalOffset  = 0;
  int              nOutPOC;
  REFERENCE_TIME        rtOutStart;
  int iGotPicture = 0;
  Nalu.SetBuffer (pDataIn, nSize, m_nNALLength);

  int usedbyte;
  
  
  usedbyte = m_pFilter->m_dllAvCodec.avcodec_decode_video(m_pFilter->GetAVCtx(), m_pFilter->GetFrame(), &iGotPicture, pDataIn, nSize);
  while (Nalu.ReadNext())
  {
    switch (Nalu.GetType())
    {
    case NALU_TYPE_SLICE:
    case NALU_TYPE_IDR:
        if(m_bUseLongSlice)
        {
          m_pSliceLong[nSlices].BSNALunitDataLocation  = nNalOffset;
          m_pSliceLong[nSlices].SliceBytesInBuffer  = Nalu.GetDataLength()+3;
          m_pSliceLong[nSlices].slice_id        = nSlices;

          if (nSlices>0)
            m_pSliceLong[nSlices-1].NumMbsForSlice = m_pSliceLong[nSlices].NumMbsForSlice = m_pSliceLong[nSlices].first_mb_in_slice - m_pSliceLong[nSlices-1].first_mb_in_slice;
        }
        nSlices++; 
        nNalOffset += (UINT)(Nalu.GetDataLength() + 3);
        if (nSlices > MAX_SLICES) 
          break;
        break;
    }
  }
  if (nSlices == 0) 
    return S_FALSE;
  directshow_dxva_h264* dxvapicture;
  
  dxvapicture = (directshow_dxva_h264*)m_pFilter->GetAVCtx()->coded_frame->data[0];
  
  m_DXVAPicParams = dxvapicture->picture_params;
  *m_pSliceLong = *dxvapicture->slice_long;
  m_DXVAScalingMatrix = dxvapicture->picture_qmatrix;
  m_nMaxWaiting  = std::min ( std::max (m_DXVAPicParams.num_ref_frames, (UCHAR) 3), (UCHAR) 8);

  // If parsing fail (probably no PPS/SPS), continue anyway it may arrived later (happen on truncated streams)
  /*if (FAILED (m_pFilter->m_dllAvCodec.FFH264BuildPicParams (&m_DXVAPicParams, &m_DXVAScalingMatrix, &nFieldType, &nSliceType, m_pFilter->GetAVCtx(), m_pFilter->GetPCIVendor())))
    return S_FALSE;*/

  // Wait I frame after a flush
  if (m_bFlushed && !m_DXVAPicParams.IntraPicFlag)
    return S_FALSE;

  
  /*CHECK_HR (GetFreeSurfaceIndex (dxvapicture->decoder_surface_index, &pSampleToDeliver, rtStart, rtStop));
  /*m_pFilter->m_dllAvCodec.FFH264SetCurrentPicture (nSurfaceIndex, &m_DXVAPicParams, m_pFilter->GetAVCtx());*/
  

  CHECK_HR (BeginFrame(dxvapicture->decoder_surface_index, pSampleToDeliver));
  
  m_DXVAPicParams.StatusReportFeedbackNumber++;

  //CLog::DebugLog("CDXVADecoderH264 : Decode frame %u", m_DXVAPicParams.StatusReportFeedbackNumber);

  // Send picture parameters
  CHECK_HR (AddExecuteBuffer (DXVA2_PictureParametersBufferType, sizeof(m_DXVAPicParams), &m_DXVAPicParams));
  CHECK_HR (Execute());

  // Add bitstream, slice control and quantization matrix
  CHECK_HR (AddExecuteBuffer (DXVA2_BitStreamDateBufferType, nSize, pDataIn, &nSize));

  if (m_bUseLongSlice)
  {
    CHECK_HR(AddExecuteBuffer(DXVA2_SliceControlBufferType,  sizeof(DXVA_Slice_H264_Long)*nSlices, m_pSliceLong));
  }
  else
  {
    /*verify that we dont slow down the process by filling the slicelong when we use the short*/
    CHECK_HR (AddExecuteBuffer (DXVA2_SliceControlBufferType, sizeof (DXVA_Slice_H264_Short)*nSlices, m_pSliceLong));
  }

  CHECK_HR (AddExecuteBuffer (DXVA2_InverseQuantizationMatrixBufferType, sizeof (DXVA_Qmatrix_H264), (void*)&m_DXVAScalingMatrix));

  // Decode bitstream
  CHECK_HR (Execute());

  CHECK_HR (EndFrame(dxvapicture->decoder_surface_index));

#ifdef _DEBUG
//  DisplayStatus();
#endif

  bool bAdded    = AddToStore (dxvapicture->decoder_surface_index, pSampleToDeliver, m_DXVAPicParams.RefPicFlag, rtStart, rtStop,
                  m_DXVAPicParams.field_pic_flag, (FF_FIELD_TYPE)dxvapicture->field_type, 
                  (FF_SLICE_TYPE)dxvapicture->slice_type, dxvapicture->frame_poc);

  /*m_pFilter->m_dllAvCodec.FFH264UpdateRefFramesList (&m_DXVAPicParams, m_pFilter->GetAVCtx());*/
  ClearUnusedRefFrames();


  if (bAdded) 
  {
    hr        = DisplayNextFrame();

    if (dxvapicture->frame_poc != INT_MIN)
    {
      m_nOutPOC    = dxvapicture->frame_poc;
      m_rtOutStart  = dxvapicture->frame_start;
    }
  }
  m_bFlushed    = false;
done:
  return hr;
}

void CDXVADecoderH264::RemoveUndisplayedFrame(int nPOC)
{
  // Find frame with given POC, and free the slot
  for (int i=0; i<m_nPicEntryNumber; i++)
  {
    if (m_pPictureStore[i].bInUse && m_pPictureStore[i].nCodecSpecific == nPOC)
    {
      m_pPictureStore[i].bDisplayed = true;
      RemoveRefFrame (i);
      return;
    }
  }
}

void CDXVADecoderH264::ClearUnusedRefFrames()
{
  // Remove old reference frames (not anymore a short or long ref frame)
  /*for (int i=0; i<m_nPicEntryNumber; i++)
  {
    if (m_pPictureStore[i].bRefPicture && m_pPictureStore[i].bDisplayed)
      if (!m_pFilter->GetHardware()->RefFrameInUse(i))
        RemoveRefFrame (i);
  }*/
}

void CDXVADecoderH264::SetExtraData (BYTE* pDataIn, UINT nSize)
{
  AVCodecContext*    pAVCtx = m_pFilter->GetAVCtx();

  m_nNALLength  =4;//pAVCtx->nal_length_size;
  //m_nNALLength the nal length size should be 1 2 or 4
  /*m_pFilter->m_dllAvCodec.FFH264DecodeBuffer (pAVCtx, pDataIn, nSize, NULL, NULL, NULL);*/
  /*m_pFilter->m_dllAvCodec.FFH264SetDxvaSliceLong (pAVCtx, m_pSliceLong);*/
}


void CDXVADecoderH264::ClearRefFramesList()
{
  m_pFilter->GetHardware()->Flush();
  /*for (int i=0; i<m_nPicEntryNumber; i++)
  {
    if (m_pPictureStore[i].bInUse)
    {
      m_pPictureStore[i].bDisplayed = true;
      RemoveRefFrame (i);
    }
  }*/
}


HRESULT CDXVADecoderH264::DisplayStatus()
{
  HRESULT       hr = E_INVALIDARG;
  DXVA_Status_H264   Status;

  memset (&Status, 0, sizeof(Status));

  CHECK_HR (hr = CDXVADecoder::QueryStatus(&Status, sizeof(Status)));

  /*TRACE ("CDXVADecoderH264 : Status for the frame %u : bBufType = %u, bStatus = %u, wNumMbsAffected = %u\n", 
    Status.StatusReportFeedbackNumber,
    Status.bBufType,
    Status.bStatus,
    Status.wNumMbsAffected);*/
done:
  return hr;
}


int CDXVADecoderH264::FindOldestFrame()
{
  int        nPos  = -1;
  REFERENCE_TIME  rtPos = _I64_MAX;

  for (int i=0; i<m_nPicEntryNumber; i++)
  {
    if (m_pPictureStore[i].bInUse && !m_pPictureStore[i].bDisplayed)
    {
      if (m_pPictureStore[i].nCodecSpecific == m_nOutPOC && m_pPictureStore[i].rtStart < rtPos)
      {
        nPos  = i;
        rtPos = m_pPictureStore[i].rtStart;
      }
    }
  }

  if (nPos != -1)
  {
    if (m_rtOutStart == _I64_MIN)
    {
      // If start time not set (no PTS for example), guess presentation time!
      m_rtOutStart = m_rtLastFrameDisplayed;
    }
    m_pPictureStore[nPos].rtStart  = m_rtOutStart;
    m_pPictureStore[nPos].rtStop  = m_rtOutStart + m_pFilter->GetAvrTimePerFrame();
    m_rtLastFrameDisplayed      = m_rtOutStart + m_pFilter->GetAvrTimePerFrame();
    m_pFilter->ReorderBFrames (m_pPictureStore[nPos].rtStart, m_pPictureStore[nPos].rtStop);
  }

  return nPos;
}

#endif