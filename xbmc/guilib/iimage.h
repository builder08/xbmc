/*
 *  Copyright (C) 2012-2018 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#pragma once

#include "guilib/TextureFormats.h"

#include <string>

class IImage
{
public:

  virtual ~IImage() = default;

  /*!
   \brief Load an image from memory with the format m_strMimeType to determine it's size and orientation
   \param buffer The memory location where the image data can be found
   \param bufSize The size of the buffer
   \param width The ideal width of the texture
   \param height The ideal height of the texture
   \return true if the image could be loaded
   */
  virtual bool LoadImageFromMemory(unsigned char* buffer, unsigned int bufSize, unsigned int width, unsigned int height)=0;
  /*!
   \brief Check if a Kodi texture format is compatible
   \param format The format to check
   \return true if the decoder can write such a texture
   */
  virtual bool IsFormatSupported(uint32_t format) { return format == XB_FMT_A8R8G8B8; }
  /*!
   \brief Reports to what format the image could be decoded into
   \return the Kodi texture format closest to the source format
   */
  uint32_t GetCompatibleFormat() const { return m_format; }
  /*!
   \brief Decodes the previously loaded image data to the output buffer in 32 bit raw bits
   \param pixels The output buffer
   \param width The width of the image
   \param height The height of the image
   \param pitch The pitch of the output buffer
   \param format The format of the output buffer (JpegIO only)
   \return true if the image data could be decoded to the output buffer
   */
  virtual bool Decode(unsigned char* const pixels, unsigned int width, unsigned int height, unsigned int pitch, unsigned int format)=0;
  /*!
   \brief Encodes an thumbnail from raw bits of given memory location
   \remarks Caller need to call ReleaseThumbnailBuffer() afterwards to free the output buffer
   \param bufferin The memory location where the image data can be found
   \param width The width of the thumbnail
   \param height The height of the thumbnail
   \param format The format of the input buffer (JpegIO only)
   \param pitch The pitch of the input texture stored in bufferin
   \param destFile The destination path of the thumbnail to determine the image format from the extension
   \param bufferout The output buffer (will be allocated inside the method)
   \param bufferoutSize The output buffer size
   \return true if the thumbnail was successfully created
   */
  virtual bool CreateThumbnailFromSurface(unsigned char* bufferin, unsigned int width, unsigned int height, unsigned int format, unsigned int pitch, const std::string& destFile,
                                          unsigned char* &bufferout, unsigned int &bufferoutSize)=0;
  /*!
   \brief Frees the output buffer allocated by CreateThumbnailFromSurface
   */
  virtual void ReleaseThumbnailBuffer() {}

  unsigned int Width() const              { return m_width; }
  unsigned int Height() const             { return m_height; }
  unsigned int originalWidth() const      { return m_originalWidth; }
  unsigned int originalHeight() const     { return m_originalHeight; }
  unsigned int Orientation() const        { return m_orientation; }
  bool hasAlpha() const                   { return m_hasAlpha; }

protected:

  unsigned int m_width = 0;
  unsigned int m_height = 0;
  unsigned int m_originalWidth = 0;   ///< original image width before scaling or cropping
  unsigned int m_originalHeight = 0;  ///< original image height before scaling or cropping
  unsigned int m_orientation = 0;
  bool m_hasAlpha = false;
  uint32_t m_format{XB_FMT_A8R8G8B8};
};
