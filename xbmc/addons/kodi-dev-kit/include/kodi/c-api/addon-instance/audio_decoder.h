/*
 *  Copyright (C) 2005-2020 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#ifndef C_API_ADDONINSTANCE_AUDIO_DECODER_H
#define C_API_ADDONINSTANCE_AUDIO_DECODER_H

#include "../addon_base.h"
#include "../audio_engine.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

  typedef void* KODI_ADDON_AUDIODECODER_HDL;

  struct KODI_ADDON_AUDIODECODER_INFO_TAG
  {
    char* title;
    char* artist;
    char* album;
    char* album_artist;
    char* media_type;
    char* genre;
    int duration;
    int track;
    int disc;
    char* disc_subtitle;
    int disc_total;
    char* release_date;
    char* lyrics;
    int samplerate;
    int channels;
    int bitrate;
    char* comment;
  };

  typedef bool(ATTR_APIENTRYP PFN_KODI_ADDON_AUDIODECODER_INIT_V1)(
      const KODI_ADDON_AUDIODECODER_HDL hdl,
      const char* file,
      unsigned int filecache,
      int* channels,
      int* samplerate,
      int* bitspersample,
      int64_t* totaltime,
      int* bitrate,
      enum AudioEngineDataFormat* format,
      enum AudioEngineChannel info[AUDIOENGINE_CH_MAX]);
  typedef int(ATTR_APIENTRYP PFN_KODI_ADDON_AUDIODECODER_READ_PCM_V1)(
      const KODI_ADDON_AUDIODECODER_HDL hdl, uint8_t* buffer, size_t size, size_t* actualsize);
  typedef int64_t(ATTR_APIENTRYP PFN_KODI_ADDON_AUDIODECODER_SEEK_V1)(
      const KODI_ADDON_AUDIODECODER_HDL hdl, int64_t time);
  typedef bool(ATTR_APIENTRYP PFN_KODI_ADDON_AUDIODECODER_READ_TAG_V1)(
      const KODI_ADDON_AUDIODECODER_HDL hdl,
      const char* file,
      struct KODI_ADDON_AUDIODECODER_INFO_TAG* tag);
  typedef int(ATTR_APIENTRYP PFN_KODI_ADDON_AUDIODECODER_TRACK_COUNT_V1)(
      const KODI_ADDON_AUDIODECODER_HDL hdl, const char* file);

  typedef struct AddonToKodiFuncTable_AudioDecoder
  {
    KODI_HANDLE kodiInstance;
  } AddonToKodiFuncTable_AudioDecoder;

  typedef struct KodiToAddonFuncTable_AudioDecoder
  {
    KODI_HANDLE addonInstance;
    PFN_KODI_ADDON_AUDIODECODER_INIT_V1 init;
    PFN_KODI_ADDON_AUDIODECODER_READ_PCM_V1 read_pcm;
    PFN_KODI_ADDON_AUDIODECODER_SEEK_V1 seek;
    PFN_KODI_ADDON_AUDIODECODER_READ_TAG_V1 read_tag;
    PFN_KODI_ADDON_AUDIODECODER_TRACK_COUNT_V1 track_count;
  } KodiToAddonFuncTable_AudioDecoder;

  typedef struct AddonInstance_AudioDecoder
  {
    struct AddonToKodiFuncTable_AudioDecoder* toKodi;
    struct KodiToAddonFuncTable_AudioDecoder* toAddon;
  } AddonInstance_AudioDecoder;

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* !C_API_ADDONINSTANCE_AUDIO_DECODER_H */
