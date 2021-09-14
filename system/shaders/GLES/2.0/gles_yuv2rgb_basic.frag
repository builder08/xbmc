/*
 *      Copyright (C) 2010-2013 Team XBMC
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

#version 100

#extension GL_OES_EGL_image_external : require

precision mediump float;

uniform samplerExternalOES m_samp0;

uniform sampler2D m_sampY;
uniform sampler2D m_sampU;
uniform sampler2D m_sampV;
varying vec2 m_cordY;
varying vec2 m_cordU;
varying vec2 m_cordV;
uniform vec2 m_step;
uniform mat4 m_yuvmat;
uniform mat3 m_primMat;
uniform float m_gammaDstInv;
uniform float m_gammaSrc;
uniform float m_toneP1;
uniform vec3 m_coefsDst;
uniform float m_alpha;

void main()
{
  vec4 rgb;
  vec4 yuv;

#if defined(XBMC_YV12)

  yuv = vec4(texture2D(m_sampY, m_cordY).r,
             texture2D(m_sampU, m_cordU).r,
             texture2D(m_sampV, m_cordV).r,
             1.0);

  rgb = m_yuvmat * yuv;

#elif defined(XBMC_NV12)

  yuv = vec4(texture2D(m_sampY, m_cordY).r,
             texture2D(m_sampU, m_cordU).rg,
             1.0);

  rgb = m_yuvmat * yuv;

#elif defined(XBMC_NV12_RRG)

  yuv = vec4(texture2D(m_sampY, m_cordY).r,
             texture2D(m_sampU, m_cordU).r,
             texture2D(m_sampV, m_cordV).g,
             1.0);

  rgb = m_yuvmat * yuv;

#elif defined(XBMC_OES)

  rgb = texture2D(m_samp0, m_cordY);

#endif

  rgb.a = m_alpha;

#if defined(XBMC_COL_CONVERSION)
  rgb.rgb = pow(max(vec3(0), rgb.rgb), vec3(m_gammaSrc));
  rgb.rgb = max(vec3(0), m_primMat * rgb.rgb);
  rgb.rgb = pow(rgb.rgb, vec3(m_gammaDstInv));

#if defined(XBMC_TONE_MAPPING)
  float luma = dot(rgb.rgb, m_coefsDst);
  rgb.rgb *= tonemap(luma) / luma;
#endif

#endif

  gl_FragColor = rgb;
}

