#include <iostream>
#include "opencv2/core/types.hpp"
#include "opencv2/opencv.hpp"
#include <opencv2/highgui.hpp>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

cv::Mat process_frame(AVFrame *frame, AVCodecContext *codec_ctx) {
    SwsContext *sws_ctx =
            sws_getContext(codec_ctx->width, codec_ctx->height, codec_ctx->pix_fmt,
                           codec_ctx->width, codec_ctx->height, AV_PIX_FMT_BGR24,
                           SWS_BILINEAR, nullptr, nullptr, nullptr);

    cv::Mat img(codec_ctx->height, codec_ctx->width, CV_8UC3);
    uint8_t *data[1] = {img.data};
    int linesize[1] = {(int)(img.step[0])};

    sws_scale(sws_ctx, frame->data, frame->linesize, 0, codec_ctx->height, data,
              linesize);

    sws_freeContext(sws_ctx);


     cv::imshow("Keyframe", img);

    return img;
}

int main() {
  std::cout << "hello" << std::endl;

    const char *filename = "1.mp4";
    AVFormatContext *format_ctx = avformat_alloc_context();

    if (avformat_open_input(&format_ctx, filename, nullptr, nullptr) != 0) {
        std::cerr << "Could not open video file" << std::endl;
        return -1;
    }

    if (avformat_find_stream_info(format_ctx, nullptr) < 0) {
        std::cerr << "Could not find stream information" << std::endl;
        return -1;
    }

    int video_stream_index = -1;
    AVCodecContext *codec_ctx = nullptr;
    const AVCodec *codec = nullptr;

    for (int i = 0; i < format_ctx->nb_streams; i++) {
        if (format_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_stream_index = i;
            codec = avcodec_find_decoder(format_ctx->streams[i]->codecpar->codec_id);
            codec_ctx = avcodec_alloc_context3(codec);
            avcodec_parameters_to_context(codec_ctx,
                                          format_ctx->streams[i]->codecpar);
            avcodec_open2(codec_ctx, codec, nullptr);
            break;
        }
    }

    if (video_stream_index == -1) {
        std::cerr << "Could not find a video stream" << std::endl;
        return -1;
    }

    AVPacket packet;
    AVFrame *frame = av_frame_alloc();
    int frameCount = 0;
    while (av_read_frame(format_ctx, &packet) >= 0) {
        if (packet.stream_index == video_stream_index) {
            avcodec_send_packet(codec_ctx, &packet);
            while (avcodec_receive_frame(codec_ctx, frame) >= 0) {
                if (frame->pict_type == AV_PICTURE_TYPE_I ||
                    frame->pict_type == AV_PICTURE_TYPE_P) {
                    std::cout << "Key Frame #" << frameCount << std::endl;
                    cv::Mat key_frame = process_frame(frame, codec_ctx);
                    av_frame_unref(frame);
                }
                frameCount++;
            }
        }
        av_packet_unref(&packet);
    }

    av_frame_free(&frame);
    avcodec_free_context(&codec_ctx);
    avformat_close_input(&format_ctx);

  return 0;
}