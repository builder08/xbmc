/*
 * Copyright (C) 2004 Michael Niedermayer <michaelni@gmx.at>
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

#include "libavutil/intmath.h"
#include "libavutil/log.h"
#include "libavutil/opt.h"
#include "avcodec.h"
#include "dsputil.h"
#include "dwt.h"
#include "snow.h"

#include "rangecoder.h"
#include "mathops.h"

#include "mpegvideo.h"
#include "h263.h"

#undef NDEBUG
#include <assert.h>

#define QUANTIZE2 0

#if QUANTIZE2==1
#define Q2_STEP 8

static void find_sse(SnowContext *s, Plane *p, int *score, int score_stride, IDWTELEM *r0, IDWTELEM *r1, int level, int orientation){
    SubBand *b= &p->band[level][orientation];
    int x, y;
    int xo=0;
    int yo=0;
    int step= 1 << (s->spatial_decomposition_count - level);

    if(orientation&1)
        xo= step>>1;
    if(orientation&2)
        yo= step>>1;

    //FIXME bias for nonzero ?
    //FIXME optimize
    memset(score, 0, sizeof(*score)*score_stride*((p->height + Q2_STEP-1)/Q2_STEP));
    for(y=0; y<p->height; y++){
        for(x=0; x<p->width; x++){
            int sx= (x-xo + step/2) / step / Q2_STEP;
            int sy= (y-yo + step/2) / step / Q2_STEP;
            int v= r0[x + y*p->width] - r1[x + y*p->width];
            assert(sx>=0 && sy>=0 && sx < score_stride);
            v= ((v+8)>>4)<<4;
            score[sx + sy*score_stride] += v*v;
            assert(score[sx + sy*score_stride] >= 0);
        }
    }
}

static void dequantize_all(SnowContext *s, Plane *p, IDWTELEM *buffer, int width, int height){
    int level, orientation;

    for(level=0; level<s->spatial_decomposition_count; level++){
        for(orientation=level ? 1 : 0; orientation<4; orientation++){
            SubBand *b= &p->band[level][orientation];
            IDWTELEM *dst= buffer + (b->ibuf - s->spatial_idwt_buffer);

            dequantize(s, b, dst, b->stride);
        }
    }
}

static void dwt_quantize(SnowContext *s, Plane *p, DWTELEM *buffer, int width, int height, int stride, int type){
    int level, orientation, ys, xs, x, y, pass;
    IDWTELEM best_dequant[height * stride];
    IDWTELEM idwt2_buffer[height * stride];
    const int score_stride= (width + 10)/Q2_STEP;
    int best_score[(width + 10)/Q2_STEP * (height + 10)/Q2_STEP]; //FIXME size
    int score[(width + 10)/Q2_STEP * (height + 10)/Q2_STEP]; //FIXME size
    int threshold= (s->m.lambda * s->m.lambda) >> 6;

    //FIXME pass the copy cleanly ?

//    memcpy(dwt_buffer, buffer, height * stride * sizeof(DWTELEM));
    ff_spatial_dwt(buffer, width, height, stride, type, s->spatial_decomposition_count);

    for(level=0; level<s->spatial_decomposition_count; level++){
        for(orientation=level ? 1 : 0; orientation<4; orientation++){
            SubBand *b= &p->band[level][orientation];
            IDWTELEM *dst= best_dequant + (b->ibuf - s->spatial_idwt_buffer);
             DWTELEM *src=       buffer + (b-> buf - s->spatial_dwt_buffer);
            assert(src == b->buf); // code does not depend on this but it is true currently

            quantize(s, b, dst, src, b->stride, s->qbias);
        }
    }
    for(pass=0; pass<1; pass++){
        if(s->qbias == 0) //keyframe
            continue;
        for(level=0; level<s->spatial_decomposition_count; level++){
            for(orientation=level ? 1 : 0; orientation<4; orientation++){
                SubBand *b= &p->band[level][orientation];
                IDWTELEM *dst= idwt2_buffer + (b->ibuf - s->spatial_idwt_buffer);
                IDWTELEM *best_dst= best_dequant + (b->ibuf - s->spatial_idwt_buffer);

                for(ys= 0; ys<Q2_STEP; ys++){
                    for(xs= 0; xs<Q2_STEP; xs++){
                        memcpy(idwt2_buffer, best_dequant, height * stride * sizeof(IDWTELEM));
                        dequantize_all(s, p, idwt2_buffer, width, height);
                        ff_spatial_idwt(idwt2_buffer, width, height, stride, type, s->spatial_decomposition_count);
                        find_sse(s, p, best_score, score_stride, idwt2_buffer, s->spatial_idwt_buffer, level, orientation);
                        memcpy(idwt2_buffer, best_dequant, height * stride * sizeof(IDWTELEM));
                        for(y=ys; y<b->height; y+= Q2_STEP){
                            for(x=xs; x<b->width; x+= Q2_STEP){
                                if(dst[x + y*b->stride]<0) dst[x + y*b->stride]++;
                                if(dst[x + y*b->stride]>0) dst[x + y*b->stride]--;
                                //FIXME try more than just --
                            }
                        }
                        dequantize_all(s, p, idwt2_buffer, width, height);
                        ff_spatial_idwt(idwt2_buffer, width, height, stride, type, s->spatial_decomposition_count);
                        find_sse(s, p, score, score_stride, idwt2_buffer, s->spatial_idwt_buffer, level, orientation);
                        for(y=ys; y<b->height; y+= Q2_STEP){
                            for(x=xs; x<b->width; x+= Q2_STEP){
                                int score_idx= x/Q2_STEP + (y/Q2_STEP)*score_stride;
                                if(score[score_idx] <= best_score[score_idx] + threshold){
                                    best_score[score_idx]= score[score_idx];
                                    if(best_dst[x + y*b->stride]<0) best_dst[x + y*b->stride]++;
                                    if(best_dst[x + y*b->stride]>0) best_dst[x + y*b->stride]--;
                                    //FIXME copy instead
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    memcpy(s->spatial_idwt_buffer, best_dequant, height * stride * sizeof(IDWTELEM)); //FIXME work with that directly instead of copy at the end
}

#endif /* QUANTIZE2==1 */

#if CONFIG_SNOW_ENCODER
static av_cold int encode_init(AVCodecContext *avctx)
{
    SnowContext *s = avctx->priv_data;
    int plane_index;

    if(avctx->strict_std_compliance > FF_COMPLIANCE_EXPERIMENTAL){
        av_log(avctx, AV_LOG_ERROR, "This codec is under development, files encoded with it may not be decodable with future versions!!!\n"
               "Use vstrict=-2 / -strict -2 to use it anyway.\n");
        return -1;
    }

    if(avctx->prediction_method == DWT_97
       && (avctx->flags & CODEC_FLAG_QSCALE)
       && avctx->global_quality == 0){
        av_log(avctx, AV_LOG_ERROR, "The 9/7 wavelet is incompatible with lossless mode.\n");
        return -1;
    }

    s->spatial_decomposition_type= avctx->prediction_method; //FIXME add decorrelator type r transform_type

    s->mv_scale       = (avctx->flags & CODEC_FLAG_QPEL) ? 2 : 4;
    s->block_max_depth= (avctx->flags & CODEC_FLAG_4MV ) ? 1 : 0;

    for(plane_index=0; plane_index<3; plane_index++){
        s->plane[plane_index].diag_mc= 1;
        s->plane[plane_index].htaps= 6;
        s->plane[plane_index].hcoeff[0]=  40;
        s->plane[plane_index].hcoeff[1]= -10;
        s->plane[plane_index].hcoeff[2]=   2;
        s->plane[plane_index].fast_mc= 1;
    }

    ff_snow_common_init(avctx);
    ff_snow_alloc_blocks(s);

    s->version=0;

    s->m.avctx   = avctx;
    s->m.flags   = avctx->flags;
    s->m.bit_rate= avctx->bit_rate;

    s->m.me.temp      =
    s->m.me.scratchpad= av_mallocz((avctx->width+64)*2*16*2*sizeof(uint8_t));
    s->m.me.map       = av_mallocz(ME_MAP_SIZE*sizeof(uint32_t));
    s->m.me.score_map = av_mallocz(ME_MAP_SIZE*sizeof(uint32_t));
    s->m.obmc_scratchpad= av_mallocz(MB_SIZE*MB_SIZE*12*sizeof(uint32_t));
    h263_encode_init(&s->m); //mv_penalty

    s->max_ref_frames = FFMAX(FFMIN(avctx->refs, MAX_REF_FRAMES), 1);

    if(avctx->flags&CODEC_FLAG_PASS1){
        if(!avctx->stats_out)
            avctx->stats_out = av_mallocz(256);
    }
    if((avctx->flags&CODEC_FLAG_PASS2) || !(avctx->flags&CODEC_FLAG_QSCALE)){
        if(ff_rate_control_init(&s->m) < 0)
            return -1;
    }
    s->pass1_rc= !(avctx->flags & (CODEC_FLAG_QSCALE|CODEC_FLAG_PASS2));

    avctx->coded_frame= &s->current_picture;
    switch(avctx->pix_fmt){
//    case PIX_FMT_YUV444P:
//    case PIX_FMT_YUV422P:
    case PIX_FMT_YUV420P:
//     case PIX_FMT_GRAY8:
//    case PIX_FMT_YUV411P:
//    case PIX_FMT_YUV410P:
        s->colorspace_type= 0;
        break;
/*    case PIX_FMT_RGB32:
        s->colorspace= 1;
        break;*/
    default:
        av_log(avctx, AV_LOG_ERROR, "pixel format not supported\n");
        return -1;
    }
//    avcodec_get_chroma_sub_sample(avctx->pix_fmt, &s->chroma_h_shift, &s->chroma_v_shift);
    s->chroma_h_shift= 1;
    s->chroma_v_shift= 1;

    ff_set_cmp(&s->dsp, s->dsp.me_cmp, s->avctx->me_cmp);
    ff_set_cmp(&s->dsp, s->dsp.me_sub_cmp, s->avctx->me_sub_cmp);

    s->avctx->get_buffer(s->avctx, &s->input_picture);

    if(s->avctx->me_method == ME_ITER){
        int i;
        int size= s->b_width * s->b_height << 2*s->block_max_depth;
        for(i=0; i<s->max_ref_frames; i++){
            s->ref_mvs[i]= av_mallocz(size*sizeof(int16_t[2]));
            s->ref_scores[i]= av_mallocz(size*sizeof(uint32_t));
        }
    }

    return 0;
}

//near copy & paste from dsputil, FIXME
static int pix_sum(uint8_t * pix, int line_size, int w)
{
    int s, i, j;

    s = 0;
    for (i = 0; i < w; i++) {
        for (j = 0; j < w; j++) {
            s += pix[0];
            pix ++;
        }
        pix += line_size - w;
    }
    return s;
}

//near copy & paste from dsputil, FIXME
static int pix_norm1(uint8_t * pix, int line_size, int w)
{
    int s, i, j;
    uint32_t *sq = ff_squareTbl + 256;

    s = 0;
    for (i = 0; i < w; i++) {
        for (j = 0; j < w; j ++) {
            s += sq[pix[0]];
            pix ++;
        }
        pix += line_size - w;
    }
    return s;
}

