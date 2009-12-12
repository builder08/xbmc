/*
 *
 * (C) 2003-2006 Gabest
 * (C) 2006-2007 see AUTHORS
 *
 * This file is part of mplayerc.
 *
 * Mplayerc is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Mplayerc is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "DShowUtil/dshowutil.h"
#include "DShowUtil/DSGeometry.h"
#include "cores/VideoRenderers/RenderManager.h"

#include "WindowingFactory.h" //d3d device and d3d interface
#include "VMR9AllocatorPresenter.h"
#include "application.h"
#include "ApplicationRenderer.h"
#include "utils/log.h"
#include "MacrovisionKicker.h"
#include "IPinHook.h"
#include "GuiSettings.h"


class COuterVMR9
  : public CUnknown
  , public IVideoWindow
  , public IBasicVideo2
  , public IVMRWindowlessControl
  , public IVMRMixerBitmap9
{
  CComPtr<IUnknown>  m_pVMR;
  VMR9AlphaBitmap*  m_pVMR9AlphaBitmap;
  CVMR9AllocatorPresenter *m_pAllocatorPresenter;

public:

  COuterVMR9(const TCHAR* pName, LPUNKNOWN pUnk, VMR9AlphaBitmap* pVMR9AlphaBitmap, CVMR9AllocatorPresenter *_pAllocatorPresenter) : CUnknown(pName, pUnk)
  {
    m_pVMR.CoCreateInstance(CLSID_VideoMixingRenderer9, GetOwner());
    m_pVMR9AlphaBitmap = pVMR9AlphaBitmap;
    m_pAllocatorPresenter = _pAllocatorPresenter;
  }

  ~COuterVMR9()
  {
    m_pVMR = NULL;
  }

  DECLARE_IUNKNOWN;
  STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv)
  {
    HRESULT hr;

    // Casimir666 : en mode Renderless faire l'incrustation � la place du VMR
    if(riid == __uuidof(IVMRMixerBitmap9))
      return GetInterface((IVMRMixerBitmap9*)this, ppv);

    hr = m_pVMR ? m_pVMR->QueryInterface(riid, ppv) : E_NOINTERFACE;
    if(m_pVMR && FAILED(hr))
    {
      if(riid == __uuidof(IVideoWindow))
        return GetInterface((IVideoWindow*)this, ppv);
      if(riid == __uuidof(IBasicVideo))
        return GetInterface((IBasicVideo*)this, ppv);
      if(riid == __uuidof(IBasicVideo2))
        return GetInterface((IBasicVideo2*)this, ppv);
/*      if(riid == __uuidof(IVMRWindowlessControl))
        return GetInterface((IVMRWindowlessControl*)this, ppv);
*/
    }

    return SUCCEEDED(hr) ? hr : __super::NonDelegatingQueryInterface(riid, ppv);
  }

  // IVMRWindowlessControl

  STDMETHODIMP GetNativeVideoSize(LONG* lpWidth, LONG* lpHeight, LONG* lpARWidth, LONG* lpARHeight)
  {
    if(CComQIPtr<IVMRWindowlessControl9> pWC9 = m_pVMR)
    {
      return pWC9->GetNativeVideoSize(lpWidth, lpHeight, lpARWidth, lpARHeight);
    }

    return E_NOTIMPL;
  }
  STDMETHODIMP GetMinIdealVideoSize(LONG* lpWidth, LONG* lpHeight) {return E_NOTIMPL;}
  STDMETHODIMP GetMaxIdealVideoSize(LONG* lpWidth, LONG* lpHeight) {return E_NOTIMPL;}
  STDMETHODIMP SetVideoPosition(const LPRECT lpSRCRect, const LPRECT lpDSTRect) {return E_NOTIMPL;}
    STDMETHODIMP GetVideoPosition(LPRECT lpSRCRect, LPRECT lpDSTRect)
  {
    if(CComQIPtr<IVMRWindowlessControl9> pWC9 = m_pVMR)
    {
      return pWC9->GetVideoPosition(lpSRCRect, lpDSTRect);
    }

    return E_NOTIMPL;
  }
  STDMETHODIMP GetAspectRatioMode(DWORD* lpAspectRatioMode)
  {
    if(CComQIPtr<IVMRWindowlessControl9> pWC9 = m_pVMR)
    {
      *lpAspectRatioMode = VMR_ARMODE_NONE;
      return S_OK;
    }

    return E_NOTIMPL;
  }
  STDMETHODIMP SetAspectRatioMode(DWORD AspectRatioMode) {return E_NOTIMPL;}
  STDMETHODIMP SetVideoClippingWindow(HWND hwnd) {return E_NOTIMPL;}
  STDMETHODIMP RepaintVideo(HWND hwnd, HDC hdc) {return E_NOTIMPL;}
  STDMETHODIMP DisplayModeChanged() {return E_NOTIMPL;}
  STDMETHODIMP GetCurrentImage(BYTE** lpDib) {return E_NOTIMPL;}
  STDMETHODIMP SetBorderColor(COLORREF Clr) {return E_NOTIMPL;}
  STDMETHODIMP GetBorderColor(COLORREF* lpClr) {return E_NOTIMPL;}
  STDMETHODIMP SetColorKey(COLORREF Clr) {return E_NOTIMPL;}
  STDMETHODIMP GetColorKey(COLORREF* lpClr) {return E_NOTIMPL;}

  // IVideoWindow
  STDMETHODIMP GetTypeInfoCount(UINT* pctinfo) {return E_NOTIMPL;}
  STDMETHODIMP GetTypeInfo(UINT iTInfo, LCID lcid, ITypeInfo** ppTInfo) {return E_NOTIMPL;}
  STDMETHODIMP GetIDsOfNames(REFIID riid, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId) {return E_NOTIMPL;}
  STDMETHODIMP Invoke(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr) {return E_NOTIMPL;}
    STDMETHODIMP put_Caption(BSTR strCaption) {return E_NOTIMPL;}
    STDMETHODIMP get_Caption(BSTR* strCaption) {return E_NOTIMPL;}
  STDMETHODIMP put_WindowStyle(long WindowStyle) {return E_NOTIMPL;}
  STDMETHODIMP get_WindowStyle(long* WindowStyle) {return E_NOTIMPL;}
  STDMETHODIMP put_WindowStyleEx(long WindowStyleEx) {return E_NOTIMPL;}
  STDMETHODIMP get_WindowStyleEx(long* WindowStyleEx) {return E_NOTIMPL;}
  STDMETHODIMP put_AutoShow(long AutoShow) {return E_NOTIMPL;}
  STDMETHODIMP get_AutoShow(long* AutoShow) {return E_NOTIMPL;}
  STDMETHODIMP put_WindowState(long WindowState) {return E_NOTIMPL;}
  STDMETHODIMP get_WindowState(long* WindowState) {return E_NOTIMPL;}
  STDMETHODIMP put_BackgroundPalette(long BackgroundPalette) {return E_NOTIMPL;}
  STDMETHODIMP get_BackgroundPalette(long* pBackgroundPalette) {return E_NOTIMPL;}
  STDMETHODIMP put_Visible(long Visible) {return E_NOTIMPL;}
  STDMETHODIMP get_Visible(long* pVisible) {return E_NOTIMPL;}
  STDMETHODIMP put_Left(long Left) {return E_NOTIMPL;}
  STDMETHODIMP get_Left(long* pLeft) {return E_NOTIMPL;}
  STDMETHODIMP put_Width(long Width) {return E_NOTIMPL;}
  STDMETHODIMP get_Width(long* pWidth)
  {
    if(CComQIPtr<IVMRWindowlessControl9> pWC9 = m_pVMR)
    {
      tagRECT s, d;
      HRESULT hr = pWC9->GetVideoPosition(&s, &d);
      *pWidth = d.right-d.left;
      //*pWidth = d.Width();
      return hr;
    }

    return E_NOTIMPL;
  }
  STDMETHODIMP put_Top(long Top) {return E_NOTIMPL;}
  STDMETHODIMP get_Top(long* pTop) {return E_NOTIMPL;}
  STDMETHODIMP put_Height(long Height) {return E_NOTIMPL;}
  STDMETHODIMP get_Height(long* pHeight)
  {
    if(CComQIPtr<IVMRWindowlessControl9> pWC9 = m_pVMR)
    {
      tagRECT s, d;
      HRESULT hr = pWC9->GetVideoPosition(&s, &d);
      *pHeight = g_geometryHelper.GetHeight(d);
      return hr;
    }

    return E_NOTIMPL;
  }
  STDMETHODIMP put_Owner(OAHWND Owner) {return E_NOTIMPL;}
  STDMETHODIMP get_Owner(OAHWND* Owner) {return E_NOTIMPL;}
  STDMETHODIMP put_MessageDrain(OAHWND Drain) {return E_NOTIMPL;}
  STDMETHODIMP get_MessageDrain(OAHWND* Drain) {return E_NOTIMPL;}
  STDMETHODIMP get_BorderColor(long* Color) {return E_NOTIMPL;}
  STDMETHODIMP put_BorderColor(long Color) {return E_NOTIMPL;}
  STDMETHODIMP get_FullScreenMode(long* FullScreenMode) {return E_NOTIMPL;}
  STDMETHODIMP put_FullScreenMode(long FullScreenMode) {return E_NOTIMPL;}
    STDMETHODIMP SetWindowForeground(long Focus) {return E_NOTIMPL;}
    STDMETHODIMP NotifyOwnerMessage(OAHWND hwnd, long uMsg, LONG_PTR wParam, LONG_PTR lParam) {return E_NOTIMPL;}
    STDMETHODIMP SetWindowPosition(long Left, long Top, long Width, long Height) {return E_NOTIMPL;}
  STDMETHODIMP GetWindowPosition(long* pLeft, long* pTop, long* pWidth, long* pHeight) {return E_NOTIMPL;}
  STDMETHODIMP GetMinIdealImageSize(long* pWidth, long* pHeight) {return E_NOTIMPL;}
  STDMETHODIMP GetMaxIdealImageSize(long* pWidth, long* pHeight) {return E_NOTIMPL;}
  STDMETHODIMP GetRestorePosition(long* pLeft, long* pTop, long* pWidth, long* pHeight) {return E_NOTIMPL;}
  STDMETHODIMP HideCursor(long HideCursor) {return E_NOTIMPL;}
  STDMETHODIMP IsCursorHidden(long* CursorHidden) {return E_NOTIMPL;}

  // IBasicVideo2
    STDMETHODIMP get_AvgTimePerFrame(REFTIME* pAvgTimePerFrame) {return E_NOTIMPL;}
    STDMETHODIMP get_BitRate(long* pBitRate) {return E_NOTIMPL;}
    STDMETHODIMP get_BitErrorRate(long* pBitErrorRate) {return E_NOTIMPL;}
    STDMETHODIMP get_VideoWidth(long* pVideoWidth) {return E_NOTIMPL;}
    STDMETHODIMP get_VideoHeight(long* pVideoHeight) {return E_NOTIMPL;}
    STDMETHODIMP put_SourceLeft(long SourceLeft) {return E_NOTIMPL;}
    STDMETHODIMP get_SourceLeft(long* pSourceLeft) {return E_NOTIMPL;}
    STDMETHODIMP put_SourceWidth(long SourceWidth) {return E_NOTIMPL;}
    STDMETHODIMP get_SourceWidth(long* pSourceWidth) {return E_NOTIMPL;}
    STDMETHODIMP put_SourceTop(long SourceTop) {return E_NOTIMPL;}
    STDMETHODIMP get_SourceTop(long* pSourceTop) {return E_NOTIMPL;}
    STDMETHODIMP put_SourceHeight(long SourceHeight) {return E_NOTIMPL;}
    STDMETHODIMP get_SourceHeight(long* pSourceHeight) {return E_NOTIMPL;}
    STDMETHODIMP put_DestinationLeft(long DestinationLeft) {return E_NOTIMPL;}
    STDMETHODIMP get_DestinationLeft(long* pDestinationLeft) {return E_NOTIMPL;}
    STDMETHODIMP put_DestinationWidth(long DestinationWidth) {return E_NOTIMPL;}
    STDMETHODIMP get_DestinationWidth(long* pDestinationWidth) {return E_NOTIMPL;}
    STDMETHODIMP put_DestinationTop(long DestinationTop) {return E_NOTIMPL;}
    STDMETHODIMP get_DestinationTop(long* pDestinationTop) {return E_NOTIMPL;}
    STDMETHODIMP put_DestinationHeight(long DestinationHeight) {return E_NOTIMPL;}
    STDMETHODIMP get_DestinationHeight(long* pDestinationHeight) {return E_NOTIMPL;}
    STDMETHODIMP SetSourcePosition(long Left, long Top, long Width, long Height) {return E_NOTIMPL;}
    STDMETHODIMP GetSourcePosition(long* pLeft, long* pTop, long* pWidth, long* pHeight)
  {
    // DVD Nav. bug workaround fix
    {
      *pLeft = *pTop = 0;
      return GetVideoSize(pWidth, pHeight);
    }
/*
    if(CComQIPtr<IVMRWindowlessControl9> pWC9 = m_pVMR)
    {
      tagRECT s, d;
      HRESULT hr = pWC9->GetVideoPosition(&s, &d);
      *pLeft = s.left;
      *pTop = s.top;
      *pWidth = s.Width();
      *pHeight = s.Height();
      return hr;
    }
*/
    return E_NOTIMPL;
  }
    STDMETHODIMP SetDefaultSourcePosition() {return E_NOTIMPL;}
    STDMETHODIMP SetDestinationPosition(long Left, long Top, long Width, long Height) {return E_NOTIMPL;}
    STDMETHODIMP GetDestinationPosition(long* pLeft, long* pTop, long* pWidth, long* pHeight)
  {
    if(CComQIPtr<IVMRWindowlessControl9> pWC9 = m_pVMR)
    {
      tagRECT s, d;
      HRESULT hr = pWC9->GetVideoPosition(&s, &d);
      *pLeft = d.left;
      *pTop = d.top;
      *pWidth = g_geometryHelper.GetWidth(d);
      *pHeight = g_geometryHelper.GetHeight(d);
      return hr;
    }

    return E_NOTIMPL;
  }
    STDMETHODIMP SetDefaultDestinationPosition() {return E_NOTIMPL;}
    STDMETHODIMP GetVideoSize(long* pWidth, long* pHeight)
  {
    if(CComQIPtr<IVMRWindowlessControl9> pWC9 = m_pVMR)
    {
      LONG aw, ah;
      HRESULT hr = pWC9->GetNativeVideoSize(pWidth, pHeight, &aw, &ah);
      *pWidth = *pHeight * aw / ah;
      return hr;
    }

    return E_NOTIMPL;
  }

    STDMETHODIMP GetVideoPaletteEntries(long StartIndex, long Entries, long* pRetrieved, long* pPalette) {return E_NOTIMPL;}
    STDMETHODIMP GetCurrentImage(long* pBufferSize, long* pDIBImage) {return E_NOTIMPL;}
    STDMETHODIMP IsUsingDefaultSource() {return E_NOTIMPL;}
    STDMETHODIMP IsUsingDefaultDestination() {return E_NOTIMPL;}

  STDMETHODIMP GetPreferredAspectRatio(long* plAspectX, long* plAspectY)
  {
    if(CComQIPtr<IVMRWindowlessControl9> pWC9 = m_pVMR)
    {
      LONG w, h;
      return pWC9->GetNativeVideoSize(&w, &h, plAspectX, plAspectY);
    }

    return E_NOTIMPL;
  }

  // IVMRMixerBitmap9
  STDMETHODIMP GetAlphaBitmapParameters(VMR9AlphaBitmap* pBmpParms)
  {
    CheckPointer(pBmpParms, E_POINTER);
    CAutoLock BitMapLock(&m_pAllocatorPresenter->m_VMR9AlphaBitmapLock);
    memcpy (pBmpParms, m_pVMR9AlphaBitmap, sizeof(VMR9AlphaBitmap));
    return S_OK;
  }
  
  STDMETHODIMP SetAlphaBitmap(const VMR9AlphaBitmap*  pBmpParms)
  {
    CheckPointer(pBmpParms, E_POINTER);
    CAutoLock BitMapLock(&m_pAllocatorPresenter->m_VMR9AlphaBitmapLock);
    memcpy (m_pVMR9AlphaBitmap, pBmpParms, sizeof(VMR9AlphaBitmap));
    m_pVMR9AlphaBitmap->dwFlags |= VMRBITMAP_UPDATE;
    m_pAllocatorPresenter->UpdateAlphaBitmap();
    return S_OK;
  }

  STDMETHODIMP UpdateAlphaBitmapParameters(const VMR9AlphaBitmap* pBmpParms)
  {
    CheckPointer(pBmpParms, E_POINTER);
    CAutoLock BitMapLock(&m_pAllocatorPresenter->m_VMR9AlphaBitmapLock);
    memcpy (m_pVMR9AlphaBitmap, pBmpParms, sizeof(VMR9AlphaBitmap));
    m_pVMR9AlphaBitmap->dwFlags |= VMRBITMAP_UPDATE;
    m_pAllocatorPresenter->UpdateAlphaBitmap();
    return S_OK;
  }
};

