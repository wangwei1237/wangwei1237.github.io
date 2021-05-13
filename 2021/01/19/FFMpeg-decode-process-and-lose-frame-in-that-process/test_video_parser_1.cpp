#include <string>
#include <iostream>

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavdevice/avdevice.h"
#include "libavformat/avformat.h"
#include "libavutil/imgutils.h"
}

/**
 * Refer to ffprobe.c
 */
int process_frame(AVFormatContext *fmt_ctx, 
                  AVCodecContext *dec_ctx, 
                  AVCodecParameters *par, 
                  AVFrame *frame, 
                  AVPacket *pkt, 
                  int *packet_new) {
    int ret = 0, got_frame = 0;

    if (dec_ctx && dec_ctx->codec) {
        switch (par->codec_type) {
        case AVMEDIA_TYPE_VIDEO:
        case AVMEDIA_TYPE_AUDIO:
            if (*packet_new) {
                ret = avcodec_send_packet(dec_ctx, pkt);
                if (ret == AVERROR(EAGAIN)) {
                    ret = 0;
                } else if (ret >= 0 || ret == AVERROR_EOF) {
                    ret = 0;
                    *packet_new = 0;
                }
            }
            if (ret >= 0) {
                ret = avcodec_receive_frame(dec_ctx, frame);
                if (ret >= 0) {
                    got_frame = 1;
                } else if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                    ret = 0;
                }
            }
            break;

        case AVMEDIA_TYPE_SUBTITLE:
            *packet_new = 0;
            break;
        default:
            *packet_new = 0;
        }
    } else {
        *packet_new = 0;
    }

    if (ret < 0) {
        return ret;
    }
    
    return got_frame || *packet_new;
}

int main() {
    std::string mp4="/Users/wangwei/Downloads/t265.mp4";
    
    // 1. register all codecs, demux and protocols
    avdevice_register_all();
    
    // 2. 得到一个ffmpeg的上下文（上下文里面封装了视频的比特率，分辨率等等信息...非常重要）
    AVFormatContext *pFmtContext = avformat_alloc_context();
    if (!pFmtContext) {
        std::cout << "could not allocate avformat context." << std::endl;
        free(pFmtContext);
        return -2;
    }

    // 3. 打开视频
    if (avformat_open_input(&pFmtContext, mp4.c_str(), NULL, NULL) < 0) {
        std::cout << "open video file failed." << std::endl;
        free(pFmtContext);
        return -3;
    }

    // 4. 获取视频信息，视频信息封装在上下文中
    if (avformat_find_stream_info(pFmtContext, NULL) < 0) {
        std::cout << "get the information failed." << std::endl;
        free(pFmtContext);
        return -4;
    }

    // 5. 用来记住视频流的索引
    int video_stream_idx = -1;
    video_stream_idx = av_find_best_stream(pFmtContext, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
    if (video_stream_idx < 0) {
        std::cout << "can not find video stream." << std::endl;
        free(pFmtContext);
        return -5;
    }

    // 6. 获取编码器上下文和编码器
    AVCodecContext *pCodecCtx = avcodec_alloc_context3(NULL);
    if (!pCodecCtx) {
        std::cout << "get codec context failed." << std::endl;
        free(pFmtContext);
        free(pCodecCtx);
        return -6;
    }

    if (avcodec_parameters_to_context(pCodecCtx, 
                                      pFmtContext->streams[video_stream_idx]->codecpar
                                     ) < 0) {
        std::cout << "get codec parameters failed." << std::endl;
        free(pFmtContext);
        free(pCodecCtx);
        return -61;
    }

    // 7. 打开解码器
    AVCodec *pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
    if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
        std::cout << "decode the video stream failed." << std::endl;
        free(pFmtContext);
        free(pCodecCtx);
        return -7;
    }

    // 8. 解码
    AVPacket *pkt = (AVPacket *)av_malloc(sizeof(AVPacket));
    av_init_packet(pkt);
    AVFrame *pFrame = av_frame_alloc();
    int i = 0;

    while (!av_read_frame(pFmtContext, pkt)) {
        if (pkt->stream_index != video_stream_idx) {
            continue;
        }

        int packet_new = 1;
        while (process_frame(pFmtContext, pCodecCtx, pFmtContext->streams[video_stream_idx]->codecpar, 
                             pFrame, pkt, &packet_new) > 0) {
            i++;
        };
       av_packet_unref(pkt);
    }
    
    std::cout << "frame count: " << i << std::endl;
    av_frame_free(&pFrame);
    avcodec_close(pCodecCtx);
    avformat_free_context(pFmtContext);

    return 0;
}