//FIXME copy&paste
#define P_LEFT P[1]
#define P_TOP P[2]
#define P_TOPRIGHT P[3]
#define P_MEDIAN P[4]
#define P_MV1 P[9]
#define FLAG_QPEL   1 //must be 1

static int encode_q_branch(SnowContext *s, int level, int x, int y){
    uint8_t p_buffer[1024];
    uint8_t i_buffer[1024];
    uint8_t p_state[sizeof(s->block_state)];
    uint8_t i_state[sizeof(s->block_state)];
    RangeCoder pc, ic;
    uint8_t *pbbak= s->c.bytestream;
    uint8_t *pbbak_start= s->c.bytestream_start;
    int score, score2, iscore, i_len, p_len, block_s, sum, base_bits;
    const int w= s->b_width  << s->block_max_depth;
    const int h= s->b_height << s->block_max_depth;
    const int rem_depth= s->block_max_depth - level;
    const int index= (x + y*w) << rem_depth;
    const int block_w= 1<<(LOG2_MB_SIZE - level);
    int trx= (x+1)<<rem_depth;
    int try= (y+1)<<rem_depth;
    const BlockNode *left  = x ? &s->block[index-1] : &null_block;
    const BlockNode *top   = y ? &s->block[index-w] : &null_block;
    const BlockNode *right = trx<w ? &s->block[index+1] : &null_block;
    const BlockNode *bottom= try<h ? &s->block[index+w] : &null_block;
    const BlockNode *tl    = y && x ? &s->block[index-w-1] : left;
    const BlockNode *tr    = y && trx<w && ((x&1)==0 || level==0) ? &s->block[index-w+(1<<rem_depth)] : tl; //FIXME use lt
    int pl = left->color[0];
    int pcb= left->color[1];
    int pcr= left->color[2];
    int pmx, pmy;
    int mx=0, my=0;
    int l,cr,cb;
    const int stride= s->current_picture.linesize[0];
    const int uvstride= s->current_picture.linesize[1];
    uint8_t *current_data[3]= { s->input_picture.data[0] + (x + y*  stride)*block_w,
                                s->input_picture.data[1] + (x + y*uvstride)*block_w/2,
                                s->input_picture.data[2] + (x + y*uvstride)*block_w/2};
    int P[10][2];
    int16_t last_mv[3][2];
    int qpel= !!(s->avctx->flags & CODEC_FLAG_QPEL); //unused
    const int shift= 1+qpel;
    MotionEstContext *c= &s->m.me;
    int ref_context= av_log2(2*left->ref) + av_log2(2*top->ref);
    int mx_context= av_log2(2*FFABS(left->mx - top->mx));
    int my_context= av_log2(2*FFABS(left->my - top->my));
    int s_context= 2*left->level + 2*top->level + tl->level + tr->level;
    int ref, best_ref, ref_score, ref_mx, ref_my;

    assert(sizeof(s->block_state) >= 256);
    if(s->keyframe){
        set_blocks(s, level, x, y, pl, pcb, pcr, 0, 0, 0, BLOCK_INTRA);
        return 0;
    }

//    clip predictors / edge ?

    P_LEFT[0]= left->mx;
    P_LEFT[1]= left->my;
    P_TOP [0]= top->mx;
    P_TOP [1]= top->my;
    P_TOPRIGHT[0]= tr->mx;
    P_TOPRIGHT[1]= tr->my;

    last_mv[0][0]= s->block[index].mx;
    last_mv[0][1]= s->block[index].my;
    last_mv[1][0]= right->mx;
    last_mv[1][1]= right->my;
    last_mv[2][0]= bottom->mx;
    last_mv[2][1]= bottom->my;

    s->m.mb_stride=2;
    s->m.mb_x=
    s->m.mb_y= 0;
    c->skip= 0;

    assert(c->  stride ==   stride);
    assert(c->uvstride == uvstride);

    c->penalty_factor    = get_penalty_factor(s->lambda, s->lambda2, c->avctx->me_cmp);
    c->sub_penalty_factor= get_penalty_factor(s->lambda, s->lambda2, c->avctx->me_sub_cmp);
    c->mb_penalty_factor = get_penalty_factor(s->lambda, s->lambda2, c->avctx->mb_cmp);
    c->current_mv_penalty= c->mv_penalty[s->m.f_code=1] + MAX_MV;

    c->xmin = - x*block_w - 16+3;
    c->ymin = - y*block_w - 16+3;
    c->xmax = - (x+1)*block_w + (w<<(LOG2_MB_SIZE - s->block_max_depth)) + 16-3;
    c->ymax = - (y+1)*block_w + (h<<(LOG2_MB_SIZE - s->block_max_depth)) + 16-3;

    if(P_LEFT[0]     > (c->xmax<<shift)) P_LEFT[0]    = (c->xmax<<shift);
    if(P_LEFT[1]     > (c->ymax<<shift)) P_LEFT[1]    = (c->ymax<<shift);
    if(P_TOP[0]      > (c->xmax<<shift)) P_TOP[0]     = (c->xmax<<shift);
    if(P_TOP[1]      > (c->ymax<<shift)) P_TOP[1]     = (c->ymax<<shift);
    if(P_TOPRIGHT[0] < (c->xmin<<shift)) P_TOPRIGHT[0]= (c->xmin<<shift);
    if(P_TOPRIGHT[0] > (c->xmax<<shift)) P_TOPRIGHT[0]= (c->xmax<<shift); //due to pmx no clip
    if(P_TOPRIGHT[1] > (c->ymax<<shift)) P_TOPRIGHT[1]= (c->ymax<<shift);

    P_MEDIAN[0]= mid_pred(P_LEFT[0], P_TOP[0], P_TOPRIGHT[0]);
    P_MEDIAN[1]= mid_pred(P_LEFT[1], P_TOP[1], P_TOPRIGHT[1]);

    if (!y) {
        c->pred_x= P_LEFT[0];
        c->pred_y= P_LEFT[1];
    } else {
        c->pred_x = P_MEDIAN[0];
        c->pred_y = P_MEDIAN[1];
    }

    score= INT_MAX;
    best_ref= 0;
    for(ref=0; ref<s->ref_frames; ref++){
        init_ref(c, current_data, s->last_picture[ref].data, NULL, block_w*x, block_w*y, 0);

        ref_score= ff_epzs_motion_search(&s->m, &ref_mx, &ref_my, P, 0, /*ref_index*/ 0, last_mv,
                                         (1<<16)>>shift, level-LOG2_MB_SIZE+4, block_w);

        assert(ref_mx >= c->xmin);
        assert(ref_mx <= c->xmax);
        assert(ref_my >= c->ymin);
        assert(ref_my <= c->ymax);

        ref_score= c->sub_motion_search(&s->m, &ref_mx, &ref_my, ref_score, 0, 0, level-LOG2_MB_SIZE+4, block_w);
        ref_score= ff_get_mb_score(&s->m, ref_mx, ref_my, 0, 0, level-LOG2_MB_SIZE+4, block_w, 0);
        ref_score+= 2*av_log2(2*ref)*c->penalty_factor;
        if(s->ref_mvs[ref]){
            s->ref_mvs[ref][index][0]= ref_mx;
            s->ref_mvs[ref][index][1]= ref_my;
            s->ref_scores[ref][index]= ref_score;
        }
        if(score > ref_score){
            score= ref_score;
            best_ref= ref;
            mx= ref_mx;
            my= ref_my;
        }
    }
    //FIXME if mb_cmp != SSE then intra cannot be compared currently and mb_penalty vs. lambda2

  //  subpel search
    base_bits= get_rac_count(&s->c) - 8*(s->c.bytestream - s->c.bytestream_start);
    pc= s->c;
    pc.bytestream_start=
    pc.bytestream= p_buffer; //FIXME end/start? and at the other stoo
    memcpy(p_state, s->block_state, sizeof(s->block_state));

    if(level!=s->block_max_depth)
        put_rac(&pc, &p_state[4 + s_context], 1);
    put_rac(&pc, &p_state[1 + left->type + top->type], 0);
    if(s->ref_frames > 1)
        put_symbol(&pc, &p_state[128 + 1024 + 32*ref_context], best_ref, 0);
    pred_mv(s, &pmx, &pmy, best_ref, left, top, tr);
    put_symbol(&pc, &p_state[128 + 32*(mx_context + 16*!!best_ref)], mx - pmx, 1);
    put_symbol(&pc, &p_state[128 + 32*(my_context + 16*!!best_ref)], my - pmy, 1);
    p_len= pc.bytestream - pc.bytestream_start;
    score += (s->lambda2*(get_rac_count(&pc)-base_bits))>>FF_LAMBDA_SHIFT;

    block_s= block_w*block_w;
    sum = pix_sum(current_data[0], stride, block_w);
    l= (sum + block_s/2)/block_s;
    iscore = pix_norm1(current_data[0], stride, block_w) - 2*l*sum + l*l*block_s;

    block_s= block_w*block_w>>2;
    sum = pix_sum(current_data[1], uvstride, block_w>>1);
    cb= (sum + block_s/2)/block_s;
//    iscore += pix_norm1(&current_mb[1][0], uvstride, block_w>>1) - 2*cb*sum + cb*cb*block_s;
    sum = pix_sum(current_data[2], uvstride, block_w>>1);
    cr= (sum + block_s/2)/block_s;
//    iscore += pix_norm1(&current_mb[2][0], uvstride, block_w>>1) - 2*cr*sum + cr*cr*block_s;

    ic= s->c;
    ic.bytestream_start=
    ic.bytestream= i_buffer; //FIXME end/start? and at the other stoo
    memcpy(i_state, s->block_state, sizeof(s->block_state));
    if(level!=s->block_max_depth)
        put_rac(&ic, &i_state[4 + s_context], 1);
    put_rac(&ic, &i_state[1 + left->type + top->type], 1);
    put_symbol(&ic, &i_state[32],  l-pl , 1);
    put_symbol(&ic, &i_state[64], cb-pcb, 1);
    put_symbol(&ic, &i_state[96], cr-pcr, 1);
    i_len= ic.bytestream - ic.bytestream_start;
    iscore += (s->lambda2*(get_rac_count(&ic)-base_bits))>>FF_LAMBDA_SHIFT;

//    assert(score==256*256*256*64-1);
    assert(iscore < 255*255*256 + s->lambda2*10);
    assert(iscore >= 0);
    assert(l>=0 && l<=255);
    assert(pl>=0 && pl<=255);

    if(level==0){
        int varc= iscore >> 8;
        int vard= score >> 8;
        if (vard <= 64 || vard < varc)
            c->scene_change_score+= ff_sqrt(vard) - ff_sqrt(varc);
        else
            c->scene_change_score+= s->m.qscale;
    }

    if(level!=s->block_max_depth){
        put_rac(&s->c, &s->block_state[4 + s_context], 0);
        score2 = encode_q_branch(s, level+1, 2*x+0, 2*y+0);
        score2+= encode_q_branch(s, level+1, 2*x+1, 2*y+0);
        score2+= encode_q_branch(s, level+1, 2*x+0, 2*y+1);
        score2+= encode_q_branch(s, level+1, 2*x+1, 2*y+1);
        score2+= s->lambda2>>FF_LAMBDA_SHIFT; //FIXME exact split overhead

        if(score2 < score && score2 < iscore)
            return score2;
    }

    if(iscore < score){
        pred_mv(s, &pmx, &pmy, 0, left, top, tr);
        memcpy(pbbak, i_buffer, i_len);
        s->c= ic;
        s->c.bytestream_start= pbbak_start;
        s->c.bytestream= pbbak + i_len;
        set_blocks(s, level, x, y, l, cb, cr, pmx, pmy, 0, BLOCK_INTRA);
        memcpy(s->block_state, i_state, sizeof(s->block_state));
        return iscore;
    }else{
        memcpy(pbbak, p_buffer, p_len);
        s->c= pc;
        s->c.bytestream_start= pbbak_start;
        s->c.bytestream= pbbak + p_len;
        set_blocks(s, level, x, y, pl, pcb, pcr, mx, my, best_ref, 0);
        memcpy(s->block_state, p_state, sizeof(s->block_state));
        return score;
    }
}

