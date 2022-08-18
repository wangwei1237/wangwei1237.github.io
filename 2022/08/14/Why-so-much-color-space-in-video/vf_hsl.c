/*
 * Copyright (c) 2022 Wang Wei(wangwei1237@gmail.com)
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

/**
 * @file
 * Calculate the HSL color space information.
 * HSL is a color space that is more perceptually uniform than RGB :
 *     https://en.wikipedia.org/wiki/HSL_and_HSV
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

typedef float num;

typedef struct HSL_COLOR {
    num H;
    num S;
    num L;
} HSL_COLOR;

typedef struct HSLContext {
    const AVClass *class;
    int width, height;
    uint64_t nb_frames;
    float max_hue, max_sat, max_light;
    float min_hue, min_sat, min_light;
    float sum_hue, sum_sat, sum_light;
    int print_summary;
    AVFrame *rgbFrame;
    enum AVPixelFormat rgb_format;
} HSLContext;

static const enum AVPixelFormat pix_fmts[] = {
    AV_PIX_FMT_YUV420P,   AV_PIX_FMT_YUV422P,
    AV_PIX_FMT_YUVJ420P,  AV_PIX_FMT_YUVJ422P,
    AV_PIX_FMT_YUV420P10, AV_PIX_FMT_YUV422P10,
    AV_PIX_FMT_NONE
};

static av_cold int init(AVFilterContext *ctx)
{
    // User options but no input data
    HSLContext *s = ctx->priv;
    s->max_hue    = 0;
    s->max_sat    = 0;
    s->max_light  = 0;

    return 0;
}

static av_cold void uninit(AVFilterContext *ctx)
{
    HSLContext *s = ctx->priv;

    if (s->print_summary) {
        float avg_hue   = s->sum_hue   / s->nb_frames;
        float avg_sat   = s->sum_sat   / s->nb_frames;
        float avg_light = s->sum_light / s->nb_frames;
        av_log(ctx, AV_LOG_INFO,
               "HSL Summary:\nTotal frames: %"PRId64"\n\n"
               "Hue Information:\nAverage: %.1f\nMax: %.1f\nMin: %.1f\n\n"
               "Saturation Information:\nAverage: %.1f\nMax: %.1f\nMin: %.1f\n\n"
               "Lightness Information:\nAverage: %.1f\nMax: %.1f\nMin: %.1f\n\n",
               s->nb_frames, 
               avg_hue,   s->max_hue,   s->min_hue,
               avg_sat   * 100, s->max_sat   * 100, s->min_sat   * 100,
               avg_light * 100, s->max_light * 100, s->min_light * 100
        );
    }

    av_frame_free(&s->rgbFrame);
}

static int config_input(AVFilterLink *inlink)
{
    // Video input data avilable
    AVFilterContext *ctx = inlink->dst;
    HSLContext *s = ctx->priv;
    
    // free previous buffers in case they are allocated already
    av_frame_free(&s->rgbFrame);
    s->width  = inlink->w;
    s->height = inlink->h;
    s->rgb_format = AV_PIX_FMT_RGB24;
    s->rgbFrame = av_frame_alloc();
    if (!s->rgbFrame) {
        return AVERROR(ENOMEM);
    }

    s->rgbFrame->format = s->rgb_format;
    s->rgbFrame->width  = s->width;
    s->rgbFrame->height = s->height;
    if (av_frame_get_buffer(s->rgbFrame, 0)) {
        return AVERROR(ENOMEM);
    }

    return 0;
}

static void set_meta(AVDictionary **metadata, const char *key, float d)
{
    char value[128];
    snprintf(value, sizeof(value), "%0.1f", d);
    av_dict_set(metadata, key, value, 0);
}

static void YUV2RGB(const AVFrame* src, enum AVPixelFormat dstFormat, AVFrame* dst) 
{
    int width  = src->width;
    int height = src->height;
    
    struct SwsContext* conversion = NULL;
    conversion = sws_getContext(width,
                                height,
                                (enum AVPixelFormat)src->format,
                                width,
                                height,
                                dstFormat,
                                SWS_FAST_BILINEAR,
                                NULL,
                                NULL,
                                NULL);
    sws_scale(conversion, src->data, src->linesize, 0, height, dst->data, dst->linesize);
    sws_freeContext(conversion);
}

static const float EPSILON = 1e-9;

/** @brief Equal of A and B */
#define EQ(A,B)    ((fabs((A) - (B)) < EPSILON) ? 1 : 0)


/** 
 * @brief Convert an sRGB color to Hue-Saturation-Lightness (HSL)
 * 
 * @param H, S, L pointers to hold the result
 * @param R, G, B the input sRGB values scaled in [0,1]
 *
 * This routine transforms from sRGB to the double hexcone HSL color space
 * The sRGB values are assumed to be between 0 and 1.  The outputs are
 *   H = hexagonal hue angle                (0 <= H < 360),
 *   S = { C/(2L)     if L <= 1/2           (0 <= S <= 1),
 *       { C/(2 - 2L) if L >  1/2
 *   L = (max(R',G',B') + min(R',G',B'))/2  (0 <= L <= 1),
 * where C = max(R',G',B') - min(R',G',B').
 *
 * Wikipedia: http://en.wikipedia.org/wiki/HSL_and_HSV
 */
