/*
 * MPEG1/2 demuxer
 * Copyright (c) 2000, 2001, 2002 Fabrice Bellard.
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
#include "mpeg.h"

//#define DEBUG_SEEK

#undef NDEBUG
#include <assert.h>

/*********************************************/
/* demux code */

#define MAX_SYNC_SIZE 100000

static int cdxa_probe(AVProbeData *p)
{
    /* check file header */
    if (p->buf[0] == 'R' && p->buf[1] == 'I' &&
        p->buf[2] == 'F' && p->buf[3] == 'F' &&
        p->buf[8] == 'C' && p->buf[9] == 'D' &&
        p->buf[10] == 'X' && p->buf[11] == 'A')
        return AVPROBE_SCORE_MAX;
    else
        return 0;
}

static int mpegps_probe(AVProbeData *p)
{
    uint32_t code= -1;
    int sys=0, pspack=0, priv1=0, vid=0, audio=0;
    int i;
    int score=0;

    score = cdxa_probe(p);
    if (score > 0) return score;

    /* Search for MPEG stream */
    for(i=0; i<p->buf_size; i++){
        code = (code<<8) + p->buf[i];
        if ((code & 0xffffff00) == 0x100) {
            if(code == SYSTEM_HEADER_START_CODE) sys++;
            else if(code == PRIVATE_STREAM_1)    priv1++;
            else if(code == PACK_START_CODE)     pspack++;
            else if((code & 0xf0) == VIDEO_ID)   vid++;
            else if((code & 0xe0) == AUDIO_ID)   audio++;
        }
    }

    if(vid || audio)            /* invalid VDR files nd short PES streams */
        score= AVPROBE_SCORE_MAX/4;

//av_log(NULL, AV_LOG_ERROR, "%d %d %d %d %d len:%d\n", sys, priv1, pspack,vid, audio, p->buf_size);
    if(sys && sys*9 <= pspack*10)
        return AVPROBE_SCORE_MAX/2+2; // +1 for .mpg
    if((priv1 || vid || audio) && (priv1+vid+audio)*9 <= pspack*10)
        return AVPROBE_SCORE_MAX/2+2; // +1 for .mpg
    if((!!vid ^ !!audio) && (audio+vid > 1) && !sys && !pspack && p->buf_size>2048) /* PES stream */
        return AVPROBE_SCORE_MAX/2+2;

    //02-Penguin.flac has sys:0 priv1:0 pspack:0 vid:0 audio:1
    return score;
}


typedef struct MpegDemuxContext {
    int32_t header_state;
    unsigned char psm_es_type[256];
} MpegDemuxContext;

static int mpegps_read_header(AVFormatContext *s,
                              AVFormatParameters *ap)
{
    MpegDemuxContext *m = s->priv_data;
    m->header_state = 0xff;
    s->ctx_flags |= AVFMTCTX_NOHEADER;

    /* no need to do more */
    return 0;
}

static int64_t get_pts(ByteIOContext *pb, int c)
{
    int64_t pts;
    int val;

    if (c < 0)
        c = get_byte(pb);
    pts = (int64_t)((c >> 1) & 0x07) << 30;
    val = get_be16(pb);
    pts |= (int64_t)(val >> 1) << 15;
    val = get_be16(pb);
    pts |= (int64_t)(val >> 1);
    return pts;
}

static int find_next_start_code(ByteIOContext *pb, int *size_ptr,
                                int32_t *header_state)
{
    unsigned int state, v;
    int val, n;

    state = *header_state;
    n = *size_ptr;
    while (n > 0) {
        if (url_feof(pb))
            break;
        v = get_byte(pb);
        n--;
        if (state == 0x000001) {
            state = ((state << 8) | v) & 0xffffff;
            val = state;
            goto found;
        }
        state = ((state << 8) | v) & 0xffffff;
    }
    val = -1;
 found:
    *header_state = state;
    *size_ptr = n;
    return val;
}

