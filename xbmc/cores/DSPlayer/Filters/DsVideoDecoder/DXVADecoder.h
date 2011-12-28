/* 
 *  Copyright (C) 2003-2006 Gabest
 *  http://www.gabest.org
 *
 *  Copyright (C) 2005-2010 Team XBMC
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

#pragma once

#include "DShowUtil/DShowUtil.h"
#include <dxva2api.h>
#include "streams.h"
#include <videoacc.h>
#include "Codecs/DllAvCodec.h"



namespace DIRECTSHOW {

enum PCI_Vendors
{
	PCIV_ATI				= 0x1002,
	PCIV_nVidia				= 0x10DE,
	PCIV_Intel				= 0x8086,
	PCIV_S3_Graphics		= 0x5333
};

// Bitmasks for DXVA compatibility check
#define DXVA_UNSUPPORTED_LEVEL		1
#define DXVA_TOO_MUCH_REF_FRAMES	2
#define DXVA_INCOMPATIBLE_SAR		4

typedef enum
{
	ENGINE_DXVA1,
	ENGINE_DXVA2
} DXVA_ENGINE;

/*typedef struct
{
	bool						bRefPicture;	// True if reference picture
	int							bInUse;			// Slot in use
	bool						bDisplayed;		// True if picture have been presented
	Com::SmartPtr<IMediaSample>		pSample;		// Only for DXVA2 !
	REFERENCE_TIME				rtStart;
	REFERENCE_TIME				rtStop;
	FF_FIELD_TYPE				n1FieldType;	// Top or bottom for the 1st field
	FF_SLICE_TYPE				nSliceType;
	int							nCodecSpecific;
	DWORD						dwDisplayCount;
} PICTURE_STORE;*/

#define MAX_COM_BUFFER				6		// Max uncompressed buffer for an Execute command (DXVA1)
#define COMP_BUFFER_COUNT			18
#define NO_REF_FRAME			0xFFFF


