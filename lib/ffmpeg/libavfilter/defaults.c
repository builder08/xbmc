/*
 * Filter layer - default implementations
 * Copyright (c) 2007 Bobby Bingham
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

#include "libavutil/avassert.h"
#include "libavutil/audioconvert.h"
#include "libavutil/imgutils.h"
#include "libavutil/samplefmt.h"
#include "avfilter.h"
#include "internal.h"

void ff_avfilter_default_free_buffer(AVFilterBuffer *ptr)
{
    av_free(ptr->data[0]);
    av_free(ptr);
}

/* TODO: set the buffer's priv member to a context structure for the whole
 * filter chain.  This will allow for a buffer pool instead of the constant
 * alloc & free cycle currently implemented. */
AVFilterBufferRef *avfilter_default_get_video_buffer(AVFilterLink *link, int perms, int w, int h)
{
    int linesize[4];
    uint8_t *data[4];
    int i;
    AVFilterBufferRef *picref = NULL;
    AVFilterPool *pool = link->pool;

    if (pool) {
        for (i = 0; i < POOL_SIZE; i++) {
            picref = pool->pic[i];
            if (picref && picref->buf->format == link->format && picref->buf->w == w && picref->buf->h == h) {
                AVFilterBuffer *pic = picref->buf;
                pool->pic[i] = NULL;
                pool->count--;
                picref->video->w = w;
                picref->video->h = h;
                picref->perms = perms | AV_PERM_READ;
                picref->format = link->format;
                pic->refcount = 1;
                memcpy(picref->data,     pic->data,     sizeof(picref->data));
                memcpy(picref->linesize, pic->linesize, sizeof(picref->linesize));
                pool->refcount++;
                return picref;
            }
        }
    } else {
        pool = link->pool = av_mallocz(sizeof(AVFilterPool));
        pool->refcount = 1;
    }

    // align: +2 is needed for swscaler, +16 to be SIMD-friendly
    if ((i = av_image_alloc(data, linesize, w, h, link->format, 32)) < 0)
        return NULL;

    picref = avfilter_get_video_buffer_ref_from_arrays(data, linesize,
                                                       perms, w, h, link->format);
    if (!picref) {
        av_free(data[0]);
        return NULL;
    }
    memset(data[0], 128, i);

    picref->buf->priv = pool;
    picref->buf->free = NULL;
    pool->refcount++;

    return picref;
}

AVFilterBufferRef *avfilter_default_get_audio_buffer(AVFilterLink *link, int perms,
                                                     int nb_samples)
{
    AVFilterBufferRef *samplesref = NULL;
    int linesize[8] = {0};
    uint8_t *data[8] = {0};
    int ch, nb_channels = av_get_channel_layout_nb_channels(link->channel_layout);

    /* right now we don't support more than 8 channels */
    av_assert0(nb_channels <= 8);

    /* Calculate total buffer size, round to multiple of 16 to be SIMD friendly */
    if (av_samples_alloc(data, linesize,
                         nb_channels, nb_samples,
                         av_get_alt_sample_fmt(link->format, link->planar),
                         16) < 0)
        return NULL;

    for (ch = 1; link->planar && ch < nb_channels; ch++)
        linesize[ch] = linesize[0];
    samplesref =
        avfilter_get_audio_buffer_ref_from_arrays(data, linesize, perms,
                                                  nb_samples, link->format,
                                                  link->channel_layout, link->planar);
    if (!samplesref) {
        av_free(data[0]);
        return NULL;
    }

    return samplesref;
}

void avfilter_default_start_frame(AVFilterLink *inlink, AVFilterBufferRef *picref)
{
    AVFilterLink *outlink = NULL;

    if (inlink->dst->output_count)
        outlink = inlink->dst->outputs[0];

    if (outlink) {
        outlink->out_buf = avfilter_get_video_buffer(outlink, AV_PERM_WRITE, outlink->w, outlink->h);
        avfilter_copy_buffer_ref_props(outlink->out_buf, picref);
        avfilter_start_frame(outlink, avfilter_ref_buffer(outlink->out_buf, ~0));
    }
}

void avfilter_default_draw_slice(AVFilterLink *inlink, int y, int h, int slice_dir)
{
    AVFilterLink *outlink = NULL;

    if (inlink->dst->output_count)
        outlink = inlink->dst->outputs[0];

    if (outlink)
        avfilter_draw_slice(outlink, y, h, slice_dir);
}

void avfilter_default_end_frame(AVFilterLink *inlink)
{
    AVFilterLink *outlink = NULL;

    if (inlink->dst->output_count)
        outlink = inlink->dst->outputs[0];

    avfilter_unref_buffer(inlink->cur_buf);
    inlink->cur_buf = NULL;

    if (outlink) {
        if (outlink->out_buf) {
            avfilter_unref_buffer(outlink->out_buf);
            outlink->out_buf = NULL;
        }
        avfilter_end_frame(outlink);
    }
}