static void encode_q_branch2(SnowContext *s, int level, int x, int y){
    const int w= s->b_width  << s->block_max_depth;
    const int rem_depth= s->block_max_depth - level;
    const int index= (x + y*w) << rem_depth;
    int trx= (x+1)<<rem_depth;
    BlockNode *b= &s->block[index];
    const BlockNode *left  = x ? &s->block[index-1] : &null_block;
    const BlockNode *top   = y ? &s->block[index-w] : &null_block;
    const BlockNode *tl    = y && x ? &s->block[index-w-1] : left;
    const BlockNode *tr    = y && trx<w && ((x&1)==0 || level==0) ? &s->block[index-w+(1<<rem_depth)] : tl; //FIXME use lt
    int pl = left->color[0];
    int pcb= left->color[1];
    int pcr= left->color[2];
    int pmx, pmy;
    int ref_context= av_log2(2*left->ref) + av_log2(2*top->ref);
    int mx_context= av_log2(2*FFABS(left->mx - top->mx)) + 16*!!b->ref;
    int my_context= av_log2(2*FFABS(left->my - top->my)) + 16*!!b->ref;
    int s_context= 2*left->level + 2*top->level + tl->level + tr->level;

    if(s->keyframe){
        set_blocks(s, level, x, y, pl, pcb, pcr, 0, 0, 0, BLOCK_INTRA);
        return;
    }

    if(level!=s->block_max_depth){
        if(same_block(b,b+1) && same_block(b,b+w) && same_block(b,b+w+1)){
            put_rac(&s->c, &s->block_state[4 + s_context], 1);
        }else{
            put_rac(&s->c, &s->block_state[4 + s_context], 0);
            encode_q_branch2(s, level+1, 2*x+0, 2*y+0);
            encode_q_branch2(s, level+1, 2*x+1, 2*y+0);
            encode_q_branch2(s, level+1, 2*x+0, 2*y+1);
            encode_q_branch2(s, level+1, 2*x+1, 2*y+1);
            return;
        }
    }
    if(b->type & BLOCK_INTRA){
        pred_mv(s, &pmx, &pmy, 0, left, top, tr);
        put_rac(&s->c, &s->block_state[1 + (left->type&1) + (top->type&1)], 1);
        put_symbol(&s->c, &s->block_state[32], b->color[0]-pl , 1);
        put_symbol(&s->c, &s->block_state[64], b->color[1]-pcb, 1);
        put_symbol(&s->c, &s->block_state[96], b->color[2]-pcr, 1);
        set_blocks(s, level, x, y, b->color[0], b->color[1], b->color[2], pmx, pmy, 0, BLOCK_INTRA);
    }else{
        pred_mv(s, &pmx, &pmy, b->ref, left, top, tr);
        put_rac(&s->c, &s->block_state[1 + (left->type&1) + (top->type&1)], 0);
        if(s->ref_frames > 1)
            put_symbol(&s->c, &s->block_state[128 + 1024 + 32*ref_context], b->ref, 0);
        put_symbol(&s->c, &s->block_state[128 + 32*mx_context], b->mx - pmx, 1);
        put_symbol(&s->c, &s->block_state[128 + 32*my_context], b->my - pmy, 1);
        set_blocks(s, level, x, y, pl, pcb, pcr, b->mx, b->my, b->ref, 0);
    }
}

static int get_dc(SnowContext *s, int mb_x, int mb_y, int plane_index){
    int i, x2, y2;
    Plane *p= &s->plane[plane_index];
    const int block_size = MB_SIZE >> s->block_max_depth;
    const int block_w    = plane_index ? block_size/2 : block_size;
    const uint8_t *obmc  = plane_index ? obmc_tab[s->block_max_depth+1] : obmc_tab[s->block_max_depth];
    const int obmc_stride= plane_index ? block_size : 2*block_size;
    const int ref_stride= s->current_picture.linesize[plane_index];
    uint8_t *src= s-> input_picture.data[plane_index];
    IDWTELEM *dst= (IDWTELEM*)s->m.obmc_scratchpad + plane_index*block_size*block_size*4; //FIXME change to unsigned
    const int b_stride = s->b_width << s->block_max_depth;
    const int w= p->width;
    const int h= p->height;
    int index= mb_x + mb_y*b_stride;
    BlockNode *b= &s->block[index];
    BlockNode backup= *b;
    int ab=0;
    int aa=0;

    b->type|= BLOCK_INTRA;
    b->color[plane_index]= 0;
    memset(dst, 0, obmc_stride*obmc_stride*sizeof(IDWTELEM));

    for(i=0; i<4; i++){
        int mb_x2= mb_x + (i &1) - 1;
        int mb_y2= mb_y + (i>>1) - 1;
        int x= block_w*mb_x2 + block_w/2;
        int y= block_w*mb_y2 + block_w/2;

        add_yblock(s, 0, NULL, dst + ((i&1)+(i>>1)*obmc_stride)*block_w, NULL, obmc,
                    x, y, block_w, block_w, w, h, obmc_stride, ref_stride, obmc_stride, mb_x2, mb_y2, 0, 0, plane_index);

        for(y2= FFMAX(y, 0); y2<FFMIN(h, y+block_w); y2++){
            for(x2= FFMAX(x, 0); x2<FFMIN(w, x+block_w); x2++){
                int index= x2-(block_w*mb_x - block_w/2) + (y2-(block_w*mb_y - block_w/2))*obmc_stride;
                int obmc_v= obmc[index];
                int d;
                if(y<0) obmc_v += obmc[index + block_w*obmc_stride];
                if(x<0) obmc_v += obmc[index + block_w];
                if(y+block_w>h) obmc_v += obmc[index - block_w*obmc_stride];
                if(x+block_w>w) obmc_v += obmc[index - block_w];
                //FIXME precalculate this or simplify it somehow else

                d = -dst[index] + (1<<(FRAC_BITS-1));
                dst[index] = d;
                ab += (src[x2 + y2*ref_stride] - (d>>FRAC_BITS)) * obmc_v;
                aa += obmc_v * obmc_v; //FIXME precalculate this
            }
        }
    }
    *b= backup;

    return av_clip(((ab<<LOG2_OBMC_MAX) + aa/2)/aa, 0, 255); //FIXME we should not need clipping
}

static inline int get_block_bits(SnowContext *s, int x, int y, int w){
    const int b_stride = s->b_width << s->block_max_depth;
    const int b_height = s->b_height<< s->block_max_depth;
    int index= x + y*b_stride;
    const BlockNode *b     = &s->block[index];
    const BlockNode *left  = x ? &s->block[index-1] : &null_block;
    const BlockNode *top   = y ? &s->block[index-b_stride] : &null_block;
    const BlockNode *tl    = y && x ? &s->block[index-b_stride-1] : left;
    const BlockNode *tr    = y && x+w<b_stride ? &s->block[index-b_stride+w] : tl;
    int dmx, dmy;
//  int mx_context= av_log2(2*FFABS(left->mx - top->mx));
//  int my_context= av_log2(2*FFABS(left->my - top->my));

    if(x<0 || x>=b_stride || y>=b_height)
        return 0;
/*
1            0      0
01X          1-2    1
001XX        3-6    2-3
0001XXX      7-14   4-7
00001XXXX   15-30   8-15
*/
//FIXME try accurate rate
//FIXME intra and inter predictors if surrounding blocks are not the same type
    if(b->type & BLOCK_INTRA){
        return 3+2*( av_log2(2*FFABS(left->color[0] - b->color[0]))
                   + av_log2(2*FFABS(left->color[1] - b->color[1]))
                   + av_log2(2*FFABS(left->color[2] - b->color[2])));
    }else{
        pred_mv(s, &dmx, &dmy, b->ref, left, top, tr);
        dmx-= b->mx;
        dmy-= b->my;
        return 2*(1 + av_log2(2*FFABS(dmx)) //FIXME kill the 2* can be merged in lambda
                    + av_log2(2*FFABS(dmy))
                    + av_log2(2*b->ref));
    }
}