class CXBMCVideoDecFilter;
class CDXVADecoder
  : public CXBMCVideoDecFilter::IDXVADecoder
{
public :
	// === Public functions
	virtual				   ~CDXVADecoder();
	DXVAMode				GetMode()		const { return m_nMode; };
	DXVA_ENGINE				GetEngine()		const { return m_nEngine; };
	void					  AllocExecuteParams (int nSize);
	void					  SetDirectXVideoDec (IDirectXVideoDecoder* pDirectXVideoDec)  { m_pDirectXVideoDec = pDirectXVideoDec; };

	virtual HRESULT	  DecodeFrame  (BYTE* pDataIn, UINT nSize, REFERENCE_TIME rtStart, REFERENCE_TIME rtStop) = NULL;
	virtual void			SetExtraData (BYTE* pDataIn, UINT nSize);
	virtual void			CopyBitstream(BYTE* pDXVABuffer, BYTE* pBuffer, UINT& nSize);
	virtual void			Flush();
	bool ConfigureDXVA1();
	
	static CDXVADecoder*	CreateDecoder (CXBMCVideoDecFilter* pFilter, IAMVideoAccelerator*  pAMVideoAccelerator, const GUID* guidDecoder, int nPicEntryNumber);
	static CDXVADecoder*	CreateDecoder (CXBMCVideoDecFilter* pFilter, IDirectXVideoDecoder* pDirectXVideoDec, const GUID* guidDecoder, int nPicEntryNumber, DXVA2_ConfigPictureDecode* pDXVA2Config);
  /* ffmpeg callbacks*/
  int   GetBuffer(AVCodecContext *avctx, AVFrame *pic);
  void  RelBuffer(AVCodecContext *avctx, AVFrame *pic);
  /* AVHWAccel dxvadecoder callbacks*/


protected :
	CDXVADecoder (CXBMCVideoDecFilter* pFilter, IAMVideoAccelerator*  pAMVideoAccelerator, DXVAMode nMode, int nPicEntryNumber);
	CDXVADecoder (CXBMCVideoDecFilter* pFilter, IDirectXVideoDecoder* pDirectXVideoDec, DXVAMode nMode, int nPicEntryNumber, DXVA2_ConfigPictureDecode* pDXVA2Config);

	CXBMCVideoDecFilter* m_pFilter;
	bool							m_bFlushed;
	int								m_nMaxWaiting;

  std::vector<PICTURE_STORE> m_pPictureStore;		// Store reference picture, and delayed B-frames
	int								m_nPicEntryNumber;		// Total number of picture in store
	int								m_nWaitingPics;			// Number of picture not yet displayed

	// === DXVA functions
	HRESULT						AddExecuteBuffer (DWORD CompressedBufferType, UINT nSize, void* pBuffer, UINT* pRealSize = NULL);
	HRESULT						GetDeliveryBuffer(REFERENCE_TIME rtStart, REFERENCE_TIME rtStop, IMediaSample** ppSampleToDeliver);
	HRESULT						Execute();
	DWORD						GetDXVA1CompressedType (DWORD dwDXVA2CompressedType);
	HRESULT						FindFreeDXVA1Buffer(DWORD dwTypeIndex, DWORD& dwBufferIndex);
	HRESULT						BeginFrame(int nSurfaceIndex, IMediaSample* pSampleToDeliver);
	HRESULT						EndFrame(int nSurfaceIndex);
	HRESULT						QueryStatus(PVOID LPDXVAStatus, UINT nSize);
	BYTE						GetConfigIntraResidUnsigned();
	BYTE						GetConfigResidDiffAccelerator();
	DXVA_ConfigPictureDecode*	GetDXVA1Config() { return &m_DXVA1Config; };
	DXVA2_ConfigPictureDecode*	GetDXVA2Config() { return &m_DXVA2Config; };

	// === Picture store functions
	bool					AddToStore (int nSurfaceIndex, IMediaSample* pSample, bool bRefPicture, REFERENCE_TIME rtStart, REFERENCE_TIME rtStop, bool bIsField, FF_FIELD_TYPE nFieldType, FF_SLICE_TYPE nSliceType, int nCodecSpecific);
	void					UpdateStore (int nSurfaceIndex, REFERENCE_TIME rtStart, REFERENCE_TIME rtStop);
	void					RemoveRefFrame (int nSurfaceIndex);
	HRESULT					DisplayNextFrame();
	HRESULT					GetFreeSurfaceIndex(int& nSurfaceIndex, IMediaSample** ppSampleToDeliver, REFERENCE_TIME rtStart, REFERENCE_TIME rtStop);
	virtual int				FindOldestFrame();

private :
	DXVAMode						m_nMode;
	DXVA_ENGINE						m_nEngine;

	Com::SmartPtr<IMediaSample>			m_pFieldSample;
	int								m_nFieldSurface;

	// === DXVA1 variables
	Com::SmartQIPtr<IAMVideoAccelerator>	m_pAMVideoAccelerator;
	AMVABUFFERINFO					m_DXVA1BufferInfo[MAX_COM_BUFFER];
	DXVA_BufferDescription 			m_DXVA1BufferDesc[MAX_COM_BUFFER];
	DWORD							m_dwNumBuffersInfo;
	DXVA_ConfigPictureDecode		m_DXVA1Config;
	AMVACompBufferInfo				m_ComBufferInfo[COMP_BUFFER_COUNT];
	DWORD							m_dwBufferIndex;
	DWORD							m_dwDisplayCount;

	// === DXVA2 variables
	Com::SmartPtr<IDirectXVideoDecoder>	m_pDirectXVideoDec;
	DXVA2_ConfigPictureDecode		m_DXVA2Config;
	DXVA2_DecodeExecuteParams		m_ExecuteParams;

	void					Init(CXBMCVideoDecFilter* pFilter, DXVAMode nMode, int nPicEntryNumber);
	void					FreePictureSlot (int nSurfaceIndex);
	void					SetTypeSpecificFlags(PICTURE_STORE* pPicture, IMediaSample* pMS);
};

};