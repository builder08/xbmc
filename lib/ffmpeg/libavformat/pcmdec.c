/*
 * RAW PCM demuxers
 * Copyright (c) 2002 Fabrice Bellard
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

#include "avformat.h"
#include "rawdec.h"
#include "pcm.h"

#define RAW_SAMPLES     1024

static int raw_read_packet(AVFormatContext *s, AVPacket *pkt)
{
    int ret, size, bps;
    //    AVStream *st = s->streams[0];

    size= RAW_SAMPLES*s->streams[0]->codec->block_align;

    ret= av_get_packet(s->pb, pkt, size);

    pkt->stream_index = 0;
    if (ret < 0)
        return ret;

    bps= av_get_bits_per_sample(s->streams[0]->codec->codec_id);
    assert(bps); // if false there IS a bug elsewhere (NOT in this function)
    pkt->dts=
    pkt->pts= pkt->pos*8 / (bps * s->streams[0]->codec->channels);

    return ret;
}

#define PCMDEF(name, long_name, ext, codec) \
AVInputFormat ff_pcm_ ## name ## _demuxer = {\
    #name,\
    NULL_IF_CONFIG_SMALL(long_name),\
    0,\
    NULL,\
    ff_raw_read_header,\
    raw_read_packet,\
    NULL,\
    pcm_read_seek,\
    .flags= AVFMT_GENERIC_INDEX,\
    .extensions = ext,\
    .value = codec,\
};

PCMDEF(f64be, "PCM 64 bit floating-point big-endian format",
       NULL, CODEC_ID_PCM_F64BE)

PCMDEF(f64le, "PCM 64 bit floating-point little-endian format",
       NULL, CODEC_ID_PCM_F64LE)

PCMDEF(f32be, "PCM 32 bit floating-point big-endian format",
       NULL, CODEC_ID_PCM_F32BE)

PCMDEF(f32le, "PCM 32 bit floating-point little-endian format",
       NULL, CODEC_ID_PCM_F32LE)

PCMDEF(s32be, "PCM signed 32 bit big-endian format",
       NULL, CODEC_ID_PCM_S32BE)

PCMDEF(s32le, "PCM signed 32 bit little-endian format",
       NULL, CODEC_ID_PCM_S32LE)

PCMDEF(s24be, "PCM signed 24 bit big-endian format",
       NULL, CODEC_ID_PCM_S24BE)

PCMDEF(s24le, "PCM signed 24 bit little-endian format",
       NULL, CODEC_ID_PCM_S24LE)

PCMDEF(s16be, "PCM signed 16 bit big-endian format",
       AV_NE("sw", NULL), CODEC_ID_PCM_S16BE)

PCMDEF(s16le, "PCM signed 16 bit little-endian format",
       AV_NE(NULL, "sw"), CODEC_ID_PCM_S16LE)

PCMDEF(s8, "PCM signed 8 bit format",
       "sb", CODEC_ID_PCM_S8)

PCMDEF(u32be, "PCM unsigned 32 bit big-endian format",
       NULL, CODEC_ID_PCM_U32BE)

PCMDEF(u32le, "PCM unsigned 32 bit little-endian format",
       NULL, CODEC_ID_PCM_U32LE)

PCMDEF(u24be, "PCM unsigned 24 bit big-endian format",
       NULL, CODEC_ID_PCM_U24BE)

PCMDEF(u24le, "PCM unsigned 24 bit little-endian format",
       NULL, CODEC_ID_PCM_U24LE)

PCMDEF(u16be, "PCM unsigned 16 bit big-endian format",
       AV_NE("uw", NULL), CODEC_ID_PCM_U16BE)

PCMDEF(u16le, "PCM unsigned 16 bit little-endian format",
       AV_NE(NULL, "uw"), CODEC_ID_PCM_U16LE)

PCMDEF(u8, "PCM unsigned 8 bit format",
       "ub", CODEC_ID_PCM_U8)

PCMDEF(alaw, "PCM A-law format",
       "al", CODEC_ID_PCM_ALAW)

PCMDEF(mulaw, "PCM mu-law format",
       "ul", CODEC_ID_PCM_MULAW)