static int get_block_rd(SnowContext *s, int mb_x, int mb_y, int plane_index, const uint8_t *obmc_edged){
    Plane *p= &s->plane[plane_index];
    const int block_size = MB_SIZE >> s->block_max_depth;
    const int block_w    = plane_index ? block_size/2 : block_size;
    const int obmc_stride= plane_index ? block_size : 2*block_size;
    const int ref_stride= s->current_picture.linesize[plane_index];
    uint8_t *dst= s->current_picture.data[plane_index];
    uint8_t *src= s->  input_picture.data[plane_index];
    IDWTELEM *pred= (IDWTELEM*)s->m.obmc_scratchpad + plane_index*block_size*block_size*4;
    uint8_t *cur = s->scratchbuf;
    uint8_t tmp[ref_stride*(2*MB_SIZE+HTAPS_MAX-1)];
    const int b_stride = s->b_width << s->block_max_depth;
    const int b_height = s->b_height<< s->block_max_depth;
    const int w= p->width;
    const int h= p->height;
    int distortion;
    int rate= 0;
    const int penalty_factor= get_penalty_factor(s->lambda, s->lambda2, s->avctx->me_cmp);
    int sx= block_w*mb_x - block_w/2;
    int sy= block_w*mb_y - block_w/2;
    int x0= FFMAX(0,-sx);
    int y0= FFMAX(0,-sy);
    int x1= FFMIN(block_w*2, w-sx);
    int y1= FFMIN(block_w*2, h-sy);
    int i,x,y;

    ff_snow_pred_block(s, cur, tmp, ref_stride, sx, sy, block_w*2, block_w*2, &s->block[mb_x + mb_y*b_stride], plane_index, w, h);

    for(y=y0; y<y1; y++){
        const uint8_t *obmc1= obmc_edged + y*obmc_stride;
        const IDWTELEM *pred1 = pred + y*obmc_stride;
        uint8_t *cur1 = cur + y*ref_stride;
        uint8_t *dst1 = dst + sx + (sy+y)*ref_stride;
        for(x=x0; x<x1; x++){
#if FRAC_BITS >= LOG2_OBMC_MAX
            int v = (cur1[x] * obmc1[x]) << (FRAC_BITS - LOG2_OBMC_MAX);
#else
            int v = (cur1[x] * obmc1[x] + (1<<(LOG2_OBMC_MAX - FRAC_BITS-1))) >> (LOG2_OBMC_MAX - FRAC_BITS);
#endif
            v = (v + pred1[x]) >> FRAC_BITS;
            if(v&(~255)) v= ~(v>>31);
            dst1[x] = v;
        }
    }

    /* copy the regions where obmc[] = (uint8_t)256 */
    if(LOG2_OBMC_MAX == 8
        && (mb_x == 0 || mb_x == b_stride-1)
        && (mb_y == 0 || mb_y == b_height-1)){
        if(mb_x == 0)
            x1 = block_w;
        else
            x0 = block_w;
        if(mb_y == 0)
            y1 = block_w;
        else
            y0 = block_w;
        for(y=y0; y<y1; y++)
            memcpy(dst + sx+x0 + (sy+y)*ref_stride, cur + x0 + y*ref_stride, x1-x0);
    }

    if(block_w==16){
        /* FIXME rearrange dsputil to fit 32x32 cmp functions */
        /* FIXME check alignment of the cmp wavelet vs the encoding wavelet */
        /* FIXME cmps overlap but do not cover the wavelet's whole support.
         * So improving the score of one block is not strictly guaranteed
         * to improve the score of the whole frame, thus iterative motion
         * estimation does not always converge. */
        if(s->avctx->me_cmp == FF_CMP_W97)
            distortion = ff_w97_32_c(&s->m, src + sx + sy*ref_stride, dst + sx + sy*ref_stride, ref_stride, 32);
        else if(s->avctx->me_cmp == FF_CMP_W53)
            distortion = ff_w53_32_c(&s->m, src + sx + sy*ref_stride, dst + sx + sy*ref_stride, ref_stride, 32);
        else{
            distortion = 0;
            for(i=0; i<4; i++){
                int off = sx+16*(i&1) + (sy+16*(i>>1))*ref_stride;
                distortion += s->dsp.me_cmp[0](&s->m, src + off, dst + off, ref_stride, 16);
            }
        }
    }else{
        assert(block_w==8);
        distortion = s->dsp.me_cmp[0](&s->m, src + sx + sy*ref_stride, dst + sx + sy*ref_stride, ref_stride, block_w*2);
    }

    if(plane_index==0){
        for(i=0; i<4; i++){
/* ..RRr
 * .RXx.
 * rxx..
 */
            rate += get_block_bits(s, mb_x + (i&1) - (i>>1), mb_y + (i>>1), 1);
        }
        if(mb_x == b_stride-2)
            rate += get_block_bits(s, mb_x + 1, mb_y + 1, 1);
    }
    return distortion + rate*penalty_factor;
}

static int get_4block_rd(SnowContext *s, int mb_x, int mb_y, int plane_index){
    int i, y2;
    Plane *p= &s->plane[plane_index];
    const int block_size = MB_SIZE >> s->block_max_depth;
    const int block_w    = plane_index ? block_size/2 : block_size;
    const uint8_t *obmc  = plane_index ? obmc_tab[s->block_max_depth+1] : obmc_tab[s->block_max_depth];
    const int obmc_stride= plane_index ? block_size : 2*block_size;
    const int ref_stride= s->current_picture.linesize[plane_index];
    uint8_t *dst= s->current_picture.data[plane_index];
    uint8_t *src= s-> input_picture.data[plane_index];
    //FIXME zero_dst is const but add_yblock changes dst if add is 0 (this is never the case for dst=zero_dst
    // const has only been removed from zero_dst to suppress a warning
    static IDWTELEM zero_dst[4096]; //FIXME
    const int b_stride = s->b_width << s->block_max_depth;
    const int w= p->width;
    const int h= p->height;
    int distortion= 0;
    int rate= 0;
    const int penalty_factor= get_penalty_factor(s->lambda, s->lambda2, s->avctx->me_cmp);

    for(i=0; i<9; i++){
        int mb_x2= mb_x + (i%3) - 1;
        int mb_y2= mb_y + (i/3) - 1;
        int x= block_w*mb_x2 + block_w/2;
        int y= block_w*mb_y2 + block_w/2;

        add_yblock(s, 0, NULL, zero_dst, dst, obmc,
                   x, y, block_w, block_w, w, h, /*dst_stride*/0, ref_stride, obmc_stride, mb_x2, mb_y2, 1, 1, plane_index);

        //FIXME find a cleaner/simpler way to skip the outside stuff
        for(y2= y; y2<0; y2++)
            memcpy(dst + x + y2*ref_stride, src + x + y2*ref_stride, block_w);
        for(y2= h; y2<y+block_w; y2++)
            memcpy(dst + x + y2*ref_stride, src + x + y2*ref_stride, block_w);
        if(x<0){
            for(y2= y; y2<y+block_w; y2++)
                memcpy(dst + x + y2*ref_stride, src + x + y2*ref_stride, -x);
        }
        if(x+block_w > w){
            for(y2= y; y2<y+block_w; y2++)
                memcpy(dst + w + y2*ref_stride, src + w + y2*ref_stride, x+block_w - w);
        }

        assert(block_w== 8 || block_w==16);
        distortion += s->dsp.me_cmp[block_w==8](&s->m, src + x + y*ref_stride, dst + x + y*ref_stride, ref_stride, block_w);
    }

    if(plane_index==0){
        BlockNode *b= &s->block[mb_x+mb_y*b_stride];
        int merged= same_block(b,b+1) && same_block(b,b+b_stride) && same_block(b,b+b_stride+1);

/* ..RRRr
 * .RXXx.
 * .RXXx.
 * rxxx.
 */
        if(merged)
            rate = get_block_bits(s, mb_x, mb_y, 2);
        for(i=merged?4:0; i<9; i++){
            static const int dxy[9][2] = {{0,0},{1,0},{0,1},{1,1},{2,0},{2,1},{-1,2},{0,2},{1,2}};
            rate += get_block_bits(s, mb_x + dxy[i][0], mb_y + dxy[i][1], 1);
        }
    }
    return distortion + rate*penalty_factor;
}

static int encode_subband_c0run(SnowContext *s, SubBand *b, IDWTELEM *src, IDWTELEM *parent, int stride, int orientation){
    const int w= b->width;
    const int h= b->height;
    int x, y;

    if(1){
        int run=0;
        int runs[w*h];
        int run_index=0;
        int max_index;

        for(y=0; y<h; y++){
            for(x=0; x<w; x++){
                int v, p=0;
                int /*ll=0, */l=0, lt=0, t=0, rt=0;
                v= src[x + y*stride];

                if(y){
                    t= src[x + (y-1)*stride];
                    if(x){
                        lt= src[x - 1 + (y-1)*stride];
                    }
                    if(x + 1 < w){
                        rt= src[x + 1 + (y-1)*stride];
                    }
                }
                if(x){
                    l= src[x - 1 + y*stride];
                    /*if(x > 1){
                        if(orientation==1) ll= src[y + (x-2)*stride];
                        else               ll= src[x - 2 + y*stride];
                    }*/
                }
                if(parent){
                    int px= x>>1;
                    int py= y>>1;
                    if(px<b->parent->width && py<b->parent->height)
                        p= parent[px + py*2*stride];
                }
                if(!(/*ll|*/l|lt|t|rt|p)){
                    if(v){
                        runs[run_index++]= run;
                        run=0;
                    }else{
                        run++;
                    }
                }
            }
        }
        max_index= run_index;
        runs[run_index++]= run;
        run_index=0;
        run= runs[run_index++];

        put_symbol2(&s->c, b->state[30], max_index, 0);
        if(run_index <= max_index)
            put_symbol2(&s->c, b->state[1], run, 3);

        for(y=0; y<h; y++){
            if(s->c.bytestream_end - s->c.bytestream < w*40){
                av_log(s->avctx, AV_LOG_ERROR, "encoded frame too large\n");
                return -1;
            }
            for(x=0; x<w; x++){
                int v, p=0;
                int /*ll=0, */l=0, lt=0, t=0, rt=0;
                v= src[x + y*stride];

                if(y){
                    t= src[x + (y-1)*stride];
                    if(x){
                        lt= src[x - 1 + (y-1)*stride];
                    }
                    if(x + 1 < w){
                        rt= src[x + 1 + (y-1)*stride];
                    }
                }
                if(x){
                    l= src[x - 1 + y*stride];
                    /*if(x > 1){
                        if(orientation==1) ll= src[y + (x-2)*stride];
                        else               ll= src[x - 2 + y*stride];
                    }*/
                }
                if(parent){
                    int px= x>>1;
                    int py= y>>1;
                    if(px<b->parent->width && py<b->parent->height)
                        p= parent[px + py*2*stride];
                }
                if(/*ll|*/l|lt|t|rt|p){
                    int context= av_log2(/*FFABS(ll) + */3*FFABS(l) + FFABS(lt) + 2*FFABS(t) + FFABS(rt) + FFABS(p));

                    put_rac(&s->c, &b->state[0][context], !!v);
                }else{
                    if(!run){
                        run= runs[run_index++];

                        if(run_index <= max_index)
                            put_symbol2(&s->c, b->state[1], run, 3);
                        assert(v);
                    }else{
                        run--;
                        assert(!v);
                    }
                }
                if(v){
                    int context= av_log2(/*FFABS(ll) + */3*FFABS(l) + FFABS(lt) + 2*FFABS(t) + FFABS(rt) + FFABS(p));
                    int l2= 2*FFABS(l) + (l<0);
                    int t2= 2*FFABS(t) + (t<0);

                    put_symbol2(&s->c, b->state[context + 2], FFABS(v)-1, context-4);
                    put_rac(&s->c, &b->state[0][16 + 1 + 3 + quant3bA[l2&0xFF] + 3*quant3bA[t2&0xFF]], v<0);
                }
            }
        }
    }
    return 0;
}

static int encode_subband(SnowContext *s, SubBand *b, IDWTELEM *src, IDWTELEM *parent, int stride, int orientation){
//    encode_subband_qtree(s, b, src, parent, stride, orientation);
//    encode_subband_z0run(s, b, src, parent, stride, orientation);
    return encode_subband_c0run(s, b, src, parent, stride, orientation);
//    encode_subband_dzr(s, b, src, parent, stride, orientation);
}

