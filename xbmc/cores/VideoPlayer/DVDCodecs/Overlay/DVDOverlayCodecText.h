/*
 *  Copyright (c) 2020 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#pragma once

#include "DVDOverlayCodec.h"

class CDVDOverlayText;

class CDVDOverlayCodecText : public CDVDOverlayCodec
{
public:
  CDVDOverlayCodecText();
  ~CDVDOverlayCodecText() override;
  bool Open(CDVDStreamInfo &hints, CDVDCodecOptions &options) override;
  void Dispose() override;
  int Decode(DemuxPacket *pPacket) override;
  void Reset() override;
  void Flush() override;
  CDVDOverlay* GetOverlay() override;

private:
  bool             m_bIsSSA;
  CDVDOverlayText* m_pOverlay;
};
