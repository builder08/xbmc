/*
 *  Copyright (C) 2017-2018 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#include "RPRendererOpenGL.h"

#include "cores/RetroPlayer/buffers/RenderBufferOpenGL.h"
#include "cores/RetroPlayer/buffers/RenderBufferPoolOpenGL.h"
#include "cores/RetroPlayer/rendering/RenderContext.h"
#include "utils/GLUtils.h"
#include "utils/log.h"

#include <cstddef>

using namespace KODI;
using namespace RETRO;

// --- CRendererFactoryOpenGL --------------------------------------------------

std::string CRendererFactoryOpenGL::RenderSystemName() const
{
  return "OpenGL";
}

CRPBaseRenderer* CRendererFactoryOpenGL::CreateRenderer(
    const CRenderSettings& settings,
    CRenderContext& context,
    std::shared_ptr<IRenderBufferPool> bufferPool)
{
  return new CRPRendererOpenGL(settings, context, std::move(bufferPool));
}

RenderBufferPoolVector CRendererFactoryOpenGL::CreateBufferPools(CRenderContext& context)
{
  return {std::make_shared<CRenderBufferPoolOpenGL>()};
}

// --- CRPRendererOpenGL -------------------------------------------------------

CRPRendererOpenGL::CRPRendererOpenGL(const CRenderSettings& renderSettings,
                                     CRenderContext& context,
                                     std::shared_ptr<IRenderBufferPool> bufferPool)
  : CRPBaseRenderer(renderSettings, context, std::move(bufferPool))
{
  // Initialize CRPRendererOpenGL
  m_clearColour = m_context.UseLimitedColor() ? (16.0f / 0xff) : 0.0f;

  // Set up main screen VAO/VBOs
  gl::GenVertexArrays(1, &m_mainVAO);
  gl::BindVertexArray(m_mainVAO);

  gl::GenBuffers(1, &m_mainVertexVBO);
  gl::GenBuffers(1, &m_mainIndexVBO);

  m_context.EnableGUIShader(GL_SHADER_METHOD::TEXTURE);
  GLint vertLoc = m_context.GUIShaderGetPos();
  GLint loc = m_context.GUIShaderGetCoord0();
  m_context.DisableGUIShader();

  gl::BindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_mainIndexVBO);
  gl::BindBuffer(GL_ARRAY_BUFFER, m_mainVertexVBO);
  gl::EnableVertexAttribArray(vertLoc);
  gl::VertexAttribPointer(vertLoc, 3, GL_FLOAT, 0, sizeof(PackedVertex),
                          reinterpret_cast<const GLvoid*>(offsetof(PackedVertex, x)));
  gl::EnableVertexAttribArray(loc);
  gl::VertexAttribPointer(loc, 2, GL_FLOAT, 0, sizeof(PackedVertex),
                          reinterpret_cast<const GLvoid*>(offsetof(PackedVertex, u1)));

  // Set up black bars VAO/VBO
  gl::GenVertexArrays(1, &m_blackbarsVAO);
  gl::BindVertexArray(m_blackbarsVAO);

  gl::GenBuffers(1, &m_blackbarsVertexVBO);
  gl::BindBuffer(GL_ARRAY_BUFFER, m_blackbarsVertexVBO);

  m_context.EnableGUIShader(GL_SHADER_METHOD::DEFAULT);
  GLint posLoc = m_context.GUIShaderGetPos();
  m_context.DisableGUIShader();

  gl::EnableVertexAttribArray(posLoc);
  gl::VertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, sizeof(Svertex), 0);

  // Unbind everything just to be safe
  gl::BindVertexArray(0);
  gl::BindBuffer(GL_ARRAY_BUFFER, 0);
  gl::BindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

CRPRendererOpenGL::~CRPRendererOpenGL()
{
  gl::DeleteBuffers(1, &m_mainIndexVBO);
  gl::DeleteBuffers(1, &m_mainVertexVBO);
  gl::DeleteBuffers(1, &m_blackbarsVertexVBO);

  gl::DeleteVertexArrays(1, &m_mainVAO);
  gl::DeleteVertexArrays(1, &m_blackbarsVAO);
}

void CRPRendererOpenGL::RenderInternal(bool clear, uint8_t alpha)
{
  if (clear)
  {
    if (alpha == 255)
      DrawBlackBars();
    else
      ClearBackBuffer();
  }

  if (alpha < 255)
  {
    gl::Enable(GL_BLEND);
    gl::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  }
  else
  {
    gl::Disable(GL_BLEND);
  }

  Render(alpha);

  gl::Enable(GL_BLEND);
  gl::Flush();
}

void CRPRendererOpenGL::FlushInternal()
{
  if (!m_bConfigured)
    return;

  gl::Finish();
}

bool CRPRendererOpenGL::Supports(RENDERFEATURE feature) const
{
  return feature == RENDERFEATURE::STRETCH || feature == RENDERFEATURE::ZOOM ||
         feature == RENDERFEATURE::PIXEL_RATIO || feature == RENDERFEATURE::ROTATION;
}

bool CRPRendererOpenGL::SupportsScalingMethod(SCALINGMETHOD method)
{
  return method == SCALINGMETHOD::NEAREST || method == SCALINGMETHOD::LINEAR;
}

void CRPRendererOpenGL::ClearBackBuffer()
{
  gl::ClearColor(m_clearColour, m_clearColour, m_clearColour, 0.0f);
  gl::Clear(GL_COLOR_BUFFER_BIT);
  gl::ClearColor(0.0f, 0.0f, 0.0f, 0.0f);
}

void CRPRendererOpenGL::DrawBlackBars()
{
  gl::Disable(GL_BLEND);

  Svertex vertices[24];
  GLubyte count = 0;

  m_context.EnableGUIShader(GL_SHADER_METHOD::DEFAULT);
  GLint uniCol = m_context.GUIShaderGetUniCol();

  gl::Uniform4f(uniCol, m_clearColour / 255.0f, m_clearColour / 255.0f, m_clearColour / 255.0f,
                1.0f);

  // top quad
  if (m_rotatedDestCoords[0].y > 0.0f)
  {
    GLubyte quad = count;
    vertices[quad].x = 0.0;
    vertices[quad].y = 0.0;
    vertices[quad].z = 0;
    vertices[quad + 1].x = m_context.GetScreenWidth();
    vertices[quad + 1].y = 0;
    vertices[quad + 1].z = 0;
    vertices[quad + 2].x = m_context.GetScreenWidth();
    vertices[quad + 2].y = m_rotatedDestCoords[0].y;
    vertices[quad + 2].z = 0;
    vertices[quad + 3] = vertices[quad + 2];
    vertices[quad + 4].x = 0;
    vertices[quad + 4].y = m_rotatedDestCoords[0].y;
    vertices[quad + 4].z = 0;
    vertices[quad + 5] = vertices[quad];
    count += 6;
  }

  // bottom quad
  if (m_rotatedDestCoords[2].y < m_context.GetScreenHeight())
  {
    GLubyte quad = count;
    vertices[quad].x = 0.0;
    vertices[quad].y = m_rotatedDestCoords[2].y;
    vertices[quad].z = 0;
    vertices[quad + 1].x = m_context.GetScreenWidth();
    vertices[quad + 1].y = m_rotatedDestCoords[2].y;
    vertices[quad + 1].z = 0;
    vertices[quad + 2].x = m_context.GetScreenWidth();
    vertices[quad + 2].y = m_context.GetScreenHeight();
    vertices[quad + 2].z = 0;
    vertices[quad + 3] = vertices[quad + 2];
    vertices[quad + 4].x = 0;
    vertices[quad + 4].y = m_context.GetScreenHeight();
    vertices[quad + 4].z = 0;
    vertices[quad + 5] = vertices[quad];
    count += 6;
  }

  // left quad
  if (m_rotatedDestCoords[0].x > 0.0f)
  {
    GLubyte quad = count;
    vertices[quad].x = 0.0;
    vertices[quad].y = m_rotatedDestCoords[0].y;
    vertices[quad].z = 0;
    vertices[quad + 1].x = m_rotatedDestCoords[0].x;
    vertices[quad + 1].y = m_rotatedDestCoords[0].y;
    vertices[quad + 1].z = 0;
    vertices[quad + 2].x = m_rotatedDestCoords[3].x;
    vertices[quad + 2].y = m_rotatedDestCoords[3].y;
    vertices[quad + 2].z = 0;
    vertices[quad + 3] = vertices[quad + 2];
    vertices[quad + 4].x = 0;
    vertices[quad + 4].y = m_rotatedDestCoords[3].y;
    vertices[quad + 4].z = 0;
    vertices[quad + 5] = vertices[quad];
    count += 6;
  }

  // right quad
  if (m_rotatedDestCoords[2].x < m_context.GetScreenWidth())
  {
    GLubyte quad = count;
    vertices[quad].x = m_rotatedDestCoords[1].x;
    vertices[quad].y = m_rotatedDestCoords[1].y;
    vertices[quad].z = 0;
    vertices[quad + 1].x = m_context.GetScreenWidth();
    vertices[quad + 1].y = m_rotatedDestCoords[1].y;
    vertices[quad + 1].z = 0;
    vertices[quad + 2].x = m_context.GetScreenWidth();
    vertices[quad + 2].y = m_rotatedDestCoords[2].y;
    vertices[quad + 2].z = 0;
    vertices[quad + 3] = vertices[quad + 2];
    vertices[quad + 4].x = m_rotatedDestCoords[1].x;
    vertices[quad + 4].y = m_rotatedDestCoords[2].y;
    vertices[quad + 4].z = 0;
    vertices[quad + 5] = vertices[quad];
    count += 6;
  }

  gl::BindVertexArray(m_blackbarsVAO);

  gl::BindBuffer(GL_ARRAY_BUFFER, m_blackbarsVertexVBO);
  gl::BufferData(GL_ARRAY_BUFFER, sizeof(Svertex) * count, &vertices[0], GL_STATIC_DRAW);

  gl::DrawArrays(GL_TRIANGLES, 0, count);

  // Unbind VAO/VBO just to be safe
  gl::BindVertexArray(0);
  gl::BindBuffer(GL_ARRAY_BUFFER, 0);

  m_context.DisableGUIShader();
}

void CRPRendererOpenGL::Render(uint8_t alpha)
{
  CRenderBufferOpenGL* renderBuffer = static_cast<CRenderBufferOpenGL*>(m_renderBuffer);

  if (renderBuffer == nullptr)
    return;

  CRect rect = m_sourceRect;

  rect.x1 /= renderBuffer->GetWidth();
  rect.x2 /= renderBuffer->GetWidth();
  rect.y1 /= renderBuffer->GetHeight();
  rect.y2 /= renderBuffer->GetHeight();

  const uint32_t color = (alpha << 24) | 0xFFFFFF;

  gl::BindTexture(m_textureTarget, renderBuffer->TextureID());

  GLint filter = GL_NEAREST;
  if (GetRenderSettings().VideoSettings().GetScalingMethod() == SCALINGMETHOD::LINEAR)
    filter = GL_LINEAR;
  gl::TexParameteri(m_textureTarget, GL_TEXTURE_MAG_FILTER, filter);
  gl::TexParameteri(m_textureTarget, GL_TEXTURE_MIN_FILTER, filter);
  gl::TexParameteri(m_textureTarget, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  gl::TexParameteri(m_textureTarget, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  m_context.EnableGUIShader(GL_SHADER_METHOD::TEXTURE);

  GLubyte colour[4];
  GLubyte idx[4] = {0, 1, 3, 2}; // Determines order of triangle strip
  PackedVertex vertex[4];

  GLint uniColLoc = m_context.GUIShaderGetUniCol();

  // Setup color values
  colour[0] = UTILS::GL::GetChannelFromARGB(UTILS::GL::ColorChannel::R, color);
  colour[1] = UTILS::GL::GetChannelFromARGB(UTILS::GL::ColorChannel::G, color);
  colour[2] = UTILS::GL::GetChannelFromARGB(UTILS::GL::ColorChannel::B, color);
  colour[3] = UTILS::GL::GetChannelFromARGB(UTILS::GL::ColorChannel::A, color);

  for (unsigned int i = 0; i < 4; i++)
  {
    // Setup vertex position values
    vertex[i].x = m_rotatedDestCoords[i].x;
    vertex[i].y = m_rotatedDestCoords[i].y;
    vertex[i].z = 0.0f;
  }

  // Setup texture coordinates
  vertex[0].u1 = vertex[3].u1 = rect.x1;
  vertex[0].v1 = vertex[1].v1 = rect.y1;
  vertex[1].u1 = vertex[2].u1 = rect.x2;
  vertex[2].v1 = vertex[3].v1 = rect.y2;

  gl::BindVertexArray(m_mainVAO);

  gl::BindBuffer(GL_ARRAY_BUFFER, m_mainVertexVBO);
  gl::BufferData(GL_ARRAY_BUFFER, sizeof(PackedVertex) * 4, &vertex[0], GL_STATIC_DRAW);

  // No need to bind the index VBO, it's part of VAO state
  gl::BufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLubyte) * 4, idx, GL_STATIC_DRAW);

  gl::Uniform4f(uniColLoc, (colour[0] / 255.0f), (colour[1] / 255.0f), (colour[2] / 255.0f),
                (colour[3] / 255.0f));

  gl::DrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_BYTE, 0);

  // Unbind VAO/VBO just to be safe
  gl::BindVertexArray(0);
  gl::BindBuffer(GL_ARRAY_BUFFER, 0);

  m_context.DisableGUIShader();
}
