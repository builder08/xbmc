/*
 * General DV muxer/demuxer
 * Copyright (c) 2003 Roman Shaposhnik
 *
 * Many thanks to Dan Dennedy <dan@dennedy.org> for providing wealth
 * of DV technical info.
 *
 * Raw DV format
 * Copyright (c) 2002 Fabrice Bellard.
 *
 * 50 Mbps (DVCPRO50) support
 * Copyright (c) 2006 Daniel Maas <dmaas@maasdigital.com>
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
#include <time.h>
#include <stdarg.h>
#include "avformat.h"
#include "dvdata.h"
#include "dv.h"
#include "fifo.h"

struct DVMuxContext {
    const DVprofile*  sys;    /* Current DV profile. E.g.: 525/60, 625/50 */
    int         n_ast;        /* Number of stereo audio streams (up to 2) */
    AVStream   *ast[2];       /* Stereo audio streams */
    AVFifoBuffer  audio_data[2]; /* Fifo for storing excessive amounts of PCM */
    int         frames;       /* Number of a current frame */
    time_t      start_time;   /* Start time of recording */
    int         has_audio;    /* frame under contruction has audio */
    int         has_video;    /* frame under contruction has video */
    uint8_t     frame_buf[DV_MAX_FRAME_SIZE]; /* frame under contruction */
};

static const int dv_aaux_packs_dist[12][9] = {
    { 0xff, 0xff, 0xff, 0x50, 0x51, 0x52, 0x53, 0xff, 0xff },
    { 0x50, 0x51, 0x52, 0x53, 0xff, 0xff, 0xff, 0xff, 0xff },
    { 0xff, 0xff, 0xff, 0x50, 0x51, 0x52, 0x53, 0xff, 0xff },
    { 0x50, 0x51, 0x52, 0x53, 0xff, 0xff, 0xff, 0xff, 0xff },
    { 0xff, 0xff, 0xff, 0x50, 0x51, 0x52, 0x53, 0xff, 0xff },
    { 0x50, 0x51, 0x52, 0x53, 0xff, 0xff, 0xff, 0xff, 0xff },
    { 0xff, 0xff, 0xff, 0x50, 0x51, 0x52, 0x53, 0xff, 0xff },
    { 0x50, 0x51, 0x52, 0x53, 0xff, 0xff, 0xff, 0xff, 0xff },
    { 0xff, 0xff, 0xff, 0x50, 0x51, 0x52, 0x53, 0xff, 0xff },
    { 0x50, 0x51, 0x52, 0x53, 0xff, 0xff, 0xff, 0xff, 0xff },
    { 0xff, 0xff, 0xff, 0x50, 0x51, 0x52, 0x53, 0xff, 0xff },
    { 0x50, 0x51, 0x52, 0x53, 0xff, 0xff, 0xff, 0xff, 0xff },
};

static int dv_audio_frame_size(const DVprofile* sys, int frame)
{
    return sys->audio_samples_dist[frame % (sizeof(sys->audio_samples_dist)/
                                            sizeof(sys->audio_samples_dist[0]))];
}

