/*
 *      Copyright (C) 2005-2010 Team XBMC
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

texture g_YTexture;
texture g_UTexture;
texture g_VTexture;
float4x4 g_ColorMatrix;

#ifdef SINGLEPASS

// Color conversion + bilinear resize in one pass

sampler YSampler =
  sampler_state {
    Texture = <g_YTexture>;
    AddressU = CLAMP;
    AddressV = CLAMP;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
  };

#else

// Color conversion only

sampler YSampler =
  sampler_state {
    Texture = <g_YTexture>;
    AddressU = CLAMP;
    AddressV = CLAMP;
    MinFilter = POINT;
    MagFilter = POINT;
  };

#endif

sampler USampler =
  sampler_state {
    Texture = <g_UTexture>;
    AddressU = CLAMP;
    AddressV = CLAMP;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
  };

sampler VSampler =
  sampler_state
  {
    Texture = <g_VTexture>;
    AddressU = CLAMP;
    AddressV = CLAMP;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
  };

struct VS_OUTPUT
{
  float4 Position   : POSITION;
  float2 TextureY   : TEXCOORD0;
  float2 TextureU   : TEXCOORD1;
  float2 TextureV   : TEXCOORD2;
};

struct PS_OUTPUT
{
  float4 RGBColor : COLOR0;
};

PS_OUTPUT YUV2RGB( VS_OUTPUT In)
{
  PS_OUTPUT OUT;
#if defined(XBMC_YV12)
  float4 YUV = float4(tex2D (YSampler, In.TextureY).x
                    , tex2D (USampler, In.TextureU).x
                    , tex2D (VSampler, In.TextureV).x
                    , 1.0);
#elif defined(XBMC_NV12)
  float4 YUV = float4(tex2D (YSampler, In.TextureY).x
                    , tex2D (USampler, In.TextureU).ra
                    , 1.0);
#endif
  OUT.RGBColor = mul(YUV, g_ColorMatrix);
  OUT.RGBColor.a = 1.0;
  return OUT;
}

technique YUV2RGB_T
{
  pass P0
  {
    PixelShader  = compile ps_2_0 YUV2RGB();
    ZEnable = False;
    FillMode = Solid;
    FogEnable = False;
  }
};
