/*
 *  Copyright (C) 2005-2018 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#pragma once

#include "IEncoder.h"

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
}

class CEncoderFFmpeg : public IEncoder
{
public:
  CEncoderFFmpeg();
  ~CEncoderFFmpeg() override = default;

  bool Init(AddonToKodiFuncTable_AudioEncoder& callbacks) override;
  int Encode(int nNumBytesRead, uint8_t* pbtStream) override;
  bool Close() override;

private:
  static int avio_write_callback(void* opaque, uint8_t* buf, int buf_size);
  static int64_t avio_seek_callback(void* opaque, int64_t offset, int whence);
  void SetTag(const std::string& tag, const std::string& value);

  AVFormatContext* m_Format;
  AVCodecContext* m_CodecCtx;
  SwrContext* m_SwrCtx;
  AVStream* m_Stream;
  AVSampleFormat m_InFormat;
  AVSampleFormat m_OutFormat;

  /* From libavformat/avio.h:
   * The buffer size is very important for performance.
   * For protocols with fixed blocksize it should be set to this
   * blocksize.
   * For others a typical size is a cache page, e.g. 4kb.
   */
  unsigned char m_BCBuffer[4096];

  unsigned int m_NeededFrames;
  unsigned int m_NeededBytes;
  uint8_t* m_Buffer;
  unsigned int m_BufferSize = 0;
  AVFrame* m_BufferFrame;
  uint8_t* m_ResampledBuffer;
  unsigned int m_ResampledBufferSize = 0;
  AVFrame* m_ResampledFrame;
  bool m_NeedConversion = false;

  AddonToKodiFuncTable_AudioEncoder m_callbacks;

  bool WriteFrame();
};