static av_always_inline int check_block(SnowContext *s, int mb_x, int mb_y, int p[3], int intra, const uint8_t *obmc_edged, int *best_rd){
    const int b_stride= s->b_width << s->block_max_depth;
    BlockNode *block= &s->block[mb_x + mb_y * b_stride];
    BlockNode backup= *block;
    unsigned value;
    int rd, index;

    assert(mb_x>=0 && mb_y>=0);
    assert(mb_x<b_stride);

    if(intra){
        block->color[0] = p[0];
        block->color[1] = p[1];
        block->color[2] = p[2];
        block->type |= BLOCK_INTRA;
    }else{
        index= (p[0] + 31*p[1]) & (ME_CACHE_SIZE-1);
        value= s->me_cache_generation + (p[0]>>10) + (p[1]<<6) + (block->ref<<12);
        if(s->me_cache[index] == value)
            return 0;
        s->me_cache[index]= value;

        block->mx= p[0];
        block->my= p[1];
        block->type &= ~BLOCK_INTRA;
    }

    rd= get_block_rd(s, mb_x, mb_y, 0, obmc_edged);

//FIXME chroma
    if(rd < *best_rd){
        *best_rd= rd;
        return 1;
    }else{
        *block= backup;
        return 0;
    }
}

/* special case for int[2] args we discard afterwards,
 * fixes compilation problem with gcc 2.95 */
static av_always_inline int check_block_inter(SnowContext *s, int mb_x, int mb_y, int p0, int p1, const uint8_t *obmc_edged, int *best_rd){
    int p[2] = {p0, p1};
    return check_block(s, mb_x, mb_y, p, 0, obmc_edged, best_rd);
}

static av_always_inline int check_4block_inter(SnowContext *s, int mb_x, int mb_y, int p0, int p1, int ref, int *best_rd){
    const int b_stride= s->b_width << s->block_max_depth;
    BlockNode *block= &s->block[mb_x + mb_y * b_stride];
    BlockNode backup[4]= {block[0], block[1], block[b_stride], block[b_stride+1]};
    unsigned value;
    int rd, index;

    assert(mb_x>=0 && mb_y>=0);
    assert(mb_x<b_stride);
    assert(((mb_x|mb_y)&1) == 0);

    index= (p0 + 31*p1) & (ME_CACHE_SIZE-1);
    value= s->me_cache_generation + (p0>>10) + (p1<<6) + (block->ref<<12);
    if(s->me_cache[index] == value)
        return 0;
    s->me_cache[index]= value;

    block->mx= p0;
    block->my= p1;
    block->ref= ref;
    block->type &= ~BLOCK_INTRA;
    block[1]= block[b_stride]= block[b_stride+1]= *block;

    rd= get_4block_rd(s, mb_x, mb_y, 0);

//FIXME chroma
    if(rd < *best_rd){
        *best_rd= rd;
        return 1;
    }else{
        block[0]= backup[0];
        block[1]= backup[1];
        block[b_stride]= backup[2];
        block[b_stride+1]= backup[3];
        return 0;
    }
}

static void iterative_me(SnowContext *s){
    int pass, mb_x, mb_y;
    const int b_width = s->b_width  << s->block_max_depth;
    const int b_height= s->b_height << s->block_max_depth;
    const int b_stride= b_width;
    int color[3];

    {
        RangeCoder r = s->c;
        uint8_t state[sizeof(s->block_state)];
        memcpy(state, s->block_state, sizeof(s->block_state));
        for(mb_y= 0; mb_y<s->b_height; mb_y++)
            for(mb_x= 0; mb_x<s->b_width; mb_x++)
                encode_q_branch(s, 0, mb_x, mb_y);
        s->c = r;
        memcpy(s->block_state, state, sizeof(s->block_state));
    }

    for(pass=0; pass<25; pass++){
        int change= 0;

        for(mb_y= 0; mb_y<b_height; mb_y++){
            for(mb_x= 0; mb_x<b_width; mb_x++){
                int dia_change, i, j, ref;
                int best_rd= INT_MAX, ref_rd;
                BlockNode backup, ref_b;
                const int index= mb_x + mb_y * b_stride;
                BlockNode *block= &s->block[index];
                BlockNode *tb =                   mb_y            ? &s->block[index-b_stride  ] : NULL;
                BlockNode *lb = mb_x                              ? &s->block[index         -1] : NULL;
                BlockNode *rb = mb_x+1<b_width                    ? &s->block[index         +1] : NULL;
                BlockNode *bb =                   mb_y+1<b_height ? &s->block[index+b_stride  ] : NULL;
                BlockNode *tlb= mb_x           && mb_y            ? &s->block[index-b_stride-1] : NULL;
                BlockNode *trb= mb_x+1<b_width && mb_y            ? &s->block[index-b_stride+1] : NULL;
                BlockNode *blb= mb_x           && mb_y+1<b_height ? &s->block[index+b_stride-1] : NULL;
                BlockNode *brb= mb_x+1<b_width && mb_y+1<b_height ? &s->block[index+b_stride+1] : NULL;
                const int b_w= (MB_SIZE >> s->block_max_depth);
                uint8_t obmc_edged[b_w*2][b_w*2];

                if(pass && (block->type & BLOCK_OPT))
                    continue;
                block->type |= BLOCK_OPT;

                backup= *block;

                if(!s->me_cache_generation)
                    memset(s->me_cache, 0, sizeof(s->me_cache));
                s->me_cache_generation += 1<<22;

                //FIXME precalculate
                {
                    int x, y;
                    memcpy(obmc_edged, obmc_tab[s->block_max_depth], b_w*b_w*4);
                    if(mb_x==0)
                        for(y=0; y<b_w*2; y++)
                            memset(obmc_edged[y], obmc_edged[y][0] + obmc_edged[y][b_w-1], b_w);
                    if(mb_x==b_stride-1)
                        for(y=0; y<b_w*2; y++)
                            memset(obmc_edged[y]+b_w, obmc_edged[y][b_w] + obmc_edged[y][b_w*2-1], b_w);
                    if(mb_y==0){
                        for(x=0; x<b_w*2; x++)
                            obmc_edged[0][x] += obmc_edged[b_w-1][x];
                        for(y=1; y<b_w; y++)
                            memcpy(obmc_edged[y], obmc_edged[0], b_w*2);
                    }
                    if(mb_y==b_height-1){
                        for(x=0; x<b_w*2; x++)
                            obmc_edged[b_w*2-1][x] += obmc_edged[b_w][x];
                        for(y=b_w; y<b_w*2-1; y++)
                            memcpy(obmc_edged[y], obmc_edged[b_w*2-1], b_w*2);
                    }
                }

                //skip stuff outside the picture
                if(mb_x==0 || mb_y==0 || mb_x==b_width-1 || mb_y==b_height-1){
                    uint8_t *src= s->  input_picture.data[0];
                    uint8_t *dst= s->current_picture.data[0];
                    const int stride= s->current_picture.linesize[0];
                    const int block_w= MB_SIZE >> s->block_max_depth;
                    const int sx= block_w*mb_x - block_w/2;
                    const int sy= block_w*mb_y - block_w/2;
                    const int w= s->plane[0].width;
                    const int h= s->plane[0].height;
                    int y;

                    for(y=sy; y<0; y++)
                        memcpy(dst + sx + y*stride, src + sx + y*stride, block_w*2);
                    for(y=h; y<sy+block_w*2; y++)
                        memcpy(dst + sx + y*stride, src + sx + y*stride, block_w*2);
                    if(sx<0){
                        for(y=sy; y<sy+block_w*2; y++)
                            memcpy(dst + sx + y*stride, src + sx + y*stride, -sx);
                    }
                    if(sx+block_w*2 > w){
                        for(y=sy; y<sy+block_w*2; y++)
                            memcpy(dst + w + y*stride, src + w + y*stride, sx+block_w*2 - w);
                    }
                }

                // intra(black) = neighbors' contribution to the current block
                for(i=0; i<3; i++)
                    color[i]= get_dc(s, mb_x, mb_y, i);

                // get previous score (cannot be cached due to OBMC)
                if(pass > 0 && (block->type&BLOCK_INTRA)){
                    int color0[3]= {block->color[0], block->color[1], block->color[2]};
                    check_block(s, mb_x, mb_y, color0, 1, *obmc_edged, &best_rd);
                }else
                    check_block_inter(s, mb_x, mb_y, block->mx, block->my, *obmc_edged, &best_rd);

                ref_b= *block;
                ref_rd= best_rd;
                for(ref=0; ref < s->ref_frames; ref++){
                    int16_t (*mvr)[2]= &s->ref_mvs[ref][index];
                    if(s->ref_scores[ref][index] > s->ref_scores[ref_b.ref][index]*3/2) //FIXME tune threshold
                        continue;
                    block->ref= ref;
                    best_rd= INT_MAX;

                    check_block_inter(s, mb_x, mb_y, mvr[0][0], mvr[0][1], *obmc_edged, &best_rd);
                    check_block_inter(s, mb_x, mb_y, 0, 0, *obmc_edged, &best_rd);
                    if(tb)
                        check_block_inter(s, mb_x, mb_y, mvr[-b_stride][0], mvr[-b_stride][1], *obmc_edged, &best_rd);
                    if(lb)
                        check_block_inter(s, mb_x, mb_y, mvr[-1][0], mvr[-1][1], *obmc_edged, &best_rd);
                    if(rb)
                        check_block_inter(s, mb_x, mb_y, mvr[1][0], mvr[1][1], *obmc_edged, &best_rd);
                    if(bb)
                        check_block_inter(s, mb_x, mb_y, mvr[b_stride][0], mvr[b_stride][1], *obmc_edged, &best_rd);

                    /* fullpel ME */
                    //FIXME avoid subpel interpolation / round to nearest integer
                    do{
                        dia_change=0;
                        for(i=0; i<FFMAX(s->avctx->dia_size, 1); i++){
                            for(j=0; j<i; j++){
                                dia_change |= check_block_inter(s, mb_x, mb_y, block->mx+4*(i-j), block->my+(4*j), *obmc_edged, &best_rd);
                                dia_change |= check_block_inter(s, mb_x, mb_y, block->mx-4*(i-j), block->my-(4*j), *obmc_edged, &best_rd);
                                dia_change |= check_block_inter(s, mb_x, mb_y, block->mx+4*(i-j), block->my-(4*j), *obmc_edged, &best_rd);
                                dia_change |= check_block_inter(s, mb_x, mb_y, block->mx-4*(i-j), block->my+(4*j), *obmc_edged, &best_rd);
                            }
                        }
                    }while(dia_change);
                    /* subpel ME */
                    do{
                        static const int square[8][2]= {{+1, 0},{-1, 0},{ 0,+1},{ 0,-1},{+1,+1},{-1,-1},{+1,-1},{-1,+1},};
                        dia_change=0;
                        for(i=0; i<8; i++)
                            dia_change |= check_block_inter(s, mb_x, mb_y, block->mx+square[i][0], block->my+square[i][1], *obmc_edged, &best_rd);
                    }while(dia_change);
                    //FIXME or try the standard 2 pass qpel or similar

                    mvr[0][0]= block->mx;
                    mvr[0][1]= block->my;
                    if(ref_rd > best_rd){
                        ref_rd= best_rd;
                        ref_b= *block;
                    }
                }
                best_rd= ref_rd;
                *block= ref_b;
                check_block(s, mb_x, mb_y, color, 1, *obmc_edged, &best_rd);
                //FIXME RD style color selection
                if(!same_block(block, &backup)){
                    if(tb ) tb ->type &= ~BLOCK_OPT;
                    if(lb ) lb ->type &= ~BLOCK_OPT;
                    if(rb ) rb ->type &= ~BLOCK_OPT;
                    if(bb ) bb ->type &= ~BLOCK_OPT;
                    if(tlb) tlb->type &= ~BLOCK_OPT;
                    if(trb) trb->type &= ~BLOCK_OPT;
                    if(blb) blb->type &= ~BLOCK_OPT;
                    if(brb) brb->type &= ~BLOCK_OPT;
                    change ++;
                }
            }
        }
        av_log(s->avctx, AV_LOG_ERROR, "pass:%d changed:%d\n", pass, change);
        if(!change)
            break;
    }

    if(s->block_max_depth == 1){
        int change= 0;
        for(mb_y= 0; mb_y<b_height; mb_y+=2){
            for(mb_x= 0; mb_x<b_width; mb_x+=2){
                int i;
                int best_rd, init_rd;
                const int index= mb_x + mb_y * b_stride;
                BlockNode *b[4];

                b[0]= &s->block[index];
                b[1]= b[0]+1;
                b[2]= b[0]+b_stride;
                b[3]= b[2]+1;
                if(same_block(b[0], b[1]) &&
                   same_block(b[0], b[2]) &&
                   same_block(b[0], b[3]))
                    continue;

                if(!s->me_cache_generation)
                    memset(s->me_cache, 0, sizeof(s->me_cache));
                s->me_cache_generation += 1<<22;

                init_rd= best_rd= get_4block_rd(s, mb_x, mb_y, 0);

                //FIXME more multiref search?
                check_4block_inter(s, mb_x, mb_y,
                                   (b[0]->mx + b[1]->mx + b[2]->mx + b[3]->mx + 2) >> 2,
                                   (b[0]->my + b[1]->my + b[2]->my + b[3]->my + 2) >> 2, 0, &best_rd);

                for(i=0; i<4; i++)
                    if(!(b[i]->type&BLOCK_INTRA))
                        check_4block_inter(s, mb_x, mb_y, b[i]->mx, b[i]->my, b[i]->ref, &best_rd);

                if(init_rd != best_rd)
                    change++;
            }
        }
        av_log(s->avctx, AV_LOG_ERROR, "pass:4mv changed:%d\n", change*4);
    }
}

