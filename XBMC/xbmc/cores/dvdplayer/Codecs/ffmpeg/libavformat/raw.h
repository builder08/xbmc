/*
 * RAW muxer and demuxer
 * Copyright (C) 2007  Aurelien Jacobs <aurel@gnuage.org>
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

#ifndef AVFORMAT_RAW_H
#define AVFORMAT_RAW_H

#include "avformat.h"

int pcm_read_seek(AVFormatContext *s,
                  int stream_index, int64_t timestamp, int flags);

int ff_raw_read_partial_packet(AVFormatContext *s, AVPacket *pkt);

#endif /* AVFORMAT_RAW_H */