#if 0 /* unused, remove? */
/* XXX: optimize */
static int find_prev_start_code(ByteIOContext *pb, int *size_ptr)
{
    int64_t pos, pos_start;
    int max_size, start_code;

    max_size = *size_ptr;
    pos_start = url_ftell(pb);

    /* in order to go faster, we fill the buffer */
    pos = pos_start - 16386;
    if (pos < 0)
        pos = 0;
    url_fseek(pb, pos, SEEK_SET);
    get_byte(pb);

    pos = pos_start;
    for(;;) {
        pos--;
        if (pos < 0 || (pos_start - pos) >= max_size) {
            start_code = -1;
            goto the_end;
        }
        url_fseek(pb, pos, SEEK_SET);
        start_code = get_be32(pb);
        if ((start_code & 0xffffff00) == 0x100)
            break;
    }
 the_end:
    *size_ptr = pos_start - pos;
    return start_code;
}
#endif

/**
 * Extracts stream types from a program stream map
 * According to ISO/IEC 13818-1 ('MPEG-2 Systems') table 2-35
 *
 * @return number of bytes occupied by PSM in the bitstream
 */
static long mpegps_psm_parse(MpegDemuxContext *m, ByteIOContext *pb)
{
    int psm_length, ps_info_length, es_map_length;

    psm_length = get_be16(pb);
    get_byte(pb);
    get_byte(pb);
    ps_info_length = get_be16(pb);

    /* skip program_stream_info */
    url_fskip(pb, ps_info_length);
    es_map_length = get_be16(pb);

    /* at least one es available? */
    while (es_map_length >= 4){
        unsigned char type = get_byte(pb);
        unsigned char es_id = get_byte(pb);
        uint16_t es_info_length = get_be16(pb);
        /* remember mapping from stream id to stream type */
        m->psm_es_type[es_id] = type;
        /* skip program_stream_info */
        url_fskip(pb, es_info_length);
        es_map_length -= 4 + es_info_length;
    }
    get_be32(pb); /* crc32 */
    return 2 + psm_length;
}

/* read the next PES header. Return its position in ppos
   (if not NULL), and its start code, pts and dts.
 */