static void encode_blocks(SnowContext *s, int search){
    int x, y;
    int w= s->b_width;
    int h= s->b_height;

    if(s->avctx->me_method == ME_ITER && !s->keyframe && search)
        iterative_me(s);

    for(y=0; y<h; y++){
        if(s->c.bytestream_end - s->c.bytestream < w*MB_SIZE*MB_SIZE*3){ //FIXME nicer limit
            av_log(s->avctx, AV_LOG_ERROR, "encoded frame too large\n");
            return;
        }
        for(x=0; x<w; x++){
            if(s->avctx->me_method == ME_ITER || !search)
                encode_q_branch2(s, 0, x, y);
            else
                encode_q_branch (s, 0, x, y);
        }
    }
}

static void quantize(SnowContext *s, SubBand *b, IDWTELEM *dst, DWTELEM *src, int stride, int bias){
    const int w= b->width;
    const int h= b->height;
    const int qlog= av_clip(s->qlog + b->qlog, 0, QROOT*16);
    const int qmul= qexp[qlog&(QROOT-1)]<<((qlog>>QSHIFT) + ENCODER_EXTRA_BITS);
    int x,y, thres1, thres2;

    if(s->qlog == LOSSLESS_QLOG){
        for(y=0; y<h; y++)
            for(x=0; x<w; x++)
                dst[x + y*stride]= src[x + y*stride];
        return;
    }

    bias= bias ? 0 : (3*qmul)>>3;
    thres1= ((qmul - bias)>>QEXPSHIFT) - 1;
    thres2= 2*thres1;

    if(!bias){
        for(y=0; y<h; y++){
            for(x=0; x<w; x++){
                int i= src[x + y*stride];

                if((unsigned)(i+thres1) > thres2){
                    if(i>=0){
                        i<<= QEXPSHIFT;
                        i/= qmul; //FIXME optimize
                        dst[x + y*stride]=  i;
                    }else{
                        i= -i;
                        i<<= QEXPSHIFT;
                        i/= qmul; //FIXME optimize
                        dst[x + y*stride]= -i;
                    }
                }else
                    dst[x + y*stride]= 0;
            }
        }
    }else{
        for(y=0; y<h; y++){
            for(x=0; x<w; x++){
                int i= src[x + y*stride];

                if((unsigned)(i+thres1) > thres2){
                    if(i>=0){
                        i<<= QEXPSHIFT;
                        i= (i + bias) / qmul; //FIXME optimize
                        dst[x + y*stride]=  i;
                    }else{
                        i= -i;
                        i<<= QEXPSHIFT;
                        i= (i + bias) / qmul; //FIXME optimize
                        dst[x + y*stride]= -i;
                    }
                }else
                    dst[x + y*stride]= 0;
            }
        }
    }
}

static void dequantize(SnowContext *s, SubBand *b, IDWTELEM *src, int stride){
    const int w= b->width;
    const int h= b->height;
    const int qlog= av_clip(s->qlog + b->qlog, 0, QROOT*16);
    const int qmul= qexp[qlog&(QROOT-1)]<<(qlog>>QSHIFT);
    const int qadd= (s->qbias*qmul)>>QBIAS_SHIFT;
    int x,y;

    if(s->qlog == LOSSLESS_QLOG) return;

    for(y=0; y<h; y++){
        for(x=0; x<w; x++){
            int i= src[x + y*stride];
            if(i<0){
                src[x + y*stride]= -((-i*qmul + qadd)>>(QEXPSHIFT)); //FIXME try different bias
            }else if(i>0){
                src[x + y*stride]=  (( i*qmul + qadd)>>(QEXPSHIFT));
            }
        }
    }
}

static void decorrelate(SnowContext *s, SubBand *b, IDWTELEM *src, int stride, int inverse, int use_median){
    const int w= b->width;
    const int h= b->height;
    int x,y;

    for(y=h-1; y>=0; y--){
        for(x=w-1; x>=0; x--){
            int i= x + y*stride;

            if(x){
                if(use_median){
                    if(y && x+1<w) src[i] -= mid_pred(src[i - 1], src[i - stride], src[i - stride + 1]);
                    else  src[i] -= src[i - 1];
                }else{
                    if(y) src[i] -= mid_pred(src[i - 1], src[i - stride], src[i - 1] + src[i - stride] - src[i - 1 - stride]);
                    else  src[i] -= src[i - 1];
                }
            }else{
                if(y) src[i] -= src[i - stride];
            }
        }
    }
}

static void correlate(SnowContext *s, SubBand *b, IDWTELEM *src, int stride, int inverse, int use_median){
    const int w= b->width;
    const int h= b->height;
    int x,y;

    for(y=0; y<h; y++){
        for(x=0; x<w; x++){
            int i= x + y*stride;

            if(x){
                if(use_median){
                    if(y && x+1<w) src[i] += mid_pred(src[i - 1], src[i - stride], src[i - stride + 1]);
                    else  src[i] += src[i - 1];
                }else{
                    if(y) src[i] += mid_pred(src[i - 1], src[i - stride], src[i - 1] + src[i - stride] - src[i - 1 - stride]);
                    else  src[i] += src[i - 1];
                }
            }else{
                if(y) src[i] += src[i - stride];
            }
        }
    }
}

static void encode_qlogs(SnowContext *s){
    int plane_index, level, orientation;

    for(plane_index=0; plane_index<2; plane_index++){
        for(level=0; level<s->spatial_decomposition_count; level++){
            for(orientation=level ? 1:0; orientation<4; orientation++){
                if(orientation==2) continue;
                put_symbol(&s->c, s->header_state, s->plane[plane_index].band[level][orientation].qlog, 1);
            }
        }
    }
}

static void encode_header(SnowContext *s){
    int plane_index, i;
    uint8_t kstate[32];

    memset(kstate, MID_STATE, sizeof(kstate));

    put_rac(&s->c, kstate, s->keyframe);
    if(s->keyframe || s->always_reset){
        ff_snow_reset_contexts(s);
        s->last_spatial_decomposition_type=
        s->last_qlog=
        s->last_qbias=
        s->last_mv_scale=
        s->last_block_max_depth= 0;
        for(plane_index=0; plane_index<2; plane_index++){
            Plane *p= &s->plane[plane_index];
            p->last_htaps=0;
            p->last_diag_mc=0;
            memset(p->last_hcoeff, 0, sizeof(p->last_hcoeff));
        }
    }
    if(s->keyframe){
        put_symbol(&s->c, s->header_state, s->version, 0);
        put_rac(&s->c, s->header_state, s->always_reset);
        put_symbol(&s->c, s->header_state, s->temporal_decomposition_type, 0);
        put_symbol(&s->c, s->header_state, s->temporal_decomposition_count, 0);
        put_symbol(&s->c, s->header_state, s->spatial_decomposition_count, 0);
        put_symbol(&s->c, s->header_state, s->colorspace_type, 0);
        put_symbol(&s->c, s->header_state, s->chroma_h_shift, 0);
        put_symbol(&s->c, s->header_state, s->chroma_v_shift, 0);
        put_rac(&s->c, s->header_state, s->spatial_scalability);
//        put_rac(&s->c, s->header_state, s->rate_scalability);
        put_symbol(&s->c, s->header_state, s->max_ref_frames-1, 0);

        encode_qlogs(s);
    }

    if(!s->keyframe){
        int update_mc=0;
        for(plane_index=0; plane_index<2; plane_index++){
            Plane *p= &s->plane[plane_index];
            update_mc |= p->last_htaps   != p->htaps;
            update_mc |= p->last_diag_mc != p->diag_mc;
            update_mc |= !!memcmp(p->last_hcoeff, p->hcoeff, sizeof(p->hcoeff));
        }
        put_rac(&s->c, s->header_state, update_mc);
        if(update_mc){
            for(plane_index=0; plane_index<2; plane_index++){
                Plane *p= &s->plane[plane_index];
                put_rac(&s->c, s->header_state, p->diag_mc);
                put_symbol(&s->c, s->header_state, p->htaps/2-1, 0);
                for(i= p->htaps/2; i; i--)
                    put_symbol(&s->c, s->header_state, FFABS(p->hcoeff[i]), 0);
            }
        }
        if(s->last_spatial_decomposition_count != s->spatial_decomposition_count){
            put_rac(&s->c, s->header_state, 1);
            put_symbol(&s->c, s->header_state, s->spatial_decomposition_count, 0);
            encode_qlogs(s);
        }else
            put_rac(&s->c, s->header_state, 0);
    }

    put_symbol(&s->c, s->header_state, s->spatial_decomposition_type - s->last_spatial_decomposition_type, 1);
    put_symbol(&s->c, s->header_state, s->qlog            - s->last_qlog    , 1);
    put_symbol(&s->c, s->header_state, s->mv_scale        - s->last_mv_scale, 1);
    put_symbol(&s->c, s->header_state, s->qbias           - s->last_qbias   , 1);
    put_symbol(&s->c, s->header_state, s->block_max_depth - s->last_block_max_depth, 1);

}