CVMR9AllocatorPresenter::CVMR9AllocatorPresenter(HRESULT& hr, CStdString &_Error)
: CDsRenderer(),
  m_refCount(1),
  m_pNbrSurface(0),
  m_pCurSurface(0)
{
  m_D3D = g_Windowing.Get3DObject();
  m_D3DDev = g_Windowing.Get3DDevice();
  hr = S_OK;
  InitializeCriticalSection(&m_critPrensent);
  m_renderingOk = true;
}

CVMR9AllocatorPresenter::~CVMR9AllocatorPresenter()
{
  DeleteSurfaces();
  DeleteCriticalSection(&m_critPrensent);
  
}

void CVMR9AllocatorPresenter::DeleteSurfaces()
{
  EnterCriticalSection(&m_critPrensent);
  
  // clear out the private texture
  m_pVideoTexture = NULL;
  for( size_t i = 0; i < m_pSurfaces.size(); ++i ) 
     m_pSurfaces[i] = NULL;
  LeaveCriticalSection(&m_critPrensent);
}

//IVMRSurfaceAllocator9
STDMETHODIMP CVMR9AllocatorPresenter::InitializeDevice(DWORD_PTR dwUserID ,VMR9AllocationInfo *lpAllocInfo, DWORD *lpNumBuffers)
{
  CLog::Log(LOGNOTICE,"vmr9:InitializeDevice() %dx%d AR %d:%d flags:%d buffers:%d  fmt:(%x) %c%c%c%c", 
    lpAllocInfo->dwWidth ,lpAllocInfo->dwHeight ,lpAllocInfo->szAspectRatio.cx,lpAllocInfo->szAspectRatio.cy,
    lpAllocInfo->dwFlags ,*lpNumBuffers, lpAllocInfo->Format, ((char)lpAllocInfo->Format&0xff),
	((char)(lpAllocInfo->Format>>8)&0xff) ,((char)(lpAllocInfo->Format>>16)&0xff) ,((char)(lpAllocInfo->Format>>24)&0xff));

  if( !lpAllocInfo || !lpNumBuffers )
    return E_POINTER;
  if( !m_pIVMRSurfAllocNotify)
    return E_FAIL;
  HRESULT hr = S_OK;

  if(lpAllocInfo->Format == '21VY' || lpAllocInfo->Format == '024I')
    return E_FAIL;
  
  int nOriginal = *lpNumBuffers;

  //To do implement the texture surface on the present image
  //if(lpAllocInfo->dwFlags & VMR9AllocFlag_3DRenderTarget)
  //lpAllocInfo->dwFlags |= VMR9AllocFlag_TextureSurface;
  CStdString strVmr9Flags;
  strVmr9Flags.append("VMR9 Flags:");
  if (lpAllocInfo->dwFlags & VMR9AllocFlag_3DRenderTarget)
    strVmr9Flags.append(" 3drendertarget,");
  if (lpAllocInfo->dwFlags & VMR9AllocFlag_DXVATarget)
    strVmr9Flags.append(" DXVATarget,");
  if (lpAllocInfo->dwFlags & VMR9AllocFlag_OffscreenSurface) 
    strVmr9Flags.append(" OffscreenSurface,");
  if (lpAllocInfo->dwFlags & VMR9AllocFlag_RGBDynamicSwitch) 
    strVmr9Flags.append(" RGBDynamicSwitch,");
  if (lpAllocInfo->dwFlags & VMR9AllocFlag_TextureSurface)
    strVmr9Flags.append(" TextureSurface.");
  CLog::Log(LOGNOTICE,"%s",strVmr9Flags.c_str());
  if (*lpNumBuffers == 1)
	{
		*lpNumBuffers = 4;
		m_pNbrSurface = 4;
	}
	else
		m_pNbrSurface = 0;

  m_pSurfaces.resize(*lpNumBuffers);
  hr = m_pIVMRSurfAllocNotify->AllocateSurfaceHelper(lpAllocInfo, lpNumBuffers, & m_pSurfaces.at(0) );
  if(FAILED(hr))
  {
    CLog::Log(LOGERROR,"%s AllocateSurfaceHelper returned:0x%x",__FUNCTION__,hr);
    return hr;
  }
  m_iVideoWidth = lpAllocInfo->dwWidth;
  m_iVideoHeight = lpAllocInfo->dwHeight;

  //INITIALIZE VIDEO SURFACE THERE
  hr = AllocSurface();
  if ( FAILED( hr ) )
    return hr;
  hr = m_D3DDev->ColorFill(m_pVideoSurface, NULL, 0);
  if ( FAILED( hr ) )
  {
    CLog::Log(LOGERROR,"%s ColorFill returned:0x%x",__FUNCTION__,hr);
    DeleteSurfaces();
    return hr;
  }
  if (m_pNbrSurface && m_pNbrSurface != *lpNumBuffers)
		m_pNbrSurface = *lpNumBuffers;
  
	*lpNumBuffers = (nOriginal < *lpNumBuffers) ? nOriginal : *lpNumBuffers;
	m_pCurSurface = 0;
  m_bNeedCheckSample = true;
  return hr;
}

