/*
 * Matroska constants
 * Copyright (c) 2003-2004 The ffmpeg Project
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef AVFORMAT_MATROSKA_H
#define AVFORMAT_MATROSKA_H

#include "libavcodec/avcodec.h"
#include "metadata.h"
#include "internal.h"

/* EBML version supported */
#define EBML_VERSION 1

/* top-level master-IDs */
#define EBML_ID_HEADER             0x1A45DFA3

/* IDs in the HEADER master */
#define EBML_ID_EBMLVERSION        0x4286
#define EBML_ID_EBMLREADVERSION    0x42F7
#define EBML_ID_EBMLMAXIDLENGTH    0x42F2
#define EBML_ID_EBMLMAXSIZELENGTH  0x42F3
#define EBML_ID_DOCTYPE            0x4282
#define EBML_ID_DOCTYPEVERSION     0x4287
#define EBML_ID_DOCTYPEREADVERSION 0x4285

/* general EBML types */
#define EBML_ID_VOID               0xEC
#define EBML_ID_CRC32              0xBF

/*
 * Matroska element IDs, max. 32 bits
 */

/* toplevel segment */
#define MATROSKA_ID_SEGMENT    0x18538067

/* Matroska top-level master IDs */
#define MATROSKA_ID_INFO       0x1549A966
#define MATROSKA_ID_TRACKS     0x1654AE6B
#define MATROSKA_ID_CUES       0x1C53BB6B
#define MATROSKA_ID_TAGS       0x1254C367
#define MATROSKA_ID_SEEKHEAD   0x114D9B74
#define MATROSKA_ID_ATTACHMENTS 0x1941A469
#define MATROSKA_ID_CLUSTER    0x1F43B675
#define MATROSKA_ID_CHAPTERS   0x1043A770

/* IDs in the info master */
#define MATROSKA_ID_TIMECODESCALE 0x2AD7B1
#define MATROSKA_ID_DURATION   0x4489
#define MATROSKA_ID_TITLE      0x7BA9
#define MATROSKA_ID_WRITINGAPP 0x5741
#define MATROSKA_ID_MUXINGAPP  0x4D80
#define MATROSKA_ID_DATEUTC    0x4461
#define MATROSKA_ID_SEGMENTUID 0x73A4

/* ID in the tracks master */
#define MATROSKA_ID_TRACKENTRY 0xAE

/* IDs in the trackentry master */
#define MATROSKA_ID_TRACKNUMBER 0xD7
#define MATROSKA_ID_TRACKUID   0x73C5
#define MATROSKA_ID_TRACKTYPE  0x83
#define MATROSKA_ID_TRACKVIDEO     0xE0
#define MATROSKA_ID_TRACKAUDIO     0xE1
#define MATROSKA_ID_TRACKOPERATION 0xE2
#define MATROSKA_ID_TRACKCOMBINEPLANES 0xE3
#define MATROSKA_ID_TRACKPLANE         0xE4
#define MATROSKA_ID_TRACKPLANEUID      0xE5
#define MATROSKA_ID_TRACKPLANETYPE     0xE6
#define MATROSKA_ID_CODECID    0x86
#define MATROSKA_ID_CODECPRIVATE 0x63A2
#define MATROSKA_ID_CODECNAME  0x258688
#define MATROSKA_ID_CODECINFOURL 0x3B4040
#define MATROSKA_ID_CODECDOWNLOADURL 0x26B240
#define MATROSKA_ID_CODECDECODEALL 0xAA
#define MATROSKA_ID_TRACKNAME  0x536E
#define MATROSKA_ID_TRACKLANGUAGE 0x22B59C
#define MATROSKA_ID_TRACKFLAGENABLED 0xB9
#define MATROSKA_ID_TRACKFLAGDEFAULT 0x88
#define MATROSKA_ID_TRACKFLAGFORCED 0x55AA
#define MATROSKA_ID_TRACKFLAGLACING 0x9C
#define MATROSKA_ID_TRACKMINCACHE 0x6DE7
#define MATROSKA_ID_TRACKMAXCACHE 0x6DF8
#define MATROSKA_ID_TRACKDEFAULTDURATION 0x23E383
#define MATROSKA_ID_TRACKCONTENTENCODINGS 0x6D80
#define MATROSKA_ID_TRACKCONTENTENCODING 0x6240
#define MATROSKA_ID_TRACKTIMECODESCALE 0x23314F
#define MATROSKA_ID_TRACKMAXBLKADDID 0x55EE

