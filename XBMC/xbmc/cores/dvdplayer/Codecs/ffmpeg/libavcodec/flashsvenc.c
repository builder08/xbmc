/*
 * Flash Screen Video encoder
 * Copyright (C) 2004 Alex Beregszaszi
 * Copyright (C) 2006 Benjamin Larsson
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

/* Encoding development sponsored by http://fh-campuswien.ac.at */

/**
 * @file flashsvenc.c
 * Flash Screen Video encoder
 * @author Alex Beregszaszi
 * @author Benjamin Larsson
 */

/* Bitstream description
 * The picture is divided into blocks that are zlib-compressed.
 *
 * The decoder is fed complete frames, the frameheader contains:
 * 4bits of block width
 * 12bits of frame width
 * 4bits of block height
 * 12bits of frame height
 *
 * Directly after the header are the compressed blocks. The blocks
 * have their compressed size represented with 16bits in the beginig.
 * If the size = 0 then the block is unchanged from the previous frame.
 * All blocks are decompressed until the buffer is consumed.
 *
 * Encoding ideas, a basic encoder would just use a fixed block size.
 * Block sizes can be multipels of 16, from 16 to 256. The blocks don't
 * have to be quadratic. A brute force search with a set of different
 * block sizes should give a better result than to just use a fixed size.
 */

/* TODO:
 * Don't reencode the frame in brute force mode if the frame is a dupe. Speed up.
 * Make the difference check faster.
 */

#include <stdio.h>
#include <stdlib.h>
#include <zlib.h>

#include "avcodec.h"
#include "bitstream.h"
#include "bytestream.h"


typedef struct FlashSVContext {
    AVCodecContext *avctx;
    uint8_t *previous_frame;
    AVFrame frame;
    int image_width, image_height;
    int block_width, block_height;
    uint8_t* tmpblock;
    uint8_t* encbuffer;
    int block_size;
    z_stream zstream;
    int last_key_frame;
} FlashSVContext;

static int copy_region_enc(uint8_t *sptr, uint8_t *dptr,
        int dx, int dy, int h, int w, int stride, uint8_t *pfptr) {
    int i,j;
    uint8_t *nsptr;
    uint8_t *npfptr;
    int diff = 0;

    for (i = dx+h; i > dx; i--) {
        nsptr = sptr+(i*stride)+dy*3;
        npfptr = pfptr+(i*stride)+dy*3;
        for (j=0 ; j<w*3 ; j++) {
            diff |=npfptr[j]^nsptr[j];
            dptr[j] = nsptr[j];
        }
        dptr += w*3;
    }
    if (diff)
        return 1;
    return 0;
}

static av_cold int flashsv_encode_init(AVCodecContext *avctx)
{
    FlashSVContext *s = avctx->priv_data;

    s->avctx = avctx;

    if ((avctx->width > 4095) || (avctx->height > 4095)) {
        av_log(avctx, AV_LOG_ERROR, "Input dimensions too large, input must be max 4096x4096 !\n");
        return -1;
    }

    if (avcodec_check_dimensions(avctx, avctx->width, avctx->height) < 0) {
        return -1;
    }

    // Needed if zlib unused or init aborted before deflateInit
    memset(&(s->zstream), 0, sizeof(z_stream));

    s->last_key_frame=0;

    s->image_width = avctx->width;
    s->image_height = avctx->height;

    s->tmpblock = av_mallocz(3*256*256);
    s->encbuffer = av_mallocz(s->image_width*s->image_height*3);

    if (!s->tmpblock || !s->encbuffer) {
        av_log(avctx, AV_LOG_ERROR, "Memory allocation failed.\n");
        return -1;
    }

    return 0;
}