static int mpegps_read_pes_header(AVFormatContext *s,
                                  int64_t *ppos, int *pstart_code,
                                  int64_t *ppts, int64_t *pdts)
{
    MpegDemuxContext *m = s->priv_data;
    int len, size, startcode, c, flags, header_len;
    int pes_ext, ext2_len, id_ext, skip;
    int64_t pts, dts;
    int64_t last_sync= url_ftell(&s->pb);

 error_redo:
        url_fseek(&s->pb, last_sync, SEEK_SET);
 redo:
        /* next start code (should be immediately after) */
        m->header_state = 0xff;
        size = MAX_SYNC_SIZE;
        startcode = find_next_start_code(&s->pb, &size, &m->header_state);
        last_sync = url_ftell(&s->pb);
    //printf("startcode=%x pos=0x%"PRIx64"\n", startcode, url_ftell(&s->pb));
    if (startcode < 0)
        return AVERROR(EIO);
    if (startcode == PACK_START_CODE)
        goto redo;
    if (startcode == SYSTEM_HEADER_START_CODE)
        goto redo;
    if (startcode == PADDING_STREAM ||
        startcode == PRIVATE_STREAM_2) {
        /* skip them */
        len = get_be16(&s->pb);
        url_fskip(&s->pb, len);
        goto redo;
    }
    if (startcode == PROGRAM_STREAM_MAP) {
        mpegps_psm_parse(m, &s->pb);
        goto redo;
    }

    /* find matching stream */
    if (!((startcode >= 0x1c0 && startcode <= 0x1df) ||
          (startcode >= 0x1e0 && startcode <= 0x1ef) ||
          (startcode == 0x1bd) || (startcode == 0x1fd)))
        goto redo;
    if (ppos) {
        *ppos = url_ftell(&s->pb) - 4;
    }
    len = get_be16(&s->pb);
    pts =
    dts = AV_NOPTS_VALUE;
    /* stuffing */
    for(;;) {
        if (len < 1)
            goto error_redo;
        c = get_byte(&s->pb);
        len--;
        /* XXX: for mpeg1, should test only bit 7 */
        if (c != 0xff)
            break;
    }
    if ((c & 0xc0) == 0x40) {
        /* buffer scale & size */
        get_byte(&s->pb);
        c = get_byte(&s->pb);
        len -= 2;
    }
    if ((c & 0xe0) == 0x20) {
        dts = pts = get_pts(&s->pb, c);
        len -= 4;
        if (c & 0x10){
            dts = get_pts(&s->pb, -1);
            len -= 5;
        }
    } else if ((c & 0xc0) == 0x80) {
        /* mpeg 2 PES */
#if 0 /* some streams have this field set for no apparent reason */
        if ((c & 0x30) != 0) {
            /* Encrypted multiplex not handled */
            goto redo;
        }
#endif
        flags = get_byte(&s->pb);
        header_len = get_byte(&s->pb);
        len -= 2;
        if (header_len > len)
            goto error_redo;
        len -= header_len;
        if (flags & 0x80) {
            dts = pts = get_pts(&s->pb, -1);
            header_len -= 5;
            if (flags & 0x40) {
                dts = get_pts(&s->pb, -1);
                header_len -= 5;
            }
        }
        if (flags & 0x01) { /* PES extension */
            pes_ext = get_byte(&s->pb);
            header_len--;
            if (pes_ext & 0x40) { /* pack header - should be zero in PS */
                goto error_redo;
            }
            /* Skip PES private data, program packet sequence counter and P-STD buffer */
            skip = (pes_ext >> 4) & 0xb;
            skip += skip & 0x9;
            url_fskip(&s->pb, skip);
            header_len -= skip;

            if (pes_ext & 0x01) { /* PES extension 2 */
                ext2_len = get_byte(&s->pb);
                header_len--;
                if ((ext2_len & 0x7f) > 0) {
                    id_ext = get_byte(&s->pb);
                    if ((id_ext & 0x80) == 0)
                        startcode = ((startcode & 0xff) << 8) | id_ext;
                    header_len--;
                }
            }
        }
        if(header_len < 0)
            goto error_redo;
        url_fskip(&s->pb, header_len);
    }
    else if( c!= 0xf )
        goto redo;

    if (startcode == PRIVATE_STREAM_1 && !m->psm_es_type[startcode & 0xff]) {
        startcode = get_byte(&s->pb);
        len--;
        if (startcode >= 0x80 && startcode <= 0xcf) {
            /* audio: skip header */
            get_byte(&s->pb);
            get_byte(&s->pb);
            get_byte(&s->pb);
            len -= 3;
            if (startcode >= 0xb0 && startcode <= 0xbf) {
                /* MLP/TrueHD audio has a 4-byte header */
                get_byte(&s->pb);
                len--;
            }
        }
    }
    if(len<0)
        goto error_redo;
    if(dts != AV_NOPTS_VALUE && ppos){
        int i;
        for(i=0; i<s->nb_streams; i++){
#ifdef _XBOX
		// for mpeg data, for every stream a few index entrys are saved to allow faster seeking
		// however, for dvd's with ac3 streams this goes pretty fast up to a few thousend entry's
		// resulting in huge memory usage, around 10 - 15 mb
		// thus, we only allow this voor the video stream and not the audio one. This is no problem
		// since seeking is only done on the video stream
		if (s->streams[i]->codec->codec_type == CODEC_TYPE_VIDEO)
#endif // _XBOX
            if(startcode == s->streams[i]->id) {
                av_add_index_entry(s->streams[i], *ppos, dts, 0, 0, AVINDEX_KEYFRAME /* FIXME keyframe? */);
            }
        }
    }

    *pstart_code = startcode;
    *ppts = pts;
    *pdts = dts;
    return len;
}

static void mpegps_get_lpcm_packet(ByteIOContext *pb, AVPacket *pkt, AVStream *st, int len)
{
    uint8_t buffer[12];
    int i, blocksize, blocksize2;
    
    blocksize = (2 * st->codec->channels * st->codec->bits_per_sample) / 8;
    blocksize2 = (2 * st->codec->channels * 16) / 8;

    av_new_packet(pkt, len * 16 / st->codec->bits_per_sample + blocksize2);
    pkt->size = 0;

    for(; len>0; len-=blocksize) {
      /* get the 16 msb bits for all channels */
      get_buffer(pb, pkt->data+pkt->size, blocksize2);
      pkt->size+=blocksize2;
      /* skip the lsb for all channels */
      url_fskip(pb, blocksize-blocksize2);      
    }

}