/* IDs in the trackvideo master */
#define MATROSKA_ID_VIDEOFRAMERATE 0x2383E3
#define MATROSKA_ID_VIDEODISPLAYWIDTH 0x54B0
#define MATROSKA_ID_VIDEODISPLAYHEIGHT 0x54BA
#define MATROSKA_ID_VIDEOPIXELWIDTH 0xB0
#define MATROSKA_ID_VIDEOPIXELHEIGHT 0xBA
#define MATROSKA_ID_VIDEOPIXELCROPB 0x54AA
#define MATROSKA_ID_VIDEOPIXELCROPT 0x54BB
#define MATROSKA_ID_VIDEOPIXELCROPL 0x54CC
#define MATROSKA_ID_VIDEOPIXELCROPR 0x54DD
#define MATROSKA_ID_VIDEODISPLAYUNIT 0x54B2
#define MATROSKA_ID_VIDEOFLAGINTERLACED 0x9A
#define MATROSKA_ID_VIDEOSTEREOMODE 0x53B8
#define MATROSKA_ID_VIDEOALPHAMODE 0x53C0
#define MATROSKA_ID_VIDEOASPECTRATIO 0x54B3
#define MATROSKA_ID_VIDEOCOLORSPACE 0x2EB524

/* IDs in the trackaudio master */
#define MATROSKA_ID_AUDIOSAMPLINGFREQ 0xB5
#define MATROSKA_ID_AUDIOOUTSAMPLINGFREQ 0x78B5

#define MATROSKA_ID_AUDIOBITDEPTH 0x6264
#define MATROSKA_ID_AUDIOCHANNELS 0x9F

/* IDs in the content encoding master */
#define MATROSKA_ID_ENCODINGORDER 0x5031
#define MATROSKA_ID_ENCODINGSCOPE 0x5032
#define MATROSKA_ID_ENCODINGTYPE 0x5033
#define MATROSKA_ID_ENCODINGCOMPRESSION 0x5034
#define MATROSKA_ID_ENCODINGCOMPALGO 0x4254
#define MATROSKA_ID_ENCODINGCOMPSETTINGS 0x4255

/* ID in the cues master */
#define MATROSKA_ID_POINTENTRY 0xBB

/* IDs in the pointentry master */
#define MATROSKA_ID_CUETIME    0xB3
#define MATROSKA_ID_CUETRACKPOSITION 0xB7

/* IDs in the cuetrackposition master */
#define MATROSKA_ID_CUETRACK   0xF7
#define MATROSKA_ID_CUECLUSTERPOSITION 0xF1
#define MATROSKA_ID_CUEBLOCKNUMBER 0x5378

/* IDs in the tags master */
#define MATROSKA_ID_TAG                 0x7373
#define MATROSKA_ID_SIMPLETAG           0x67C8
#define MATROSKA_ID_TAGNAME             0x45A3
#define MATROSKA_ID_TAGSTRING           0x4487
#define MATROSKA_ID_TAGLANG             0x447A
#define MATROSKA_ID_TAGDEFAULT          0x4484
#define MATROSKA_ID_TAGDEFAULT_BUG      0x44B4
#define MATROSKA_ID_TAGTARGETS          0x63C0
#define MATROSKA_ID_TAGTARGETS_TYPE       0x63CA
#define MATROSKA_ID_TAGTARGETS_TYPEVALUE  0x68CA
#define MATROSKA_ID_TAGTARGETS_TRACKUID   0x63C5
#define MATROSKA_ID_TAGTARGETS_CHAPTERUID 0x63C4
#define MATROSKA_ID_TAGTARGETS_ATTACHUID  0x63C6

/* IDs in the seekhead master */
#define MATROSKA_ID_SEEKENTRY  0x4DBB

/* IDs in the seekpoint master */
#define MATROSKA_ID_SEEKID     0x53AB
#define MATROSKA_ID_SEEKPOSITION 0x53AC

/* IDs in the cluster master */
#define MATROSKA_ID_CLUSTERTIMECODE 0xE7
#define MATROSKA_ID_CLUSTERPOSITION 0xA7
#define MATROSKA_ID_CLUSTERPREVSIZE 0xAB
#define MATROSKA_ID_BLOCKGROUP 0xA0
#define MATROSKA_ID_BLOCKADDITIONS 0x75A1
#define MATROSKA_ID_BLOCKMORE 0xA6
#define MATROSKA_ID_BLOCKADDID 0xEE
#define MATROSKA_ID_BLOCKADDITIONAL 0xA5
#define MATROSKA_ID_SIMPLEBLOCK 0xA3

/* IDs in the blockgroup master */
#define MATROSKA_ID_BLOCK      0xA1
#define MATROSKA_ID_BLOCKDURATION 0x9B
#define MATROSKA_ID_BLOCKREFERENCE 0xFB

/* IDs in the attachments master */
#define MATROSKA_ID_ATTACHEDFILE        0x61A7
#define MATROSKA_ID_FILEDESC            0x467E
#define MATROSKA_ID_FILENAME            0x466E
#define MATROSKA_ID_FILEMIMETYPE        0x4660
#define MATROSKA_ID_FILEDATA            0x465C
#define MATROSKA_ID_FILEUID             0x46AE