STDMETHODIMP CVMR9AllocatorPresenter::PresentImage(DWORD_PTR dwUserID, VMR9PresentationInfo *lpPresInfo)
{
  HRESULT hr;
  CheckPointer(m_pIVMRSurfAllocNotify, E_UNEXPECTED);
  if( !m_pIVMRSurfAllocNotify )
    return E_FAIL;
  m_llLastSampleTime = m_llSampleTime;
  m_llSampleTime = lpPresInfo->rtStart;
  m_ptstarget = lpPresInfo->rtEnd;
//m_rtFrameCycle
  if (m_rtTimePerFrame == 0 || m_bNeedCheckSample)
  {
    GetVideoInfo();
  }
  //if (!g_renderManager.IsConfigured())

  if (!g_renderManager.IsStarted())
    return E_FAIL;
  
  if(!lpPresInfo || !lpPresInfo->lpSurf)
    return E_POINTER;
   
  if(m_rtTimePerFrame > 0 && m_rtTimePerFrame < 300000)
  {
    REFERENCE_TIME rtCurTime, rtInterleave;
		if (m_VideoClock)
    {
			rtCurTime = m_VideoClock.GetTime();
			rtInterleave = rtCurTime - m_lastFrameArrivedTime;
			m_lastFrameArrivedTime = rtCurTime;
			if(rtInterleave > m_rtTimePerFrame*6/5 && rtInterleave < m_rtTimePerFrame * 3 && m_lastFramePainted){
				//m_pcFramesDropped++;
				m_lastFrameArrivedTime = rtCurTime;
				m_lastFramePainted = false;
				//SVP_LogMsg5(L"droped %f %f %d", double(m_rtFrameCycle) , double(rtInterleave) , m_pcFramesDropped);
				return S_OK;
			}
			//SVP_LogMsg5(L"Paint");
		}
  }
  m_lastFramePainted = true;

  EnterCriticalSection(&m_critPrensent);
  
  CComPtr<IDirect3DTexture9> pTexture;
  hr = lpPresInfo->lpSurf->GetContainer(IID_IDirect3DTexture9, (void**)&pTexture);
  if(pTexture)
  {
    // When using VMR9AllocFlag_TextureSurface
    // Didnt got it working yet
  }
  else
  {
    
    hr = m_D3DDev->StretchRect(lpPresInfo->lpSurf, NULL, m_pVideoSurface, NULL, D3DTEXF_NONE);
    RenderPresent(m_pVideoTexture, m_pVideoSurface);
    //g_renderManager.PaintVideoTexture(m_pVideoTexture, m_pVideoSurface);
  }
  LeaveCriticalSection(&m_critPrensent);
  return hr;
}

