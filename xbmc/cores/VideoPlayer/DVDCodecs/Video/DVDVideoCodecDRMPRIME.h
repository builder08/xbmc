/*
 *  Copyright (c) 2020 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#pragma once

#include "cores/VideoPlayer/DVDCodecs/Video/DVDVideoCodec.h"
#include "cores/VideoPlayer/DVDStreamInfo.h"
#include "cores/VideoPlayer/Process/VideoBuffer.h"

#include <memory>

class CDVDVideoCodecDRMPRIME : public CDVDVideoCodec
{
public:
  explicit CDVDVideoCodecDRMPRIME(CProcessInfo& processInfo);
  ~CDVDVideoCodecDRMPRIME() override;

  static CDVDVideoCodec* Create(CProcessInfo& processInfo);
  static void Register();

  bool Open(CDVDStreamInfo& hints, CDVDCodecOptions& options) override;
  bool AddData(const DemuxPacket& packet) override;
  void Reset() override;
  CDVDVideoCodec::VCReturn GetPicture(VideoPicture* pVideoPicture) override;
  const char* GetName() override { return m_name.c_str(); }
  unsigned GetAllowedReferences() override { return 5; }
  void SetCodecControl(int flags) override { m_codecControlFlags = flags; }

protected:
  void Drain();
  void SetPictureParams(VideoPicture* pVideoPicture);
  void UpdateProcessInfo(struct AVCodecContext* avctx, const enum AVPixelFormat fmt);
  static enum AVPixelFormat GetFormat(struct AVCodecContext* avctx, const enum AVPixelFormat* fmt);

  std::string m_name;
  int m_codecControlFlags = 0;
  CDVDStreamInfo m_hints;
  AVCodecContext* m_pCodecContext = nullptr;
  AVFrame* m_pFrame = nullptr;
  std::shared_ptr<IVideoBufferPool> m_videoBufferPool;
};
