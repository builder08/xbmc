/*
*      Copyright (C) 2005-2008 Team XBMC
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

#include "include.h"
#include "TextureManagerGL.h"
#include "AnimatedGif.h"
#include "GraphicContext.h"
#include "Surface.h"
#include "../xbmc/Picture.h"
#include "utils/SingleLock.h"
#include "StringUtils.h"
#include "utils/CharsetConverter.h"
#include "../xbmc/Util.h"
#include "../xbmc/FileSystem/File.h"
#include "../xbmc/FileSystem/Directory.h"
#include "../xbmc/FileSystem/SpecialProtocol.h"

#ifdef HAS_SDL_OPENGL

#define MAX_PICTURE_WIDTH  2048
#define MAX_PICTURE_HEIGHT 2048

using namespace std;

extern "C" void dllprintf( const char *format, ... );

DWORD PadPow2(DWORD x) 
{
  --x;
  x |= x >> 1;
  x |= x >> 2;
  x |= x >> 4;
  x |= x >> 8;
  x |= x >> 16;
  return ++x;
}


/************************************************************************/
/*    CSDLTexture                                                       */
/************************************************************************/
CGLTexture::CGLTexture()
: CBaseTexture()
{

}

CGLTexture::CGLTexture(XBMC::SurfacePtr surface, bool load, bool freeSurface) 
: CBaseTexture()
{
  int vmaj, vmin;

  g_graphicsContext.GetRenderVersion(vmaj, vmin);    

  if (vmaj>=2 && GLEW_ARB_texture_non_power_of_two)
    m_bRequiresPower2Textures = false;
  else
    m_bRequiresPower2Textures = true;

  Update(surface, load, freeSurface);
}


CGLTexture::~CGLTexture()
{
  g_graphicsContext.BeginPaint();
  if (glIsTexture(id)) {
    glDeleteTextures(1, &((GLuint)id));
  }
  g_graphicsContext.EndPaint();

  if (m_pixels)
    delete [] m_pixels;

  m_pixels=NULL;

  id = 0;
}

bool CGLTexture::Load(const CStdString& texturePath)
{
  CPicture pic;
  XBMC::TexturePtr original = pic.Load(texturePath, MAX_PICTURE_WIDTH, MAX_PICTURE_HEIGHT);
  if (!original)
  {
    CLog::Log(LOGERROR, "Texture manager unable to load file: %s", texturePath.c_str());
    return 0;
  }
  // make sure the texture format is correct
  SDL_PixelFormat format;
  format.palette = 0; format.colorkey = 0; format.alpha = 0;
  format.BitsPerPixel = 32; format.BytesPerPixel = 4;
  format.Amask = AMASK; format.Ashift = PIXEL_ASHIFT;
  format.Rmask = RMASK; format.Rshift = PIXEL_RSHIFT;
  format.Gmask = GMASK; format.Gshift = PIXEL_GSHIFT;
  format.Bmask = BMASK; format.Bshift = PIXEL_BSHIFT;

  XBMC::TexturePtr pTexture = SDL_ConvertSurface(original, &format, SDL_SWSURFACE);

  DELETE_TEXTURE(original);
  if (!pTexture)
  {
    CLog::Log(LOGERROR, "Texture manager unable to load file: %s", texturePath.c_str());
    return 0;
  }

  Update(pTexture, false, false);
  DELETE_TEXTURE(pTexture);

  return true;
}

void CGLTexture::Update(XBMC::TexturePtr surface, bool loadToGPU, bool freeSurface)
{
  SDL_LockSurface(surface);
  Update(surface->w, surface->h, surface->pitch, (unsigned char *)surface->pixels, loadToGPU);
  SDL_UnlockSurface(surface);

  if (freeSurface)
    DELETE_TEXTURE(surface);
}

void CGLTexture::Update(int w, int h, int pitch, const unsigned char *pixels, bool loadToGPU) 
{
  int tpitch;

  if (m_pixels)
    delete [] m_pixels;

  imageWidth = w;
  imageHeight = h;

  if (!m_bRequiresPower2Textures)
  {
    textureWidth = imageWidth;
    textureHeight = imageHeight;
  }
  else
  {
    textureWidth = PadPow2(imageWidth);
    textureHeight = PadPow2(imageHeight);
  }

  // Resize texture to POT
  const unsigned char *src = pixels;
  tpitch = min(pitch,textureWidth*4);
  m_pixels = new unsigned char[textureWidth * textureHeight * 4];
  unsigned char* resized = m_pixels;

  for (int y = 0; y < h; y++)
  {
    memcpy(resized, src, tpitch); // make sure pitch is not bigger than our width
    src += pitch;

    // repeat last column to simulate clamp_to_edge
    for(int i = tpitch; i < textureWidth*4; i+=4)
      memcpy(resized+i, src-4, 4);

    resized += (textureWidth * 4);
  }

  // repeat last row to simulate clamp_to_edge
  for(int y = h; y < textureHeight; y++) 
  {
    memcpy(resized, src - tpitch, tpitch);

    // repeat last column to simulate clamp_to_edge
    for(int i = tpitch; i < textureWidth*4; i+=4) 
      memcpy(resized+i, src-4, 4);

    resized += (textureWidth * 4);
  }
  if (loadToGPU)
    LoadToGPU();
}


void CGLTexture::LoadToGPU()
{
  if (!m_pixels) {
    // nothing to load - probably same image (no change)
    return;
  }

  g_graphicsContext.BeginPaint();
  if (!m_loadedToGPU) {
    // Have OpenGL generate a texture object handle for us
    // this happens only one time - the first time the texture is loaded
    glGenTextures(1, &id);
  }

  // Bind the texture object
  glBindTexture(GL_TEXTURE_2D, id);

  // Set the texture's stretching properties
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  static GLint maxSize = g_graphicsContext.GetMaxTextureSize();
  {
    if (textureHeight>maxSize)
    {
      CLog::Log(LOGERROR, "GL: Image height %d too big to fit into single texture unit, truncating to %d", textureHeight, (int) maxSize);
      textureHeight = maxSize;
    }
    if (textureWidth>maxSize)
    {
      CLog::Log(LOGERROR, "GL: Image width %d too big to fit into single texture unit, truncating to %d", textureWidth, (int) maxSize);
      glPixelStorei(GL_UNPACK_ROW_LENGTH, textureWidth);
      textureWidth = maxSize;
    }
  }
  //CLog::Log(LOGNOTICE, "Texture width x height: %d x %d", textureWidth, textureHeight);
  glTexImage2D(GL_TEXTURE_2D, 0, 4, textureWidth, textureHeight, 0,
    GL_BGRA, GL_UNSIGNED_BYTE, m_pixels);
  glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
  VerifyGLState();

  g_graphicsContext.EndPaint();
  delete [] m_pixels;
  m_pixels = NULL;

  m_loadedToGPU = true;           
}

#endif // HAS_SDL_OPENGL