void CVMR9AllocatorPresenter::GetVideoInfo()
{
  CComPtr<IBaseFilter> pVMR9;
  CComPtr<IPin> pPin;
  CMediaType mt;
  m_bNeedCheckSample = false;
  if (SUCCEEDED (m_pIVMRSurfAllocNotify->QueryInterface (__uuidof(IBaseFilter), (void**)&pVMR9)) &&
      SUCCEEDED (pVMR9->FindPin(L"VMR Input0", &pPin)) &&
      SUCCEEDED (pPin->ConnectionMediaType(&mt)) )
  {
    DShowUtil::ExtractAvgTimePerFrame(&mt,m_rtTimePerFrame);
    m_dFrameCycle = m_rtTimePerFrame / 10000.0;
    if (m_rtTimePerFrame > 0.0)
    {
      m_fps = 10000000.0 / m_rtTimePerFrame;
      m_dCycleDifference = 1000.0 / m_VideoClock.GetFrequency();

      if (abs(m_dCycleDifference) < 0.05) // If less than 5%
        m_bSnapToVSync = true;
      else
        m_bSnapToVSync = false;
    }
    
    m_bInterlaced = DShowUtil::ExtractInterlaced(&mt);

    if (mt.formattype==FORMAT_VideoInfo || mt.formattype==FORMAT_MPEGVideo)
    {
      VIDEOINFOHEADER *vh = (VIDEOINFOHEADER*)mt.pbFormat;
      m_iVideoWidth = vh->bmiHeader.biWidth;
      m_iVideoHeight = abs(vh->bmiHeader.biHeight);
      if (vh->rcTarget.right - vh->rcTarget.left > 0)
        m_iVideoWidth = vh->rcTarget.right - vh->rcTarget.left;
      else if (vh->rcSource.right - vh->rcSource.left > 0)
        m_iVideoWidth = vh->rcSource.right - vh->rcSource.left;
      if (vh->rcTarget.bottom - vh->rcTarget.top > 0)
        m_iVideoHeight = vh->rcTarget.bottom - vh->rcTarget.top;
      else if (vh->rcSource.bottom - vh->rcSource.top > 0)
        m_iVideoHeight = vh->rcSource.bottom - vh->rcSource.top;
    }
    else if (mt.formattype==FORMAT_VideoInfo2 || mt.formattype==FORMAT_MPEG2Video)
    {
      VIDEOINFOHEADER2 *vh = (VIDEOINFOHEADER2*)mt.pbFormat;
      m_iVideoWidth = vh->bmiHeader.biWidth;
      m_iVideoHeight = abs(vh->bmiHeader.biHeight);
      if (vh->rcTarget.right - vh->rcTarget.left > 0)
        m_iVideoWidth = vh->rcTarget.right - vh->rcTarget.left;
      else if (vh->rcSource.right - vh->rcSource.left > 0)
        m_iVideoWidth = vh->rcSource.right - vh->rcSource.left;

      if (vh->rcTarget.bottom - vh->rcTarget.top > 0)
        m_iVideoHeight = vh->rcTarget.bottom - vh->rcTarget.top;
      else if (vh->rcSource.bottom - vh->rcSource.top > 0)
        m_iVideoHeight = vh->rcSource.bottom - vh->rcSource.top;
    }
    // If 0 defaulting framerate to 23.97...
		if (m_rtTimePerFrame == 0) 
      m_rtTimePerFrame = 417166;

		m_fps = 10000000.0 / m_rtTimePerFrame;
    
    g_renderManager.Configure(m_iVideoWidth, m_iVideoHeight, m_iVideoWidth, m_iVideoHeight, m_fps, CONF_FLAGS_FULLSCREEN);

  }

}