static void update_last_header_values(SnowContext *s){
    int plane_index;

    if(!s->keyframe){
        for(plane_index=0; plane_index<2; plane_index++){
            Plane *p= &s->plane[plane_index];
            p->last_diag_mc= p->diag_mc;
            p->last_htaps  = p->htaps;
            memcpy(p->last_hcoeff, p->hcoeff, sizeof(p->hcoeff));
        }
    }

    s->last_spatial_decomposition_type  = s->spatial_decomposition_type;
    s->last_qlog                        = s->qlog;
    s->last_qbias                       = s->qbias;
    s->last_mv_scale                    = s->mv_scale;
    s->last_block_max_depth             = s->block_max_depth;
    s->last_spatial_decomposition_count = s->spatial_decomposition_count;
}

static int qscale2qlog(int qscale){
    return rint(QROOT*log(qscale / (float)FF_QP2LAMBDA)/log(2))
           + 61*QROOT/8; ///< 64 > 60
}

static int ratecontrol_1pass(SnowContext *s, AVFrame *pict)
{
    /* Estimate the frame's complexity as a sum of weighted dwt coefficients.
     * FIXME we know exact mv bits at this point,
     * but ratecontrol isn't set up to include them. */
    uint32_t coef_sum= 0;
    int level, orientation, delta_qlog;

    for(level=0; level<s->spatial_decomposition_count; level++){
        for(orientation=level ? 1 : 0; orientation<4; orientation++){
            SubBand *b= &s->plane[0].band[level][orientation];
            IDWTELEM *buf= b->ibuf;
            const int w= b->width;
            const int h= b->height;
            const int stride= b->stride;
            const int qlog= av_clip(2*QROOT + b->qlog, 0, QROOT*16);
            const int qmul= qexp[qlog&(QROOT-1)]<<(qlog>>QSHIFT);
            const int qdiv= (1<<16)/qmul;
            int x, y;
            //FIXME this is ugly
            for(y=0; y<h; y++)
                for(x=0; x<w; x++)
                    buf[x+y*stride]= b->buf[x+y*stride];
            if(orientation==0)
                decorrelate(s, b, buf, stride, 1, 0);
            for(y=0; y<h; y++)
                for(x=0; x<w; x++)
                    coef_sum+= abs(buf[x+y*stride]) * qdiv >> 16;
        }
    }

    /* ugly, ratecontrol just takes a sqrt again */
    coef_sum = (uint64_t)coef_sum * coef_sum >> 16;
    assert(coef_sum < INT_MAX);

    if(pict->pict_type == AV_PICTURE_TYPE_I){
        s->m.current_picture.mb_var_sum= coef_sum;
        s->m.current_picture.mc_mb_var_sum= 0;
    }else{
        s->m.current_picture.mc_mb_var_sum= coef_sum;
        s->m.current_picture.mb_var_sum= 0;
    }

    pict->quality= ff_rate_estimate_qscale(&s->m, 1);
    if (pict->quality < 0)
        return INT_MIN;
    s->lambda= pict->quality * 3/2;
    delta_qlog= qscale2qlog(pict->quality) - s->qlog;
    s->qlog+= delta_qlog;
    return delta_qlog;
}

static void calculate_visual_weight(SnowContext *s, Plane *p){
    int width = p->width;
    int height= p->height;
    int level, orientation, x, y;

    for(level=0; level<s->spatial_decomposition_count; level++){
        for(orientation=level ? 1 : 0; orientation<4; orientation++){
            SubBand *b= &p->band[level][orientation];
            IDWTELEM *ibuf= b->ibuf;
            int64_t error=0;

            memset(s->spatial_idwt_buffer, 0, sizeof(*s->spatial_idwt_buffer)*width*height);
            ibuf[b->width/2 + b->height/2*b->stride]= 256*16;
            ff_spatial_idwt(s->spatial_idwt_buffer, width, height, width, s->spatial_decomposition_type, s->spatial_decomposition_count);
            for(y=0; y<height; y++){
                for(x=0; x<width; x++){
                    int64_t d= s->spatial_idwt_buffer[x + y*width]*16;
                    error += d*d;
                }
            }

            b->qlog= (int)(log(352256.0/sqrt(error)) / log(pow(2.0, 1.0/QROOT))+0.5);
        }
    }
}

static int encode_frame(AVCodecContext *avctx, unsigned char *buf, int buf_size, void *data){
    SnowContext *s = avctx->priv_data;
    RangeCoder * const c= &s->c;
    AVFrame *pict = data;
    const int width= s->avctx->width;
    const int height= s->avctx->height;
    int level, orientation, plane_index, i, y;
    uint8_t rc_header_bak[sizeof(s->header_state)];
    uint8_t rc_block_bak[sizeof(s->block_state)];

    ff_init_range_encoder(c, buf, buf_size);
    ff_build_rac_states(c, 0.05*(1LL<<32), 256-8);

    for(i=0; i<3; i++){
        int shift= !!i;
        for(y=0; y<(height>>shift); y++)
            memcpy(&s->input_picture.data[i][y * s->input_picture.linesize[i]],
                   &pict->data[i][y * pict->linesize[i]],
                   width>>shift);
    }
    s->new_picture = *pict;

    s->m.picture_number= avctx->frame_number;
    if(avctx->flags&CODEC_FLAG_PASS2){
        s->m.pict_type =
        pict->pict_type= s->m.rc_context.entry[avctx->frame_number].new_pict_type;
        s->keyframe= pict->pict_type==AV_PICTURE_TYPE_I;
        if(!(avctx->flags&CODEC_FLAG_QSCALE)) {
            pict->quality= ff_rate_estimate_qscale(&s->m, 0);
            if (pict->quality < 0)
                return -1;
        }
    }else{
        s->keyframe= avctx->gop_size==0 || avctx->frame_number % avctx->gop_size == 0;
        s->m.pict_type=
        pict->pict_type= s->keyframe ? AV_PICTURE_TYPE_I : AV_PICTURE_TYPE_P;
    }

    if(s->pass1_rc && avctx->frame_number == 0)
        pict->quality= 2*FF_QP2LAMBDA;
    if(pict->quality){
        s->qlog= qscale2qlog(pict->quality);
        s->lambda = pict->quality * 3/2;
    }
    if(s->qlog < 0 || (!pict->quality && (avctx->flags & CODEC_FLAG_QSCALE))){
        s->qlog= LOSSLESS_QLOG;
        s->lambda = 0;
    }//else keep previous frame's qlog until after motion estimation

    ff_snow_frame_start(s);

    s->m.current_picture_ptr= &s->m.current_picture;
    s->m.last_picture.f.pts = s->m.current_picture.f.pts;
    s->m.current_picture.f.pts = pict->pts;
    if(pict->pict_type == AV_PICTURE_TYPE_P){
        int block_width = (width +15)>>4;
        int block_height= (height+15)>>4;
        int stride= s->current_picture.linesize[0];

        assert(s->current_picture.data[0]);
        assert(s->last_picture[0].data[0]);

        s->m.avctx= s->avctx;
        s->m.current_picture.f.data[0] = s->current_picture.data[0];
        s->m.   last_picture.f.data[0] = s->last_picture[0].data[0];
        s->m.    new_picture.f.data[0] = s->  input_picture.data[0];
        s->m.   last_picture_ptr= &s->m.   last_picture;
        s->m.linesize=
        s->m.   last_picture.f.linesize[0] =
        s->m.    new_picture.f.linesize[0] =
        s->m.current_picture.f.linesize[0] = stride;
        s->m.uvlinesize= s->current_picture.linesize[1];
        s->m.width = width;
        s->m.height= height;
        s->m.mb_width = block_width;
        s->m.mb_height= block_height;
        s->m.mb_stride=   s->m.mb_width+1;
        s->m.b8_stride= 2*s->m.mb_width+1;
        s->m.f_code=1;
        s->m.pict_type= pict->pict_type;
        s->m.me_method= s->avctx->me_method;
        s->m.me.scene_change_score=0;
        s->m.flags= s->avctx->flags;
        s->m.quarter_sample= (s->avctx->flags & CODEC_FLAG_QPEL)!=0;
        s->m.out_format= FMT_H263;
        s->m.unrestricted_mv= 1;

        s->m.lambda = s->lambda;
        s->m.qscale= (s->m.lambda*139 + FF_LAMBDA_SCALE*64) >> (FF_LAMBDA_SHIFT + 7);
        s->lambda2= s->m.lambda2= (s->m.lambda*s->m.lambda + FF_LAMBDA_SCALE/2) >> FF_LAMBDA_SHIFT;

        s->m.dsp= s->dsp; //move
        ff_init_me(&s->m);
        s->dsp= s->m.dsp;
    }

    if(s->pass1_rc){
        memcpy(rc_header_bak, s->header_state, sizeof(s->header_state));
        memcpy(rc_block_bak, s->block_state, sizeof(s->block_state));
    }

redo_frame:

    if(pict->pict_type == AV_PICTURE_TYPE_I)
        s->spatial_decomposition_count= 5;
    else
        s->spatial_decomposition_count= 5;

    s->m.pict_type = pict->pict_type;
    s->qbias= pict->pict_type == AV_PICTURE_TYPE_P ? 2 : 0;

    ff_snow_common_init_after_header(avctx);

    if(s->last_spatial_decomposition_count != s->spatial_decomposition_count){
        for(plane_index=0; plane_index<3; plane_index++){
            calculate_visual_weight(s, &s->plane[plane_index]);
        }
    }

    encode_header(s);
    s->m.misc_bits = 8*(s->c.bytestream - s->c.bytestream_start);
    encode_blocks(s, 1);
    s->m.mv_bits = 8*(s->c.bytestream - s->c.bytestream_start) - s->m.misc_bits;

    for(plane_index=0; plane_index<3; plane_index++){
        Plane *p= &s->plane[plane_index];
        int w= p->width;
        int h= p->height;
        int x, y;
//        int bits= put_bits_count(&s->c.pb);

        if (!s->memc_only) {
            //FIXME optimize
            if(pict->data[plane_index]) //FIXME gray hack
                for(y=0; y<h; y++){
                    for(x=0; x<w; x++){
                        s->spatial_idwt_buffer[y*w + x]= pict->data[plane_index][y*pict->linesize[plane_index] + x]<<FRAC_BITS;
                    }
                }
            predict_plane(s, s->spatial_idwt_buffer, plane_index, 0);

            if(   plane_index==0
               && pict->pict_type == AV_PICTURE_TYPE_P
               && !(avctx->flags&CODEC_FLAG_PASS2)
               && s->m.me.scene_change_score > s->avctx->scenechange_threshold){
                ff_init_range_encoder(c, buf, buf_size);
                ff_build_rac_states(c, 0.05*(1LL<<32), 256-8);
                pict->pict_type= AV_PICTURE_TYPE_I;
                s->keyframe=1;
                s->current_picture.key_frame=1;
                goto redo_frame;
            }

            if(s->qlog == LOSSLESS_QLOG){
                for(y=0; y<h; y++){
                    for(x=0; x<w; x++){
                        s->spatial_dwt_buffer[y*w + x]= (s->spatial_idwt_buffer[y*w + x] + (1<<(FRAC_BITS-1))-1)>>FRAC_BITS;
                    }
                }
            }else{
                for(y=0; y<h; y++){
                    for(x=0; x<w; x++){
                        s->spatial_dwt_buffer[y*w + x]=s->spatial_idwt_buffer[y*w + x]<<ENCODER_EXTRA_BITS;
                    }
                }
            }

            /*  if(QUANTIZE2)
                dwt_quantize(s, p, s->spatial_dwt_buffer, w, h, w, s->spatial_decomposition_type);
            else*/
                ff_spatial_dwt(s->spatial_dwt_buffer, w, h, w, s->spatial_decomposition_type, s->spatial_decomposition_count);

            if(s->pass1_rc && plane_index==0){
                int delta_qlog = ratecontrol_1pass(s, pict);
                if (delta_qlog <= INT_MIN)
                    return -1;
                if(delta_qlog){
                    //reordering qlog in the bitstream would eliminate this reset
                    ff_init_range_encoder(c, buf, buf_size);
                    memcpy(s->header_state, rc_header_bak, sizeof(s->header_state));
                    memcpy(s->block_state, rc_block_bak, sizeof(s->block_state));
                    encode_header(s);
                    encode_blocks(s, 0);
                }
            }

            for(level=0; level<s->spatial_decomposition_count; level++){
                for(orientation=level ? 1 : 0; orientation<4; orientation++){
                    SubBand *b= &p->band[level][orientation];

                    if(!QUANTIZE2)
                        quantize(s, b, b->ibuf, b->buf, b->stride, s->qbias);
                    if(orientation==0)
                        decorrelate(s, b, b->ibuf, b->stride, pict->pict_type == AV_PICTURE_TYPE_P, 0);
                    encode_subband(s, b, b->ibuf, b->parent ? b->parent->ibuf : NULL, b->stride, orientation);
                    assert(b->parent==NULL || b->parent->stride == b->stride*2);
                    if(orientation==0)
                        correlate(s, b, b->ibuf, b->stride, 1, 0);
                }
            }

            for(level=0; level<s->spatial_decomposition_count; level++){
                for(orientation=level ? 1 : 0; orientation<4; orientation++){
                    SubBand *b= &p->band[level][orientation];

                    dequantize(s, b, b->ibuf, b->stride);
                }
            }

            ff_spatial_idwt(s->spatial_idwt_buffer, w, h, w, s->spatial_decomposition_type, s->spatial_decomposition_count);
            if(s->qlog == LOSSLESS_QLOG){
                for(y=0; y<h; y++){
                    for(x=0; x<w; x++){
                        s->spatial_idwt_buffer[y*w + x]<<=FRAC_BITS;
                    }
                }
            }
            predict_plane(s, s->spatial_idwt_buffer, plane_index, 1);
        }else{
            //ME/MC only
            if(pict->pict_type == AV_PICTURE_TYPE_I){
                for(y=0; y<h; y++){
                    for(x=0; x<w; x++){
                        s->current_picture.data[plane_index][y*s->current_picture.linesize[plane_index] + x]=
                            pict->data[plane_index][y*pict->linesize[plane_index] + x];
                    }
                }
            }else{
                memset(s->spatial_idwt_buffer, 0, sizeof(IDWTELEM)*w*h);
                predict_plane(s, s->spatial_idwt_buffer, plane_index, 1);
            }
        }
        if(s->avctx->flags&CODEC_FLAG_PSNR){
            int64_t error= 0;

            if(pict->data[plane_index]) //FIXME gray hack
                for(y=0; y<h; y++){
                    for(x=0; x<w; x++){
                        int d= s->current_picture.data[plane_index][y*s->current_picture.linesize[plane_index] + x] - pict->data[plane_index][y*pict->linesize[plane_index] + x];
                        error += d*d;
                    }
                }
            s->avctx->error[plane_index] += error;
            s->current_picture.error[plane_index] = error;
        }

    }

    update_last_header_values(s);

    ff_snow_release_buffer(avctx);

    s->current_picture.coded_picture_number = avctx->frame_number;
    s->current_picture.pict_type = pict->pict_type;
    s->current_picture.quality = pict->quality;
    s->m.frame_bits = 8*(s->c.bytestream - s->c.bytestream_start);
    s->m.p_tex_bits = s->m.frame_bits - s->m.misc_bits - s->m.mv_bits;
    s->m.current_picture.f.display_picture_number =
    s->m.current_picture.f.coded_picture_number   = avctx->frame_number;
    s->m.current_picture.f.quality                = pict->quality;
    s->m.total_bits += 8*(s->c.bytestream - s->c.bytestream_start);
    if(s->pass1_rc)
        if (ff_rate_estimate_qscale(&s->m, 0) < 0)
            return -1;
    if(avctx->flags&CODEC_FLAG_PASS1)
        ff_write_pass1_stats(&s->m);
    s->m.last_pict_type = s->m.pict_type;
    avctx->frame_bits = s->m.frame_bits;
    avctx->mv_bits = s->m.mv_bits;
    avctx->misc_bits = s->m.misc_bits;
    avctx->p_tex_bits = s->m.p_tex_bits;

    emms_c();

    return ff_rac_terminate(c);
}