static int dv_write_pack(enum dv_pack_type pack_id, DVMuxContext *c, uint8_t* buf, ...)
{
    struct tm tc;
    time_t ct;
    int ltc_frame;
    va_list ap;

    buf[0] = (uint8_t)pack_id;
    switch (pack_id) {
    case dv_timecode:
        ct = (time_t)(c->frames / ((float)c->sys->frame_rate /
                                   (float)c->sys->frame_rate_base));
        brktimegm(ct, &tc);
        /*
         * LTC drop-frame frame counter drops two frames (0 and 1) every
         * minute, unless it is exactly divisible by 10
         */
        ltc_frame = (c->frames + 2*ct/60 - 2*ct/600) % c->sys->ltc_divisor;
        buf[1] = (0 << 7) | /* Color fame: 0 - unsync; 1 - sync mode */
                 (1 << 6) | /* Drop frame timecode: 0 - nondrop; 1 - drop */
                 ((ltc_frame / 10) << 4) | /* Tens of frames */
                 (ltc_frame % 10);         /* Units of frames */
        buf[2] = (1 << 7) | /* Biphase mark polarity correction: 0 - even; 1 - odd */
                 ((tc.tm_sec / 10) << 4) | /* Tens of seconds */
                 (tc.tm_sec % 10);         /* Units of seconds */
        buf[3] = (1 << 7) | /* Binary group flag BGF0 */
                 ((tc.tm_min / 10) << 4) | /* Tens of minutes */
                 (tc.tm_min % 10);         /* Units of minutes */
        buf[4] = (1 << 7) | /* Binary group flag BGF2 */
                 (1 << 6) | /* Binary group flag BGF1 */
                 ((tc.tm_hour / 10) << 4) | /* Tens of hours */
                 (tc.tm_hour % 10);         /* Units of hours */
        break;
    case dv_audio_source:  /* AAUX source pack */
        va_start(ap, buf);
        buf[1] = (1 << 7) | /* locked mode -- SMPTE only supports locked mode */
                 (1 << 6) | /* reserved -- always 1 */
                 (dv_audio_frame_size(c->sys, c->frames) -
                  c->sys->audio_min_samples[0]);
                            /* # of samples      */
        buf[2] = (0 << 7) | /* multi-stereo      */
                 (0 << 5) | /* #of audio channels per block: 0 -- 1 channel */
                 (0 << 4) | /* pair bit: 0 -- one pair of channels */
                 !!va_arg(ap, int); /* audio mode        */
        buf[3] = (1 << 7) | /* res               */
                 (1 << 6) | /* multi-language flag */
                 (c->sys->dsf << 5) | /*  system: 60fields/50fields */
                 (c->sys->n_difchan & 2); /* definition: 0 -- 25Mbps, 2 -- 50Mbps */
        buf[4] = (1 << 7) | /* emphasis: 1 -- off */
                 (0 << 6) | /* emphasis time constant: 0 -- reserved */
                 (0 << 3) | /* frequency: 0 -- 48Khz, 1 -- 44,1Khz, 2 -- 32Khz */
                  0;        /* quantization: 0 -- 16bit linear, 1 -- 12bit nonlinear */
        va_end(ap);
        break;
    case dv_audio_control:
        buf[1] = (0 << 6) | /* copy protection: 0 -- unrestricted */
                 (1 << 4) | /* input source: 1 -- digital input */
                 (3 << 2) | /* compression: 3 -- no information */
                  0;        /* misc. info/SMPTE emphasis off */
        buf[2] = (1 << 7) | /* recording start point: 1 -- no */
                 (1 << 6) | /* recording end point: 1 -- no */
                 (1 << 3) | /* recording mode: 1 -- original */
                  7;
        buf[3] = (1 << 7) | /* direction: 1 -- forward */
                 (c->sys->pix_fmt == PIX_FMT_YUV420P ? 0x20 : /* speed */
                                                       c->sys->ltc_divisor*4);
        buf[4] = (1 << 7) | /* reserved -- always 1 */
                  0x7f;     /* genre category */
        break;
    case dv_audio_recdate:
    case dv_video_recdate:  /* VAUX recording date */
        ct = c->start_time + (time_t)(c->frames /
             ((float)c->sys->frame_rate / (float)c->sys->frame_rate_base));
        brktimegm(ct, &tc);
        buf[1] = 0xff; /* ds, tm, tens of time zone, units of time zone */
                       /* 0xff is very likely to be "unknown" */
        buf[2] = (3 << 6) | /* reserved -- always 1 */
                 ((tc.tm_mday / 10) << 4) | /* Tens of day */
                 (tc.tm_mday % 10);         /* Units of day */
        buf[3] = /* we set high 4 bits to 0, shouldn't we set them to week? */
                 ((tc.tm_mon / 10) << 4) |    /* Tens of month */
                 (tc.tm_mon  % 10);           /* Units of month */
        buf[4] = (((tc.tm_year % 100) / 10) << 4) | /* Tens of year */
                 (tc.tm_year % 10);                 /* Units of year */
        break;
    case dv_audio_rectime:  /* AAUX recording time */
    case dv_video_rectime:  /* VAUX recording time */
        ct = c->start_time + (time_t)(c->frames /
             ((float)c->sys->frame_rate / (float)c->sys->frame_rate_base));
        brktimegm(ct, &tc);
        buf[1] = (3 << 6) | /* reserved -- always 1 */
                 0x3f; /* tens of frame, units of frame: 0x3f - "unknown" ? */
        buf[2] = (1 << 7) | /* reserved -- always 1 */
                 ((tc.tm_sec / 10) << 4) | /* Tens of seconds */
                 (tc.tm_sec % 10);         /* Units of seconds */
        buf[3] = (1 << 7) | /* reserved -- always 1 */
                 ((tc.tm_min / 10) << 4) | /* Tens of minutes */
                 (tc.tm_min % 10);         /* Units of minutes */
        buf[4] = (3 << 6) | /* reserved -- always 1 */
                 ((tc.tm_hour / 10) << 4) | /* Tens of hours */
                 (tc.tm_hour % 10);         /* Units of hours */
        break;
    default:
        buf[1] = buf[2] = buf[3] = buf[4] = 0xff;
    }
    return 5;
}