STDMETHODIMP CVMR9AllocatorPresenter::TerminateDevice(DWORD_PTR dwID)
{
    DeleteSurfaces();
    return S_OK;
}
    
STDMETHODIMP CVMR9AllocatorPresenter::GetSurface(DWORD_PTR dwUserID ,DWORD SurfaceIndex ,DWORD SurfaceFlags ,IDirect3DSurface9 **lplpSurface)
{
  if( !lplpSurface )
    return E_POINTER;

  //return if the surface index is higher than the size of the surfaces we have
  if (SurfaceIndex >= m_pSurfaces.size() ) 
    return E_FAIL;

  EnterCriticalSection(&m_critPrensent);
  if (m_pNbrSurface)
  {
    ++m_pCurSurface;
    m_pCurSurface = m_pCurSurface % m_pNbrSurface;
    (*lplpSurface = m_pSurfaces[m_pCurSurface + SurfaceIndex])->AddRef();
  }
  else
  {
    m_pNbrSurface = SurfaceIndex;
    (*lplpSurface = m_pSurfaces[SurfaceIndex])->AddRef();
  }
  LeaveCriticalSection(&m_critPrensent);

  return S_OK;
}
    
STDMETHODIMP CVMR9AllocatorPresenter::AdviseNotify(IVMRSurfaceAllocatorNotify9 *lpIVMRSurfAllocNotify)
{
    EnterCriticalSection(&m_critPrensent);
    HRESULT hr;
    m_pIVMRSurfAllocNotify = lpIVMRSurfAllocNotify;
    HMONITOR hMonitor = m_D3D->GetAdapterMonitor(GetAdapter(m_D3D));
    hr = m_pIVMRSurfAllocNotify->SetD3DDevice( m_D3DDev, hMonitor);
    LeaveCriticalSection(&m_critPrensent);
    return hr;
}