static av_cold int encode_end(AVCodecContext *avctx)
{
    SnowContext *s = avctx->priv_data;

    ff_snow_common_end(s);
    if (s->input_picture.data[0])
        avctx->release_buffer(avctx, &s->input_picture);
    av_free(avctx->stats_out);

    return 0;
}

#define OFFSET(x) offsetof(SnowContext, x)
#define VE AV_OPT_FLAG_VIDEO_PARAM | AV_OPT_FLAG_ENCODING_PARAM
static const AVOption options[] = {
    { "memc_only",      "Only do ME/MC (I frames -> ref, P frame -> ME+MC).",   OFFSET(memc_only), AV_OPT_TYPE_INT, { 0 }, 0, 1, VE },
    { NULL },
};

static const AVClass snowenc_class = {
    .class_name = "snow encoder",
    .item_name  = av_default_item_name,
    .option     = options,
    .version    = LIBAVUTIL_VERSION_INT,
};

AVCodec ff_snow_encoder = {
    .name           = "snow",
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = CODEC_ID_SNOW,
    .priv_data_size = sizeof(SnowContext),
    .init           = encode_init,
    .encode         = encode_frame,
    .close          = encode_end,
    .long_name = NULL_IF_CONFIG_SMALL("Snow"),
    .priv_class     = &snowenc_class,
};
#endif


#ifdef TEST
#undef malloc
#undef free
#undef printf

#include "libavutil/lfg.h"
#include "libavutil/mathematics.h"

int main(void){
    int width=256;
    int height=256;
    int buffer[2][width*height];
    SnowContext s;
    int i;
    AVLFG prng;
    s.spatial_decomposition_count=6;
    s.spatial_decomposition_type=1;

    av_lfg_init(&prng, 1);

    printf("testing 5/3 DWT\n");
    for(i=0; i<width*height; i++)
        buffer[0][i] = buffer[1][i] = av_lfg_get(&prng) % 54321 - 12345;

    ff_spatial_dwt(buffer[0], width, height, width, s.spatial_decomposition_type, s.spatial_decomposition_count);
    ff_spatial_idwt(buffer[0], width, height, width, s.spatial_decomposition_type, s.spatial_decomposition_count);

    for(i=0; i<width*height; i++)
        if(buffer[0][i]!= buffer[1][i]) printf("fsck: %6d %12d %7d\n",i, buffer[0][i], buffer[1][i]);

    printf("testing 9/7 DWT\n");
    s.spatial_decomposition_type=0;
    for(i=0; i<width*height; i++)
        buffer[0][i] = buffer[1][i] = av_lfg_get(&prng) % 54321 - 12345;

    ff_spatial_dwt(buffer[0], width, height, width, s.spatial_decomposition_type, s.spatial_decomposition_count);
    ff_spatial_idwt(buffer[0], width, height, width, s.spatial_decomposition_type, s.spatial_decomposition_count);

    for(i=0; i<width*height; i++)
        if(FFABS(buffer[0][i] - buffer[1][i])>20) printf("fsck: %6d %12d %7d\n",i, buffer[0][i], buffer[1][i]);

    {
    int level, orientation, x, y;
    int64_t errors[8][4];
    int64_t g=0;

        memset(errors, 0, sizeof(errors));
        s.spatial_decomposition_count=3;
        s.spatial_decomposition_type=0;
        for(level=0; level<s.spatial_decomposition_count; level++){
            for(orientation=level ? 1 : 0; orientation<4; orientation++){
                int w= width  >> (s.spatial_decomposition_count-level);
                int h= height >> (s.spatial_decomposition_count-level);
                int stride= width  << (s.spatial_decomposition_count-level);
                DWTELEM *buf= buffer[0];
                int64_t error=0;

                if(orientation&1) buf+=w;
                if(orientation>1) buf+=stride>>1;

                memset(buffer[0], 0, sizeof(int)*width*height);
                buf[w/2 + h/2*stride]= 256*256;
                ff_spatial_idwt(buffer[0], width, height, width, s.spatial_decomposition_type, s.spatial_decomposition_count);
                for(y=0; y<height; y++){
                    for(x=0; x<width; x++){
                        int64_t d= buffer[0][x + y*width];
                        error += d*d;
                        if(FFABS(width/2-x)<9 && FFABS(height/2-y)<9 && level==2) printf("%8"PRId64" ", d);
                    }
                    if(FFABS(height/2-y)<9 && level==2) printf("\n");
                }
                error= (int)(sqrt(error)+0.5);
                errors[level][orientation]= error;
                if(g) g=av_gcd(g, error);
                else g= error;
            }
        }
        printf("static int const visual_weight[][4]={\n");
        for(level=0; level<s.spatial_decomposition_count; level++){
            printf("  {");
            for(orientation=0; orientation<4; orientation++){
                printf("%8"PRId64",", errors[level][orientation]/g);
            }
            printf("},\n");
        }
        printf("};\n");
        {
            int level=2;
            int w= width  >> (s.spatial_decomposition_count-level);
            //int h= height >> (s.spatial_decomposition_count-level);
            int stride= width  << (s.spatial_decomposition_count-level);
            DWTELEM *buf= buffer[0];
            int64_t error=0;

            buf+=w;
            buf+=stride>>1;

            memset(buffer[0], 0, sizeof(int)*width*height);
            for(y=0; y<height; y++){
                for(x=0; x<width; x++){
                    int tab[4]={0,2,3,1};
                    buffer[0][x+width*y]= 256*256*tab[(x&1) + 2*(y&1)];
                }
            }
            ff_spatial_dwt(buffer[0], width, height, width, s.spatial_decomposition_type, s.spatial_decomposition_count);
            for(y=0; y<height; y++){
                for(x=0; x<width; x++){
                    int64_t d= buffer[0][x + y*width];
                    error += d*d;
                    if(FFABS(width/2-x)<9 && FFABS(height/2-y)<9) printf("%8"PRId64" ", d);
                }
                if(FFABS(height/2-y)<9) printf("\n");
            }
        }

    }
    return 0;
}
#endif /* TEST */