static int encode_bitstream(FlashSVContext *s, AVFrame *p, uint8_t *buf, int buf_size,
     int block_width, int block_height, uint8_t *previous_frame, int* I_frame) {

    PutBitContext pb;
    int h_blocks, v_blocks, h_part, v_part, i, j;
    int buf_pos, res;
    int pred_blocks = 0;

    init_put_bits(&pb, buf, buf_size*8);

    put_bits(&pb, 4, (block_width/16)-1);
    put_bits(&pb, 12, s->image_width);
    put_bits(&pb, 4, (block_height/16)-1);
    put_bits(&pb, 12, s->image_height);
    flush_put_bits(&pb);
    buf_pos=4;

    h_blocks = s->image_width / block_width;
    h_part = s->image_width % block_width;
    v_blocks = s->image_height / block_height;
    v_part = s->image_height % block_height;

    /* loop over all block columns */
    for (j = 0; j < v_blocks + (v_part?1:0); j++)
    {

        int hp = j*block_height; // horiz position in frame
        int hs = (j<v_blocks)?block_height:v_part; // size of block

        /* loop over all block rows */
        for (i = 0; i < h_blocks + (h_part?1:0); i++)
        {
            int wp = i*block_width; // vert position in frame
            int ws = (i<h_blocks)?block_width:h_part; // size of block
            int ret=Z_OK;
            uint8_t *ptr;

            ptr = buf+buf_pos;

            //copy the block to the temp buffer before compression (if it differs from the previous frame's block)
            res = copy_region_enc(p->data[0], s->tmpblock, s->image_height-(hp+hs+1), wp, hs, ws, p->linesize[0], previous_frame);

            if (res || *I_frame) {
                unsigned long zsize;
                zsize = 3*block_width*block_height;
                ret = compress2(ptr+2, &zsize, s->tmpblock, 3*ws*hs, 9);


                //ret = deflateReset(&(s->zstream));
                if (ret != Z_OK)
                    av_log(s->avctx, AV_LOG_ERROR, "error while compressing block %dx%d\n", i, j);

                bytestream_put_be16(&ptr,(unsigned int)zsize);
                buf_pos += zsize+2;
                //av_log(avctx, AV_LOG_ERROR, "buf_pos = %d\n", buf_pos);
            } else {
                pred_blocks++;
                bytestream_put_be16(&ptr,0);
                buf_pos += 2;
            }
        }
    }

    if (pred_blocks)
        *I_frame = 0;
    else
        *I_frame = 1;

    return buf_pos;
}


static int flashsv_encode_frame(AVCodecContext *avctx, uint8_t *buf, int buf_size, void *data)
{
    FlashSVContext * const s = avctx->priv_data;
    AVFrame *pict = data;
    AVFrame * const p = &s->frame;
    uint8_t *pfptr;
    int res;
    int I_frame = 0;
    int opt_w, opt_h;

    *p = *pict;

    /* First frame needs to be a keyframe */
    if (avctx->frame_number == 0) {
        s->previous_frame = av_mallocz(FFABS(p->linesize[0])*s->image_height);
        if (!s->previous_frame) {
            av_log(avctx, AV_LOG_ERROR, "Memory allocation failed.\n");
            return -1;
        }
        I_frame = 1;
    }

    if (p->linesize[0] < 0)
        pfptr = s->previous_frame - ((s->image_height-1) * p->linesize[0]);
    else
        pfptr = s->previous_frame;

    /* Check the placement of keyframes */
    if (avctx->gop_size > 0) {
        if (avctx->frame_number >= s->last_key_frame + avctx->gop_size) {
            I_frame = 1;
        }
    }

    opt_w=4;
    opt_h=4;

    if (buf_size < s->image_width*s->image_height*3) {
        //Conservative upper bound check for compressed data
        av_log(avctx, AV_LOG_ERROR, "buf_size %d <  %d\n", buf_size, s->image_width*s->image_height*3);
        return -1;
    }

    res = encode_bitstream(s, p, buf, buf_size, opt_w*16, opt_h*16, pfptr, &I_frame);

    //save the current frame
    if(p->linesize[0] > 0)
        memcpy(s->previous_frame, p->data[0], s->image_height*p->linesize[0]);
    else
        memcpy(s->previous_frame, p->data[0] + p->linesize[0] * (s->image_height-1), s->image_height*FFABS(p->linesize[0]));

    //mark the frame type so the muxer can mux it correctly
    if (I_frame) {
        p->pict_type = FF_I_TYPE;
        p->key_frame = 1;
        s->last_key_frame = avctx->frame_number;
        av_log(avctx, AV_LOG_DEBUG, "Inserting key frame at frame %d\n",avctx->frame_number);
    } else {
        p->pict_type = FF_P_TYPE;
        p->key_frame = 0;
    }

    avctx->coded_frame = p;

    return res;
}

static av_cold int flashsv_encode_end(AVCodecContext *avctx)
{
    FlashSVContext *s = avctx->priv_data;

    deflateEnd(&(s->zstream));

    av_free(s->encbuffer);
    av_free(s->previous_frame);
    av_free(s->tmpblock);

    return 0;
}

AVCodec flashsv_encoder = {
    "flashsv",
    CODEC_TYPE_VIDEO,
    CODEC_ID_FLASHSV,
    sizeof(FlashSVContext),
    flashsv_encode_init,
    flashsv_encode_frame,
    flashsv_encode_end,
    .pix_fmts = (enum PixelFormat[]){PIX_FMT_BGR24, PIX_FMT_NONE},
    .long_name = "Flash Screen Video",
};

