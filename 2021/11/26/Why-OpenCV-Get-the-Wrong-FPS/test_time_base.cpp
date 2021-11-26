#include <string>
#include <iostream>

extern "C" {
#include "libavdevice/avdevice.h"
#include "libavformat/avformat.h"
}

int main() {
    std::string f="test.ts";
    
    avdevice_register_all();
    AVFormatContext *pFmtContext = avformat_alloc_context();
    if (!pFmtContext) {
        std::cerr << "could not allocate avformat context." << std::endl;
        free(pFmtContext);
        return -1;
    }

    if (avformat_open_input(&pFmtContext, f.c_str(), NULL, NULL) < 0) {
        std::cerr << "open video file failed." << std::endl;
        free(pFmtContext);
        return -2;
    }

    if (avformat_find_stream_info(pFmtContext, NULL) < 0) {
        std::cerr << "get the information failed." << std::endl;
        free(pFmtContext);
        return -3;
    }

    int video_stream_idx = -1;
    video_stream_idx = av_find_best_stream(pFmtContext, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
    if (video_stream_idx < 0) {
        std::cerr << "can not find video stream." << std::endl;
        free(pFmtContext);
        return -4;
    }

    AVRational time_base = pFmtContext->streams[video_stream_idx]->codec->time_base;
    AVRational framerate = pFmtContext->streams[video_stream_idx]->codec->framerate;
    AVRational avg_framerate = pFmtContext->streams[video_stream_idx]->avg_frame_rate;
    
    std::cout << "time_base: " << time_base.num << "/" << time_base.den << std::endl;
    std::cout << "framerate: " << framerate.num << "/" << framerate.den << std::endl;
    std::cout << "avg_framerate: " << avg_framerate.num << "/" << avg_framerate.den << std::endl;
    
    avformat_free_context(pFmtContext);

    return 0;
}