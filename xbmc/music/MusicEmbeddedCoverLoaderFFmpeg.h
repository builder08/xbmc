/*
 *  Copyright (C) 2005-2025 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#pragma once

#include "cores/FFmpeg.h"
#include "tags/MusicInfoTag.h"

namespace MUSIC_INFO
{
class CMusicEmbeddedCoverLoaderFFmpeg
{
public:
  static void GetEmbeddedCover(AVFormatContext* fctx,
                               CMusicInfoTag& tag,
                               EmbeddedArt* art = nullptr);
};
} // namespace MUSIC_INFO
