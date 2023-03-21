/*
 * Copyright (c) 2023 Wang Wei(wangwei1237@gmail.com)
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

#include <math.h>

#include "libavutil/imgutils.h"
#include "libavutil/internal.h"
#include "libavutil/opt.h"
#include "libswscale/swscale.h"

#include "avfilter.h"
#include "formats.h"
#include "internal.h"
#include "video.h"

typedef struct MSContext {
    const AVClass *class;
    int width, height;
    int chromah;    // height of chroma plane
    int chromaw;    // width of chroma plane
    int hsub;       // horizontal subsampling
    int vsub;       // vertical subsampling
    uint64_t nb_frames;
    int ms;
    int *dst;
} MSContext;

typedef struct ThreadDataMS {
    const AVFrame *src;
    int *dst;
} ThreadDataMS;

static const enum AVPixelFormat pix_fmts[] = {
    AV_PIX_FMT_YUV420P,   AV_PIX_FMT_YUV422P,
    AV_PIX_FMT_YUVJ420P,  AV_PIX_FMT_YUVJ422P,
    AV_PIX_FMT_YUV420P10, AV_PIX_FMT_YUV422P10,
    AV_PIX_FMT_NONE
};

static av_cold int init(AVFilterContext *ctx)
{
    // User options but no input data
    MSContext *s = ctx->priv;

    return 0;
}

static av_cold void uninit(AVFilterContext *ctx)
{
    MSContext *s = ctx->priv;

    av_freep(&s->dst);
}

static int config_input(AVFilterLink *inlink)
{
    // Video input data avilable
    AVFilterContext *ctx = inlink->dst;
    MSContext *s = ctx->priv;
    const AVPixFmtDescriptor *desc = av_pix_fmt_desc_get(inlink->format);
    s->hsub = desc->log2_chroma_w;
    s->vsub = desc->log2_chroma_h;
    // free previous buffers in case they are allocated already
    s->width  = inlink->w;
    s->height = inlink->h;
    s->chromaw = AV_CEIL_RSHIFT(inlink->w, s->hsub);
    s->chromah = AV_CEIL_RSHIFT(inlink->h, s->vsub);
    s->dst = av_malloc_array(s->chromah*s->chromaw, sizeof(*s->dst));
    if (!s->dst) {
        return AVERROR(ENOMEM);
    }

    return 0;
}

static int calc(MSContext *ctx, AVFrame *frame) {
    int i, j;
    const int lsz_u = frame->linesize[1];
    const int lsz_v = frame->linesize[2];
    const int lsz_sat = ctx->chromaw;
    int *dst = ctx->dst;

    for (j = 0; j < ctx->chromah; j++) {
        const uint8_t *p_u = frame->data[1] + j * lsz_u;
        const uint8_t *p_v = frame->data[2] + j * lsz_v;
        uint8_t *p_sat = dst + j * lsz_sat;

        for (i = 0; i < ctx->chromaw; i++) {
            const int yuvu = p_u[i];
            const int yuvv = p_v[i];
            
            p_sat[i] = hypotf(yuvu - 128, yuvv - 128);
        }
    }

    return 0;
}

static int calc_ms(AVFilterContext *ctx, void *arg, int jobnr, int nb_jobs) {
    int i, j;
    ThreadDataMS *td = arg;
    const MSContext *s = ctx->priv;
    const AVFrame *src = td->src;
    int *dst = td->dst;

    const int slice_start = (s->chromah *  jobnr   ) / nb_jobs;
    const int slice_end   = (s->chromah * (jobnr+1)) / nb_jobs;

    const int lsz_u = src->linesize[1];
    const int lsz_v = src->linesize[2];
    const uint8_t *p_u = src->data[1] + slice_start * lsz_u;
    const uint8_t *p_v = src->data[2] + slice_start * lsz_v;

    const int lsz_sat = s->chromaw;
    uint8_t *p_sat = dst + slice_start * lsz_sat;

    for (j = slice_start; j < slice_end; j++) {
        for (i = 0; i < s->chromaw; i++) {
            const int yuvu = p_u[i];
            const int yuvv = p_v[i];
            p_sat[i] = hypotf(yuvu - 128, yuvv - 128);
        }
        p_u   += lsz_u;
        p_v   += lsz_v;
        p_sat += lsz_sat;
    }

    return 0;
}

static int filter_frame(AVFilterLink *inlink, AVFrame *frame)
{
    AVFilterContext *ctx = inlink->dst;
    MSContext *s = ctx->priv;
    ThreadDataMS td_ms = {
        .src = frame,
        .dst = s->dst
    };

    s->nb_frames++;

    if (!s->ms) {
        calc(s, frame);
    } else {
        ff_filter_execute(ctx, calc_ms, &td_ms,
                          NULL, FFMIN(s->chromah, ff_filter_get_nb_threads(ctx)));
        // av_log(ctx, AV_LOG_INFO, "chromah: %d, %d \n", s->chromah, s->chromaw);
    }

    return ff_filter_frame(inlink->dst->outputs[0], frame);
}

#define OFFSET(x) offsetof(MSContext, x)
#define FLAGS AV_OPT_FLAG_VIDEO_PARAM|AV_OPT_FLAG_FILTERING_PARAM

static const AVOption ms_options[] = {
    { "ms", "Multithreading or not", OFFSET(ms), AV_OPT_TYPE_BOOL, { .i64=0 }, 0, 1, FLAGS},
    { NULL }
};

AVFILTER_DEFINE_CLASS(ms);

static const AVFilterPad avfilter_vf_ms_inputs[] = {
    {
        .name         = "default",
        .type         = AVMEDIA_TYPE_VIDEO,
        .config_props = config_input,
        .filter_frame = filter_frame,
    },
};

static const AVFilterPad avfilter_vf_ms_outputs[] = {
    {
        .name = "default",
        .type = AVMEDIA_TYPE_VIDEO
    },
};

const AVFilter ff_vf_ms = {
    .name          = "ms",
    .description   = NULL_IF_CONFIG_SMALL("Test for the multithreading filter."),
    .priv_size     = sizeof(MSContext),
    .priv_class    = &ms_class,
    .init          = init,
    .uninit        = uninit,
    .flags         = AVFILTER_FLAG_SLICE_THREADS,
    FILTER_PIXFMTS_ARRAY(pix_fmts),
    FILTER_INPUTS(avfilter_vf_ms_inputs),
    FILTER_OUTPUTS(avfilter_vf_ms_outputs),
};