static int mpegps_read_packet(AVFormatContext *s,
                              AVPacket *pkt)
{
    MpegDemuxContext *m = s->priv_data;
    AVStream *st;
    int len, startcode, i, type, codec_id = 0, es_type;
    int64_t pts, dts, dummy_pos; //dummy_pos is needed for the index building to work

 redo:
    len = mpegps_read_pes_header(s, &dummy_pos, &startcode, &pts, &dts);
    if (len < 0)
        return len;

    /* now find stream */
    for(i=0;i<s->nb_streams;i++) {
        st = s->streams[i];
        if (st->id == startcode)
            goto found;
    }

    es_type = m->psm_es_type[startcode & 0xff];
    if(es_type > 0){
        if(es_type == STREAM_TYPE_VIDEO_MPEG1){
            codec_id = CODEC_ID_MPEG2VIDEO;
            type = CODEC_TYPE_VIDEO;
        } else if(es_type == STREAM_TYPE_VIDEO_MPEG2){
            codec_id = CODEC_ID_MPEG2VIDEO;
            type = CODEC_TYPE_VIDEO;
        } else if(es_type == STREAM_TYPE_AUDIO_MPEG1 ||
                  es_type == STREAM_TYPE_AUDIO_MPEG2){
            codec_id = CODEC_ID_MP3;
            type = CODEC_TYPE_AUDIO;
        } else if(es_type == STREAM_TYPE_AUDIO_AAC){
            codec_id = CODEC_ID_AAC;
            type = CODEC_TYPE_AUDIO;
        } else if(es_type == STREAM_TYPE_VIDEO_MPEG4){
            codec_id = CODEC_ID_MPEG4;
            type = CODEC_TYPE_VIDEO;
        } else if(es_type == STREAM_TYPE_VIDEO_H264){
            codec_id = CODEC_ID_H264;
            type = CODEC_TYPE_VIDEO;
        } else if(es_type == STREAM_TYPE_AUDIO_AC3){
            codec_id = CODEC_ID_AC3;
            type = CODEC_TYPE_AUDIO;
        } else {
            goto skip;
        }
    } else if (startcode >= 0x1e0 && startcode <= 0x1ef) {
        static const unsigned char avs_seqh[4] = { 0, 0, 1, 0xb0 };
        unsigned char buf[8];
        get_buffer(&s->pb, buf, 8);
        url_fseek(&s->pb, -8, SEEK_CUR);
        if(!memcmp(buf, avs_seqh, 4) && (buf[6] != 0 || buf[7] != 1))
            codec_id = CODEC_ID_CAVS;
        else
            codec_id = CODEC_ID_MPEG2VIDEO;
        type = CODEC_TYPE_VIDEO;
    } else if (startcode >= 0x1c0 && startcode <= 0x1df) {
        type = CODEC_TYPE_AUDIO;
        codec_id = CODEC_ID_MP2;
    } else if (startcode >= 0x80 && startcode <= 0x87) {
        type = CODEC_TYPE_AUDIO;
        codec_id = CODEC_ID_AC3;
    } else if ((startcode >= 0x88 && startcode <= 0x8f)
               ||( startcode >= 0x98 && startcode <= 0x9f)) {
        /* 0x90 - 0x97 is reserved for SDDS in DVD specs */
        type = CODEC_TYPE_AUDIO;
        codec_id = CODEC_ID_DTS;
    } else if (startcode >= 0xa0 && startcode <= 0xaf) {
        type = CODEC_TYPE_AUDIO;
        codec_id = CODEC_ID_PCM_S16BE;
    } else if (startcode >= 0xb0 && startcode <= 0xbf) {
        type = CODEC_TYPE_AUDIO;
        codec_id = CODEC_ID_MLP;
    } else if (startcode >= 0xc0 && startcode <= 0xcf) {
        /* Used for both AC-3 and E-AC-3 in EVOB files */
        type = CODEC_TYPE_AUDIO;
        codec_id = CODEC_ID_AC3;
    } else if (startcode >= 0x20 && startcode <= 0x3f) {
        type = CODEC_TYPE_SUBTITLE;
        codec_id = CODEC_ID_DVD_SUBTITLE;
    } else if (startcode >= 0xfd55 && startcode <= 0xfd5f) {
        type = CODEC_TYPE_VIDEO;
        codec_id = CODEC_ID_VC1;
    } else {
    skip:
        /* skip packet */
        url_fskip(&s->pb, len);
        goto redo;
    }
    /* no stream found: add a new stream */
    st = av_new_stream(s, startcode);
    if (!st)
        goto skip;
    st->codec->codec_type = type;
    st->codec->codec_id = codec_id;
    if (codec_id != CODEC_ID_PCM_S16BE)
        st->need_parsing = AVSTREAM_PARSE_FULL;
 found:
    if(st->discard >= AVDISCARD_ALL)
        goto skip;
    if (startcode >= 0xa0 && startcode <= 0xaf) {
        int b1, freq;

        /* for LPCM, we just skip the header and consider it is raw
           audio data */
        if (len <= 3)
            goto skip;
        get_byte(&s->pb); /* emphasis (1), muse(1), reserved(1), frame number(5) */
        b1 = get_byte(&s->pb); /* quant (2), freq(2), reserved(1), channels(3) */
        get_byte(&s->pb); /* dynamic range control (0x80 = off) */
        len -= 3;
        freq = (b1 >> 4) & 3;
        st->codec->sample_rate = lpcm_freq_tab[freq];
        st->codec->channels = 1 + (b1 & 7);
        st->codec->bits_per_sample = 16 + ((b1 >> 6) & 3) * 4;
        st->codec->bit_rate = st->codec->channels * st->codec->sample_rate * st->codec->bits_per_sample;
    }

    if (st->codec->codec_id == CODEC_ID_PCM_S16BE && st->codec->bits_per_sample > 16)
        mpegps_get_lpcm_packet(&s->pb, pkt, st, len);
    else
        av_get_packet(&s->pb, pkt, len);
    pkt->pts = pts;
    pkt->dts = dts;
    pkt->stream_index = st->index;
#if 0
    av_log(s, AV_LOG_DEBUG, "%d: pts=%0.3f dts=%0.3f size=%d\n",
           pkt->stream_index, pkt->pts / 90000.0, pkt->dts / 90000.0, pkt->size);
#endif

    return 0;
}