/* IDs in the chapters master */
#define MATROSKA_ID_EDITIONENTRY        0x45B9
#define MATROSKA_ID_CHAPTERATOM         0xB6
#define MATROSKA_ID_CHAPTERTIMESTART    0x91
#define MATROSKA_ID_CHAPTERTIMEEND      0x92
#define MATROSKA_ID_CHAPTERDISPLAY      0x80
#define MATROSKA_ID_CHAPSTRING          0x85
#define MATROSKA_ID_CHAPLANG            0x437C
#define MATROSKA_ID_EDITIONUID          0x45BC
#define MATROSKA_ID_EDITIONFLAGHIDDEN   0x45BD
#define MATROSKA_ID_EDITIONFLAGDEFAULT  0x45DB
#define MATROSKA_ID_EDITIONFLAGORDERED  0x45DD
#define MATROSKA_ID_CHAPTERUID          0x73C4
#define MATROSKA_ID_CHAPTERFLAGHIDDEN   0x98
#define MATROSKA_ID_CHAPTERFLAGENABLED  0x4598
#define MATROSKA_ID_CHAPTERPHYSEQUIV    0x63C3

typedef enum {
  MATROSKA_TRACK_TYPE_NONE     = 0x0,
  MATROSKA_TRACK_TYPE_VIDEO    = 0x1,
  MATROSKA_TRACK_TYPE_AUDIO    = 0x2,
  MATROSKA_TRACK_TYPE_COMPLEX  = 0x3,
  MATROSKA_TRACK_TYPE_LOGO     = 0x10,
  MATROSKA_TRACK_TYPE_SUBTITLE = 0x11,
  MATROSKA_TRACK_TYPE_CONTROL  = 0x20,
} MatroskaTrackType;

typedef enum {
  MATROSKA_TRACK_ENCODING_COMP_ZLIB        = 0,
  MATROSKA_TRACK_ENCODING_COMP_BZLIB       = 1,
  MATROSKA_TRACK_ENCODING_COMP_LZO         = 2,
  MATROSKA_TRACK_ENCODING_COMP_HEADERSTRIP = 3,
} MatroskaTrackEncodingCompAlgo;

typedef enum {
  MATROSKA_VIDEO_STEREOMODE_TYPE_MONO               = 0,
  MATROSKA_VIDEO_STEREOMODE_TYPE_LEFT_RIGHT         = 1,
  MATROSKA_VIDEO_STEREOMODE_TYPE_BOTTOM_TOP         = 2,
  MATROSKA_VIDEO_STEREOMODE_TYPE_TOP_BOTTOM         = 3,
  MATROSKA_VIDEO_STEREOMODE_TYPE_CHECKERBOARD_RL    = 4,
  MATROSKA_VIDEO_STEREOMODE_TYPE_CHECKERBOARD_LR    = 5,
  MATROSKA_VIDEO_STEREOMODE_TYPE_ROW_INTERLEAVED_RL = 6,
  MATROSKA_VIDEO_STEREOMODE_TYPE_ROW_INTERLEAVED_LR = 7,
  MATROSKA_VIDEO_STEREOMODE_TYPE_COL_INTERLEAVED_RL = 8,
  MATROSKA_VIDEO_STEREOMODE_TYPE_COL_INTERLEAVED_LR = 9,
  MATROSKA_VIDEO_STEREOMODE_TYPE_ANAGLYPH_CYAN_RED  = 10,
  MATROSKA_VIDEO_STEREOMODE_TYPE_RIGHT_LEFT         = 11,
  MATROSKA_VIDEO_STEREOMODE_TYPE_ANAGLYPH_GREEN_MAG = 12,
  MATROSKA_VIDEO_STEREOMODE_TYPE_BOTH_EYES_BLOCK_LR = 13,
  MATROSKA_VIDEO_STEREOMODE_TYPE_BOTH_EYES_BLOCK_RL = 14,
} MatroskaVideoStereoModeType;

/*
 * Matroska Codec IDs, strings
 */

typedef struct CodecTags{
    char str[20];
    enum AVCodecID id;
}CodecTags;

/* max. depth in the EBML tree structure */
#define EBML_MAX_DEPTH 16

#define MATROSKA_VIDEO_STEREO_MODE_COUNT  15
#define MATROSKA_VIDEO_STEREO_PLANE_COUNT  3

extern const CodecTags ff_mkv_codec_tags[];
extern const CodecMime ff_mkv_mime_tags[];
extern const AVMetadataConv ff_mkv_metadata_conv[];
extern const char * const ff_matroska_video_stereo_mode[MATROSKA_VIDEO_STEREO_MODE_COUNT];
extern const char * const ff_matroska_video_stereo_plane[MATROSKA_VIDEO_STEREO_PLANE_COUNT];

#endif /* AVFORMAT_MATROSKA_H */