static void dv_inject_audio(DVMuxContext *c, int channel, uint8_t* frame_ptr)
{
    int i, j, d, of, size;
    size = 4 * dv_audio_frame_size(c->sys, c->frames);
    frame_ptr += channel * c->sys->difseg_size * 150 * 80;
    for (i = 0; i < c->sys->difseg_size; i++) {
        frame_ptr += 6 * 80; /* skip DIF segment header */
        for (j = 0; j < 9; j++) {
            dv_write_pack(dv_aaux_packs_dist[i][j], c, &frame_ptr[3], i >= c->sys->difseg_size/2);
            for (d = 8; d < 80; d+=2) {
                of = c->sys->audio_shuffle[i][j] + (d - 8)/2 * c->sys->audio_stride;
                if (of*2 >= size)
                    continue;

                frame_ptr[d] = av_fifo_peek(&c->audio_data[channel], of*2+1); // FIXME: may be we have to admit
                frame_ptr[d+1] = av_fifo_peek(&c->audio_data[channel], of*2); //        that DV is a big endian PCM
            }
            frame_ptr += 16 * 80; /* 15 Video DIFs + 1 Audio DIF */
        }
    }
}

static void dv_inject_metadata(DVMuxContext *c, uint8_t* frame)
{
    int j, k;
    uint8_t* buf;

    for (buf = frame; buf < frame + c->sys->frame_size; buf += 150 * 80) {
        /* DV subcode: 2nd and 3d DIFs */
        for (j = 80; j < 80 * 3; j += 80) {
            for (k = 6; k < 6 * 8; k += 8)
                dv_write_pack(dv_timecode, c, &buf[j+k]);

            if (((long)(buf-frame)/(c->sys->frame_size/(c->sys->difseg_size*c->sys->n_difchan))%c->sys->difseg_size) > 5) { /* FIXME: is this really needed ? */
                dv_write_pack(dv_video_recdate, c, &buf[j+14]);
                dv_write_pack(dv_video_rectime, c, &buf[j+22]);
                dv_write_pack(dv_video_recdate, c, &buf[j+38]);
                dv_write_pack(dv_video_rectime, c, &buf[j+46]);
            }
        }

        /* DV VAUX: 4th, 5th and 6th 3DIFs */
        for (j = 80*3 + 3; j < 80*6; j += 80) {
            dv_write_pack(dv_video_recdate, c, &buf[j+5*2]);
            dv_write_pack(dv_video_rectime, c, &buf[j+5*3]);
            dv_write_pack(dv_video_recdate, c, &buf[j+5*11]);
            dv_write_pack(dv_video_rectime, c, &buf[j+5*12]);
        }
    }
}

/*
 * The following 3 functions constitute our interface to the world
 */

int dv_assemble_frame(DVMuxContext *c, AVStream* st,
                      const uint8_t* data, int data_size, uint8_t** frame)
{
    int i, reqasize;

    *frame = &c->frame_buf[0];
    reqasize = 4 * dv_audio_frame_size(c->sys, c->frames);

    switch (st->codec->codec_type) {
    case CODEC_TYPE_VIDEO:
        /* FIXME: we have to have more sensible approach than this one */
        if (c->has_video)
            av_log(st->codec, AV_LOG_ERROR, "Can't process DV frame #%d. Insufficient audio data or severe sync problem.\n", c->frames);

        memcpy(*frame, data, c->sys->frame_size);
        c->has_video = 1;
        break;
    case CODEC_TYPE_AUDIO:
        for (i = 0; i < c->n_ast && st != c->ast[i]; i++);

          /* FIXME: we have to have more sensible approach than this one */
        if (av_fifo_size(&c->audio_data[i]) + data_size >= 100*AVCODEC_MAX_AUDIO_FRAME_SIZE)
            av_log(st->codec, AV_LOG_ERROR, "Can't process DV frame #%d. Insufficient video data or severe sync problem.\n", c->frames);
        av_fifo_write(&c->audio_data[i], data, data_size);

        /* Lets see if we've got enough audio for one DV frame */
        c->has_audio |= ((reqasize <= av_fifo_size(&c->audio_data[i])) << i);

        break;
    default:
        break;
    }

    /* Lets see if we have enough data to construct one DV frame */
    if (c->has_video == 1 && c->has_audio + 1 == 1<<c->n_ast) {
        dv_inject_metadata(c, *frame);
        c->has_audio = 0;
        for (i=0; i<c->n_ast; i++) {
            dv_inject_audio(c, i, *frame);
            av_fifo_drain(&c->audio_data[i], reqasize);
            c->has_audio |= ((reqasize <= av_fifo_size(&c->audio_data[i])) << i);
        }

        c->has_video = 0;

        c->frames++;

        return c->sys->frame_size;
    }

    return 0;
}