STDMETHODIMP CVMR9AllocatorPresenter::StartPresenting(DWORD_PTR dwUserID)
{
  EnterCriticalSection(&m_critPrensent);
  HRESULT hr = S_OK;
  ASSERT( m_D3DDev );
  if( !m_D3DDev )
    hr =  E_FAIL;
  //Start the video clock
  InitClock();

  LeaveCriticalSection(&m_critPrensent);
  return hr;
}
// IUnknown
STDMETHODIMP CVMR9AllocatorPresenter::QueryInterface( 
        REFIID riid,
        void** ppvObject)
{
    HRESULT hr = E_NOINTERFACE;

    if( ppvObject == NULL ) {
        hr = E_POINTER;
    } 
    else if( riid == IID_IVMRSurfaceAllocator9 ) {
        *ppvObject = static_cast<IVMRSurfaceAllocator9*>( this );
        AddRef();
        hr = S_OK;
    } 
    else if( riid == IID_IVMRImagePresenter9 ) {
        *ppvObject = static_cast<IVMRImagePresenter9*>( this );
        AddRef();
        hr = S_OK;
    }
    else if( riid == IID_IUnknown ) {
        *ppvObject = 
            static_cast<IUnknown*>( 
            static_cast<IVMRSurfaceAllocator9*>( this ) );
        AddRef();
        hr = S_OK;    
    }

    return hr;
}