/* FIXME: samplesref is same as link->cur_buf. Need to consider removing the redundant parameter. */
void avfilter_default_filter_samples(AVFilterLink *inlink, AVFilterBufferRef *samplesref)
{
    AVFilterLink *outlink = NULL;

    if (inlink->dst->output_count)
        outlink = inlink->dst->outputs[0];

    if (outlink) {
        outlink->out_buf = avfilter_default_get_audio_buffer(inlink, AV_PERM_WRITE,
                                                             samplesref->audio->nb_samples);
        outlink->out_buf->pts                = samplesref->pts;
        outlink->out_buf->audio->sample_rate = samplesref->audio->sample_rate;
        avfilter_filter_samples(outlink, avfilter_ref_buffer(outlink->out_buf, ~0));
        avfilter_unref_buffer(outlink->out_buf);
        outlink->out_buf = NULL;
    }
    avfilter_unref_buffer(samplesref);
    inlink->cur_buf = NULL;
}

static void set_common_formats(AVFilterContext *ctx, AVFilterFormats *fmts,
                               enum AVMediaType type, int offin, int offout)
{
    int i;
    for (i = 0; i < ctx->input_count; i++)
        if (ctx->inputs[i] && ctx->inputs[i]->type == type)
            avfilter_formats_ref(fmts,
                                 (AVFilterFormats **)((uint8_t *)ctx->inputs[i]+offout));

    for (i = 0; i < ctx->output_count; i++)
        if (ctx->outputs[i] && ctx->outputs[i]->type == type)
            avfilter_formats_ref(fmts,
                                 (AVFilterFormats **)((uint8_t *)ctx->outputs[i]+offin));

    if (!fmts->refcount) {
        av_free(fmts->formats);
        av_free(fmts->refs);
        av_free(fmts);
    }
}

void avfilter_set_common_pixel_formats(AVFilterContext *ctx, AVFilterFormats *formats)
{
    set_common_formats(ctx, formats, AVMEDIA_TYPE_VIDEO,
                       offsetof(AVFilterLink, in_formats),
                       offsetof(AVFilterLink, out_formats));
}

void avfilter_set_common_sample_formats(AVFilterContext *ctx, AVFilterFormats *formats)
{
    set_common_formats(ctx, formats, AVMEDIA_TYPE_AUDIO,
                       offsetof(AVFilterLink, in_formats),
                       offsetof(AVFilterLink, out_formats));
}

void avfilter_set_common_channel_layouts(AVFilterContext *ctx, AVFilterFormats *formats)
{
    set_common_formats(ctx, formats, AVMEDIA_TYPE_AUDIO,
                       offsetof(AVFilterLink, in_chlayouts),
                       offsetof(AVFilterLink, out_chlayouts));
}

void avfilter_set_common_packing_formats(AVFilterContext *ctx, AVFilterFormats *formats)
{
    set_common_formats(ctx, formats, AVMEDIA_TYPE_AUDIO,
                       offsetof(AVFilterLink, in_packing),
                       offsetof(AVFilterLink, out_packing));
}

int avfilter_default_query_formats(AVFilterContext *ctx)
{
    avfilter_set_common_pixel_formats(ctx, avfilter_make_all_formats(AVMEDIA_TYPE_VIDEO));
    avfilter_set_common_sample_formats(ctx, avfilter_make_all_formats(AVMEDIA_TYPE_AUDIO));
    avfilter_set_common_channel_layouts(ctx, avfilter_make_all_channel_layouts());
    avfilter_set_common_packing_formats(ctx, avfilter_make_all_packing_formats());

    return 0;
}

void avfilter_null_start_frame(AVFilterLink *link, AVFilterBufferRef *picref)
{
    avfilter_start_frame(link->dst->outputs[0], picref);
}

void avfilter_null_draw_slice(AVFilterLink *link, int y, int h, int slice_dir)
{
    avfilter_draw_slice(link->dst->outputs[0], y, h, slice_dir);
}

void avfilter_null_end_frame(AVFilterLink *link)
{
    avfilter_end_frame(link->dst->outputs[0]);
}

void avfilter_null_filter_samples(AVFilterLink *link, AVFilterBufferRef *samplesref)
{
    avfilter_filter_samples(link->dst->outputs[0], samplesref);
}

AVFilterBufferRef *avfilter_null_get_video_buffer(AVFilterLink *link, int perms, int w, int h)
{
    return avfilter_get_video_buffer(link->dst->outputs[0], perms, w, h);
}

AVFilterBufferRef *avfilter_null_get_audio_buffer(AVFilterLink *link, int perms,
                                                  int nb_samples)
{
    return avfilter_get_audio_buffer(link->dst->outputs[0], perms, nb_samples);
}