static int mpegps_read_close(AVFormatContext *s)
{
    return 0;
}

static int64_t mpegps_read_dts(AVFormatContext *s, int stream_index,
                               int64_t *ppos, int64_t pos_limit)
{
    int len, startcode;
    int64_t pos, pts, dts;

    pos = *ppos;
#ifdef DEBUG_SEEK
    printf("read_dts: pos=0x%"PRIx64" next=%d -> ", pos, find_next);
#endif
    url_fseek(&s->pb, pos, SEEK_SET);
    for(;;) {
        len = mpegps_read_pes_header(s, &pos, &startcode, &pts, &dts);
        if (len < 0) {
#ifdef DEBUG_SEEK
            printf("none (ret=%d)\n", len);
#endif
            return AV_NOPTS_VALUE;
        }
        if (startcode == s->streams[stream_index]->id &&
            dts != AV_NOPTS_VALUE) {
            break;
        }
        url_fskip(&s->pb, len);
    }
#ifdef DEBUG_SEEK
    printf("pos=0x%"PRIx64" dts=0x%"PRIx64" %0.3f\n", pos, dts, dts / 90000.0);
#endif
    *ppos = pos;
    return dts;
}

AVInputFormat mpegps_demuxer = {
    "mpeg",
    "MPEG PS format",
    sizeof(MpegDemuxContext),
    mpegps_probe,
    mpegps_read_header,
    mpegps_read_packet,
    mpegps_read_close,
    NULL, //mpegps_read_seek,
    mpegps_read_dts,
    .flags = AVFMT_SHOW_IDS,
};
