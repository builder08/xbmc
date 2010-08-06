/*
 *      Copyright (C) 2007-2010 Team XBMC
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

#include "WinVideoFilter.h"
#include "WindowingFactory.h"
#include "../../../utils/log.h"
#include "../../../FileSystem/File.h"
#include <map>
#include "ConvolutionKernels.h"
#include "YUV2RGBShader.h"

CYUV2RGBMatrix::CYUV2RGBMatrix()
{
  m_NeedRecalc = true;
}

void CYUV2RGBMatrix::SetParameters(float contrast, float blacklevel, unsigned int flags)
{
  if (m_contrast != contrast)
  {
    m_NeedRecalc = true;
    m_contrast = contrast;
  }
  if (m_blacklevel != blacklevel)
  {
    m_NeedRecalc = true;
    m_blacklevel = blacklevel;
  }
  if (m_flags != flags)
  {
    m_NeedRecalc = true;
    m_flags = flags;
  }
}

D3DXMATRIX* CYUV2RGBMatrix::Matrix()
{
  if (m_NeedRecalc)
  {
    TransformMatrix matrix;
    CalculateYUVMatrix(matrix, m_flags, m_blacklevel, m_contrast);

    m_mat._11 = matrix.m[0][0];
    m_mat._12 = matrix.m[1][0];
    m_mat._13 = matrix.m[2][0];
    m_mat._14 = 0.0f;
    m_mat._21 = matrix.m[0][1];
    m_mat._22 = matrix.m[1][1];
    m_mat._23 = matrix.m[2][1];
    m_mat._24 = 0.0f;
    m_mat._31 = matrix.m[0][2];
    m_mat._32 = matrix.m[1][2];
    m_mat._33 = matrix.m[2][2];
    m_mat._44 = 0.0f;
    m_mat._41 = matrix.m[0][3];
    m_mat._42 = matrix.m[1][3];
    m_mat._43 = matrix.m[2][3];
    m_mat._44 = 1.0f;

    m_NeedRecalc = false;
  }
  return &m_mat;
}

//===================================================================

CWinShader::CWinShader()
{
  m_verts = NULL;
}

void CWinShader::Release()
{
  ReleaseInternal(); // virtual, so calls the child function, which is supposed to call down the hierarchy
  delete this;
}

bool CWinShader::CreateVertexBuffer(unsigned int vertCount, unsigned int vertSize, unsigned int primitivesCount)
{
  // Allocate the vertex buffer
  m_verts = malloc(vertCount * vertSize);
  m_vertsize = vertSize;
  m_primitivesCount = primitivesCount;
  return (m_verts != NULL);
}

void CWinShader::ReleaseInternal()
{
  if (m_verts)
  {
    free(m_verts);
    m_verts = NULL;
  }
  if (m_effect.Get())
    m_effect.Release();
  
  //derived classes: always call Base::ReleaseInternal() at the end
}

bool CWinShader::LoadEffect(CStdString filename, DefinesMap* defines)
{
  CLog::Log(LOGDEBUG, __FUNCTION__" - loading shader %s", filename.c_str());

  XFILE::CFileStream file;
  if(!file.Open(filename))
  {
    CLog::Log(LOGERROR, __FUNCTION__" - failed to open file %s", filename.c_str());
    return false;
  }

  CStdString pStrEffect;
  getline(file, pStrEffect, '\0');

  if (!m_effect.Create(pStrEffect, defines))
  {
    CLog::Log(LOGERROR, __FUNCTION__" %s failed", pStrEffect.c_str());
    return false;
  }

  return true;
}

bool CWinShader::Execute(LPDIRECT3DDEVICE9 pD3DDevice)
{
  UINT cPasses, iPass;
  if (!m_effect.Begin( &cPasses, 0 ))
  {
    CLog::Log(LOGERROR, __FUNCTION__" - failed to begin d3d effect");
    return false;
  }

  for( iPass = 0; iPass < cPasses; iPass++ )
  {
    if (!m_effect.BeginPass( iPass ))
    {
      CLog::Log(LOGERROR, __FUNCTION__" - failed to begin d3d effect pass");
      break;
    }
    HRESULT hr = pD3DDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, m_primitivesCount, m_verts, m_vertsize);
    if (FAILED(hr))
      CLog::Log(LOGERROR, __FUNCTION__" - failed DrawPrimitiveUP %08X", hr);

    if (!m_effect.EndPass())
      CLog::Log(LOGERROR, __FUNCTION__" - failed to end d3d effect pass");
  }
  if (!m_effect.End())
    CLog::Log(LOGERROR, __FUNCTION__" - failed to end d3d effect");

  pD3DDevice->SetPixelShader( NULL );

  for (unsigned int i=0; i < m_boundTexturesCount; i++)
    pD3DDevice->SetTexture(i, NULL);

  return true;
}

//==================================================================================

bool CYUV2RGBShader::Create(bool singlepass)
{
  ReleaseInternal();

  CWinShader::CreateVertexBuffer(4, sizeof(CUSTOMVERTEX), 2);
  m_boundTexturesCount = 3;

  DefinesMap defines;
  if (singlepass)
    defines["SINGLEPASS"] = "";

  CStdString effectString = "special://xbmc/system/shaders/yuv2rgb_d3d.fx";

  if(!LoadEffect(effectString, &defines))
  {
    CLog::Log(LOGERROR, __FUNCTION__": Failed to load shader %s.", effectString.c_str());
    return false;
  }
  return true;
}

void CYUV2RGBShader::Render(unsigned int sourceWidth, unsigned int sourceHeight,
                               CRect sourceRect,
                               CRect destRect,
                               float contrast,
                               float brightness,
                               unsigned int flags,
                               YUVBuffer* YUVbuf)
{
  LPDIRECT3DDEVICE9 pD3DDevice = g_Windowing.Get3DDevice();
  pD3DDevice->SetFVF( D3DFVF_XYZRHW | D3DFVF_TEX3 );
  PrepareParameters(sourceWidth, sourceHeight,
                               sourceRect,
                               destRect,
                               contrast,
                               brightness,
                               flags);
  SetShaderParameters(m_matrix.Matrix(), YUVbuf, sourceWidth);
  Execute(pD3DDevice);
}

void CYUV2RGBShader::PrepareParameters(unsigned int sourceWidth, unsigned int sourceHeight,
                               CRect sourceRect,
                               CRect destRect,
                               float contrast,
                               float brightness,
                               unsigned int flags)
{
  //See RGB renderer for comment on this
  #define CHROMAOFFSET_HORIZ 0.25f

  CUSTOMVERTEX* v = (CUSTOMVERTEX*)CWinShader::GetVertexBuffer();

  v[0].x = destRect.x1;
  v[0].y = destRect.y1;
  v[0].tu = sourceRect.x1 / sourceWidth;
  v[0].tv = sourceRect.y1 / sourceHeight;
  v[0].tu2 = v[0].tu3 = (sourceRect.x1 / 2.0f + CHROMAOFFSET_HORIZ) / (sourceWidth>>1);
  v[0].tv2 = v[0].tv3 = (sourceRect.y1 / 2.0f + CHROMAOFFSET_HORIZ) / (sourceHeight>>1);

  v[1].x = destRect.x2;
  v[1].y = destRect.y1;
  v[1].tu = sourceRect.x2 / sourceWidth;
  v[1].tv = sourceRect.y1 / sourceHeight;
  v[1].tu2 = v[1].tu3 = (sourceRect.x2 / 2.0f + CHROMAOFFSET_HORIZ) / (sourceWidth>>1);
  v[1].tv2 = v[1].tv3 = (sourceRect.y1 / 2.0f + CHROMAOFFSET_HORIZ) / (sourceHeight>>1);

  v[2].x = destRect.x2;
  v[2].y = destRect.y2;
  v[2].tu = sourceRect.x2 / sourceWidth;
  v[2].tv = sourceRect.y2 / sourceHeight;
  v[2].tu2 = v[2].tu3 = (sourceRect.x2 / 2.0f + CHROMAOFFSET_HORIZ) / (sourceWidth>>1);
  v[2].tv2 = v[2].tv3 = (sourceRect.y2 / 2.0f + CHROMAOFFSET_HORIZ) / (sourceHeight>>1);

  v[3].x = destRect.x1;
  v[3].y = destRect.y2;
  v[3].tu = sourceRect.x1 / sourceWidth;
  v[3].tv = sourceRect.y2 / sourceHeight;
  v[3].tu2 = v[3].tu3 = (sourceRect.x1 / 2.0f + CHROMAOFFSET_HORIZ) / (sourceWidth>>1);
  v[3].tv2 = v[3].tv3 = (sourceRect.y2 / 2.0f + CHROMAOFFSET_HORIZ) / (sourceHeight>>1);

  // -0.5 offset to compensate for D3D rasterization
  // set z and rhw
  for(int i = 0; i < 4; i++)
  {
    v[i].x -= 0.5;
    v[i].y -= 0.5;
    v[i].z = 0.0f;
    v[i].rhw = 1.0f;
  }

  m_matrix.SetParameters(contrast * 0.02f,
                         brightness * 0.01f - 0.5f,
                         flags);
}

void CYUV2RGBShader::SetShaderParameters(D3DXMATRIX* matrix, YUVBuffer* YUVbuf, unsigned int sourceWidth)
{
  m_effect.SetMatrix( "g_ColorMatrix", matrix);
  m_effect.SetTechnique( "YUV2RGB_T" );
  m_effect.SetTexture( "g_YTexture",  YUVbuf->planes[0].texture ) ;
  m_effect.SetTexture( "g_UTexture",  YUVbuf->planes[1].texture ) ;
  m_effect.SetTexture( "g_VTexture",  YUVbuf->planes[2].texture ) ;
}

//==================================================================================

bool CConvolutionShader::Create(ESCALINGMETHOD method)
{
  ReleaseInternal();

  CStdString effectString;
  switch(method)
  {
    case VS_SCALINGMETHOD_CUBIC:
    case VS_SCALINGMETHOD_LANCZOS2:
    case VS_SCALINGMETHOD_LANCZOS3_FAST:
      effectString = "special://xbmc/system/shaders/convolution-4x4_d3d.fx";
      break;
    case VS_SCALINGMETHOD_LANCZOS3:
      effectString = "special://xbmc/system/shaders/convolution-6x6_d3d.fx";
      break;
    default:
      CLog::Log(LOGERROR, __FUNCTION__": scaling method %d not supported.", method);
      return false;
  }

  CWinShader::CreateVertexBuffer(4, sizeof(CUSTOMVERTEX), 2);
  m_boundTexturesCount = 2;

  if(!LoadEffect(effectString, NULL))
  {
    CLog::Log(LOGERROR, __FUNCTION__": Failed to load shader %s.", effectString.c_str());
    return false;
  }

  if (!CreateHQKernel(method))
    return false;

  return true;
}

void CConvolutionShader::Render(CD3DTexture &sourceTexture,
                               unsigned int sourceWidth, unsigned int sourceHeight,
                               CRect sourceRect,
                               CRect destRect)
{
  LPDIRECT3DDEVICE9 pD3DDevice = g_Windowing.Get3DDevice();
  pD3DDevice->SetFVF( D3DFVF_XYZRHW | D3DFVF_TEX1 );
  PrepareParameters(sourceWidth, sourceHeight, sourceRect, destRect);
  float texSteps[] = { 1.0f/(float)sourceWidth, 1.0f/(float)sourceHeight};
  SetShaderParameters(sourceTexture, &texSteps[0], sizeof(texSteps)/sizeof(texSteps[0]));
  Execute(pD3DDevice);
}

bool CConvolutionShader::CreateHQKernel(ESCALINGMETHOD method)
{
  CConvolutionKernel kern(method, 256);

  if (!m_HQKernelTexture.Create(kern.GetSize(), 1, 1, g_Windowing.DefaultD3DUsage(), D3DFMT_A16B16G16R16F, g_Windowing.DefaultD3DPool()))
  {
    CLog::Log(LOGERROR, __FUNCTION__": Failed to create kernel texture.");
    return false;
  }

  float *kernelVals = kern.GetFloatPixels();
  D3DXFLOAT16* float16Vals = new D3DXFLOAT16[kern.GetSize()*4];

  for(int i = 0; i < kern.GetSize()*4; i++)
    float16Vals[i] = kernelVals[i];

  D3DLOCKED_RECT lr;
  if (!m_HQKernelTexture.LockRect(0, &lr, NULL, D3DLOCK_DISCARD))
    CLog::Log(LOGERROR, __FUNCTION__": Failed to lock kernel texture.");
  memcpy(lr.pBits, float16Vals, sizeof(D3DXFLOAT16)*kern.GetSize()*4);
  if (!m_HQKernelTexture.UnlockRect(0))
    CLog::Log(LOGERROR, __FUNCTION__": Failed to unlock kernel texture.");


  delete[] float16Vals;

  return true;
}

void CConvolutionShader::PrepareParameters(unsigned int sourceWidth, unsigned int sourceHeight,
                               CRect sourceRect,
                               CRect destRect)
{
  CUSTOMVERTEX* v = (CUSTOMVERTEX*)CWinShader::GetVertexBuffer();

  v[0].x = destRect.x1;
  v[0].y = destRect.y1;
  v[0].tu = sourceRect.x1 / sourceWidth;
  v[0].tv = sourceRect.y1 / sourceHeight;

  v[1].x = destRect.x2;
  v[1].y = destRect.y1;
  v[1].tu = sourceRect.x2 / sourceWidth;
  v[1].tv = sourceRect.y1 / sourceHeight;

  v[2].x = destRect.x2;
  v[2].y = destRect.y2;
  v[2].tu = sourceRect.x2 / sourceWidth;
  v[2].tv = sourceRect.y2 / sourceHeight;

  v[3].x = destRect.x1;
  v[3].y = destRect.y2;
  v[3].tu = sourceRect.x1 / sourceWidth;
  v[3].tv = sourceRect.y2 / sourceHeight;

  // -0.5 offset to compensate for D3D rasterization
  // set z and rhw
  for(int i = 0; i < 4; i++)
  {
    v[i].x -= 0.5;
    v[i].y -= 0.5;
    v[i].z = 0.0f;
    v[i].rhw = 1.0f;
  }
}

void CConvolutionShader::SetShaderParameters(CD3DTexture &sourceTexture, float* texSteps, int texStepsCount)
{
  m_effect.SetTechnique( "SCALER_T" );
  m_effect.SetTexture( "g_Texture",  sourceTexture ) ;
  m_effect.SetTexture( "g_KernelTexture", m_HQKernelTexture );
  m_effect.SetFloatArray("g_StepXY", texSteps, texStepsCount);
}

void CConvolutionShader::ReleaseInternal()
{
  if(m_HQKernelTexture.Get())
    m_HQKernelTexture.Release();
  CWinShader::ReleaseInternal();
}

//==========================================================

bool CTestShader::Create()
{
  CStdString effectString = "special://xbmc/system/shaders/testshader.fx";

  if(!LoadEffect(effectString, NULL))
  {
    CLog::Log(LOGERROR, __FUNCTION__": Failed to load shader %s.", effectString.c_str());
    return false;
  }
  return true;
}