STDMETHODIMP CVMR9AllocatorPresenter::StopPresenting(DWORD_PTR dwUserID)
{
    return S_OK;
}

ULONG CVMR9AllocatorPresenter::AddRef()
{
    return InterlockedIncrement(& m_refCount);
}

ULONG CVMR9AllocatorPresenter::Release()
{
    ULONG ret = InterlockedDecrement(& m_refCount);
    if( ret == 0 )
    {
        delete this;
    }

    return ret;
}

UINT CVMR9AllocatorPresenter::GetAdapter(IDirect3D9* pD3D)
{
  HMONITOR hMonitor = MonitorFromWindow(g_hWnd, MONITOR_DEFAULTTONEAREST);
  if(hMonitor == NULL) return D3DADAPTER_DEFAULT;

  for(UINT adp = 0, num_adp = pD3D->GetAdapterCount(); adp < num_adp; ++adp)
  {
    HMONITOR hAdpMon = pD3D->GetAdapterMonitor(adp);
    if(hAdpMon == hMonitor) 
      return adp;
  }

  return D3DADAPTER_DEFAULT;
}

STDMETHODIMP CVMR9AllocatorPresenter::CreateRenderer(IUnknown** ppRenderer)
{
  CheckPointer(ppRenderer, E_POINTER);

  *ppRenderer = NULL;

  HRESULT hr;

  do
  {
    CMacrovisionKicker* pMK = new CMacrovisionKicker(NAME("CMacrovisionKicker"), NULL);
    CComPtr<IUnknown> pUnk = (IUnknown*)(INonDelegatingUnknown*)pMK;

    COuterVMR9 *pOuter = new COuterVMR9(NAME("COuterVMR9"), pUnk, &m_VMR9AlphaBitmap, this);


    pMK->SetInner((IUnknown*)(INonDelegatingUnknown*)pOuter);
    CComQIPtr<IBaseFilter> pBF = pUnk;

    CComPtr<IPin> pPin = DShowUtil::GetFirstPin(pBF);
    CComQIPtr<IMemInputPin> pMemInputPin = pPin;
    m_fUseInternalTimer = HookNewSegmentAndReceive((IPinC*)(IPin*)pPin, (IMemInputPinC*)(IMemInputPin*)pMemInputPin);

    if(CComQIPtr<IAMVideoAccelerator> pAMVA = pPin)
      HookAMVideoAccelerator((IAMVideoAcceleratorC*)(IAMVideoAccelerator*)pAMVA);

    CComQIPtr<IVMRFilterConfig9> pConfig = pBF;
    if(!pConfig)
      break;

    if(1)//s.fVMR9MixerMode)
    {
      if(FAILED(hr = pConfig->SetNumberOfStreams(1)))
        break;

      if(CComQIPtr<IVMRMixerControl9> pMC = pBF)
      {
        DWORD dwPrefs;
        pMC->GetMixingPrefs(&dwPrefs);  

        // See http://msdn.microsoft.com/en-us/library/dd390928(VS.85).aspx
        dwPrefs |= MixerPref9_NonSquareMixing;
        dwPrefs |= MixerPref9_NoDecimation;
		if(1)//DShowUtil::IsVistaOrAbove())
        {
          dwPrefs &= ~MixerPref9_RenderTargetMask; 
          dwPrefs |= MixerPref9_RenderTargetYUV;
        }
        pMC->SetMixingPrefs(dwPrefs);    
      }
    }

    if(FAILED(hr = pConfig->SetRenderingMode(VMR9Mode_Renderless)))
      break;

    CComQIPtr<IVMRSurfaceAllocatorNotify9> pSAN = pBF;
    if(!pSAN)
      break;
    DWORD_PTR MY_USER_ID = 0xACDCACDC;
    if(FAILED(hr = pSAN->AdviseSurfaceAllocator(MY_USER_ID, static_cast<IVMRSurfaceAllocator9*>(this)))
    || FAILED(hr = AdviseNotify(pSAN)))
      break;

    *ppRenderer = (IUnknown*)pBF.Detach();

    return S_OK;
  }
  while(0);

  return E_FAIL;
}

void CVMR9AllocatorPresenter::UpdateAlphaBitmap()
{
  m_VMR9AlphaBitmapData.Free();

  if ((m_VMR9AlphaBitmap.dwFlags & VMRBITMAP_DISABLE) == 0)
  {
    HBITMAP      hBitmap = (HBITMAP)GetCurrentObject (m_VMR9AlphaBitmap.hdc, OBJ_BITMAP);
    if (!hBitmap)
      return;
    DIBSECTION    info = {0};
    if (!::GetObject(hBitmap, sizeof( DIBSECTION ), &info ))
      return;

    m_VMR9AlphaBitmapRect = g_geometryHelper.CreateRect(0, 0, info.dsBm.bmWidth, info.dsBm.bmHeight);
    m_VMR9AlphaBitmapWidthBytes = info.dsBm.bmWidthBytes;

    if (m_VMR9AlphaBitmapData.Allocate(info.dsBm.bmWidthBytes * info.dsBm.bmHeight))
    {
      memcpy((BYTE *)m_VMR9AlphaBitmapData, info.dsBm.bmBits, info.dsBm.bmWidthBytes * info.dsBm.bmHeight);
    }
  }
}