DVMuxContext* dv_init_mux(AVFormatContext* s)
{
    DVMuxContext *c = s->priv_data;
    AVStream *vst = NULL;
    int i;

    /* we support at most 1 video and 2 audio streams */
    if (s->nb_streams > 3)
        return NULL;

    c->n_ast = 0;
    c->ast[0] = c->ast[1] = NULL;

    /* We have to sort out where audio and where video stream is */
    for (i=0; i<s->nb_streams; i++) {
        switch (s->streams[i]->codec->codec_type) {
        case CODEC_TYPE_VIDEO:
            if (vst) return NULL;
            vst = s->streams[i];
            break;
        case CODEC_TYPE_AUDIO:
            if (c->n_ast > 1) return NULL;
            c->ast[c->n_ast++] = s->streams[i];
            break;
        default:
            goto bail_out;
        }
    }

    /* Some checks -- DV format is very picky about its incoming streams */
    if (!vst || vst->codec->codec_id != CODEC_ID_DVVIDEO)
        goto bail_out;
    for (i=0; i<c->n_ast; i++) {
        if (c->ast[i] && (c->ast[i]->codec->codec_id != CODEC_ID_PCM_S16LE ||
                          c->ast[i]->codec->sample_rate != 48000 ||
                          c->ast[i]->codec->channels != 2))
            goto bail_out;
    }
    c->sys = dv_codec_profile(vst->codec);
    if (!c->sys)
        goto bail_out;

    if((c->n_ast > 1) && (c->sys->n_difchan < 2)) {
        /* only 1 stereo pair is allowed in 25Mbps mode */
        goto bail_out;
    }

    /* Ok, everything seems to be in working order */
    c->frames = 0;
    c->has_audio = 0;
    c->has_video = 0;
    c->start_time = (time_t)s->timestamp;

    for (i=0; i<c->n_ast; i++) {
        if (c->ast[i] && av_fifo_init(&c->audio_data[i], 100*AVCODEC_MAX_AUDIO_FRAME_SIZE) < 0) {
            while (i>0) {
                i--;
                av_fifo_free(&c->audio_data[i]);
            }
            goto bail_out;
        }
    }

    return c;

bail_out:
    return NULL;
}

void dv_delete_mux(DVMuxContext *c)
{
    int i;
    for (i=0; i < c->n_ast; i++)
        av_fifo_free(&c->audio_data[i]);
}

#ifdef CONFIG_MUXERS
static int dv_write_header(AVFormatContext *s)
{
    if (!dv_init_mux(s)) {
        av_log(s, AV_LOG_ERROR, "Can't initialize DV format!\n"
                    "Make sure that you supply exactly two streams:\n"
                    "     video: 25fps or 29.97fps, audio: 2ch/48Khz/PCM\n"
                    "     (50Mbps allows an optional second audio stream)\n");
        return -1;
    }
    return 0;
}

static int dv_write_packet(struct AVFormatContext *s, AVPacket *pkt)
{
    uint8_t* frame;
    int fsize;

    fsize = dv_assemble_frame(s->priv_data, s->streams[pkt->stream_index],
                              pkt->data, pkt->size, &frame);
    if (fsize > 0) {
        put_buffer(&s->pb, frame, fsize);
        put_flush_packet(&s->pb);
    }
    return 0;
}

/*
 * We might end up with some extra A/V data without matching counterpart.
 * E.g. video data without enough audio to write the complete frame.
 * Currently we simply drop the last frame. I don't know whether this
 * is the best strategy of all
 */
static int dv_write_trailer(struct AVFormatContext *s)
{
    dv_delete_mux(s->priv_data);
    return 0;
}
#endif /* CONFIG_MUXERS */

#ifdef CONFIG_DV_MUXER
AVOutputFormat dv_muxer = {
    "dv",
    "DV video format",
    NULL,
    "dv",
    sizeof(DVMuxContext),
    CODEC_ID_PCM_S16LE,
    CODEC_ID_DVVIDEO,
    dv_write_header,
    dv_write_packet,
    dv_write_trailer,
};
#endif