static void RGB2HSL(num *H, num *S, num *L, num R, num G, num B)
{
    num Max = FFMAX3(R, G, B);
    num Min = FFMIN3(R, G, B);
    num C = Max - Min;

    *L = (Max + Min) / 2;
    
    if (C > 0) {
        if (EQ(Max, R)) {
            *H = (G - B) / C;
            
            if (G < B) {
                *H += 6;
            }
        } else if (EQ(Max, G)) {
            *H = 2 + (B - R) / C;
        } else {
            *H = 4 + (R - G) / C;
        }

        *H *= 60;
        *S = (*L <= 0.5) ? (C/(2*(*L))) : (C/(2 - 2*(*L)));
    } else {
        *H = *S = 0;
    }
}

static void calcHSL(HSLContext *ctx, AVFrame *frame, HSL_COLOR *hsl)
{
    // Convert to RGB
    YUV2RGB(frame, ctx->rgb_format, ctx->rgbFrame);
    
    int cnt = frame->width * frame->height;
    num R = 0, G = 0, B = 0;
    num H = 0, S = 0, L = 0;
    num H_sum = 0, S_sum = 0, L_sum = 0;
    
    for (int i = 0; i < cnt; i++) {
        R = ctx->rgbFrame->data[0][3 * i];
        G = ctx->rgbFrame->data[0][3 * i + 1];
        B = ctx->rgbFrame->data[0][3 * i + 2];
        RGB2HSL(&H, &S, &L, R/255, G/255, B/255);
        H_sum += H;
        S_sum += S;
        L_sum += L;
    }

    hsl->H = H_sum / cnt;
    hsl->S = S_sum / cnt;
    hsl->L = L_sum / cnt;
    
    // av_log(ctx, AV_LOG_INFO, "H: %.1f, S: %.1f, L: %.1f\n",
    //        hsl->H, hsl->S * 100, hsl->L * 100);
}

static int filter_frame(AVFilterLink *inlink, AVFrame *frame)
{
    AVFilterContext *ctx = inlink->dst;
    HSLContext *s = ctx->priv;
    s->nb_frames++;

    HSL_COLOR hsl;
    calcHSL(s, frame, &hsl);

    // Calculate statistics
    s->max_hue    = fmaxf(hsl.H, s->max_hue);
    s->max_sat    = fmaxf(hsl.S, s->max_sat);
    s->max_light  = fmaxf(hsl.L, s->max_light);
    s->sum_hue   += hsl.H;
    s->sum_sat   += hsl.S;
    s->sum_light += hsl.L;
    s->min_hue    = s->nb_frames == 1 ? hsl.H : fminf(hsl.H, s->min_hue);
    s->min_sat    = s->nb_frames == 1 ? hsl.S : fminf(hsl.S, s->min_sat);
    s->min_light  = s->nb_frames == 1 ? hsl.L : fminf(hsl.L, s->min_light);

    // Set HSL information in frame metadata
    set_meta(&frame->metadata, "lavfi.hsl.hue",   hsl.H);
    set_meta(&frame->metadata, "lavfi.hsl.sat",   hsl.S * 100);
    set_meta(&frame->metadata, "lavfi.hsl.light", hsl.L * 100);

    return ff_filter_frame(inlink->dst->outputs[0], frame);
}

#define OFFSET(x) offsetof(HSLContext, x)
#define FLAGS AV_OPT_FLAG_VIDEO_PARAM|AV_OPT_FLAG_FILTERING_PARAM

static const AVOption hsl_options[] = {
    { "print_summary", "Print summary showing average values", OFFSET(print_summary), AV_OPT_TYPE_BOOL, { .i64=0 }, 0, 1, FLAGS },
    { NULL }
};

AVFILTER_DEFINE_CLASS(hsl);

static const AVFilterPad avfilter_vf_hsl_inputs[] = {
    {
        .name         = "default",
        .type         = AVMEDIA_TYPE_VIDEO,
        .config_props = config_input,
        .filter_frame = filter_frame,
    },
};

static const AVFilterPad avfilter_vf_hsl_outputs[] = {
    {
        .name = "default",
        .type = AVMEDIA_TYPE_VIDEO
    },
};

const AVFilter ff_vf_hsl = {
    .name          = "hsl",
    .description   = NULL_IF_CONFIG_SMALL("Calculate the HSL color space information."),
    .priv_size     = sizeof(HSLContext),
    .priv_class    = &hsl_class,
    .init          = init,
    .uninit        = uninit,
    .flags         = AVFILTER_FLAG_METADATA_ONLY,
    FILTER_PIXFMTS_ARRAY(pix_fmts),
    FILTER_INPUTS(avfilter_vf_hsl_inputs),
    FILTER_OUTPUTS(avfilter_vf_hsl_outputs),
};