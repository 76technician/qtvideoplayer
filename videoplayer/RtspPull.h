#ifndef RTSPPULL_H
#define RTSPPULL_H
#include <QObject>
#include <QQmlEngine>
#include <QTimer>
#include <QThread>
#include <QDebug>
#include <QQueue>
#include <QMutex>

extern "C"
{
#include "libavformat/avformat.h"
#include "libavutil/mathematics.h"
#include "libavcodec/avcodec.h"
#include "libavcodec/codec.h"
#include "libavcodec/bsf.h"
#include "libavutil/imgutils.h"
#include "libswresample/swresample.h"
#include "libavutil/time.h"
}
/*
 * 2023年2月2日12:39:30
 * 完成音视频 播放 有点卡 可能需要把 网络 音频播放模块 分离为一个线程
 * 2023年2月3日14:13:38
 * 音频分离完成 能基本顺畅播放网络rtsp测试视频rtsp://wowzaec2demo.streamlock.net/vod/mp4:BigBuckBunny_115k.mp4
 * 播放推流上去的视频 会断开 无法播放
 * 2023年2月4日11:15:19
 * 不能对 同一个地址即推流 又拉流
 * 播放音画需要同步
 * 使用 cmd推流
 * 2023年2月5日13:35:00
 * 完成拉流 播放 音画已经同步
 * */

class RtspPull: public QThread
{

    Q_OBJECT
    QML_ELEMENT

    public:
        Q_INVOKABLE void rtsp_init();
        Q_INVOKABLE void stop();
        Q_INVOKABLE void open();
        Q_INVOKABLE void set_nb_samples(int a);
        void run();
        double r2d(AVRational r);
    signals:
        void rtsp_send_pkt(AVPacket* pkt,int flag);
        void video_rtsp_init(AVCodecParameters *para,long long time_base,long long sum);
        void audio_rtsp_init(AVCodecParameters *para);

    private:
        AVFormatContext* ifmt_ctx=NULL;
        const AVBitStreamFilter *bsf;
        AVBSFContext *bsf_ctx;
        int ret=0;
        double pts=0;
        long long num=0;
        bool Stop_Quit=true;
        int sampleRate,sampleSize;
        //char* rtsp_address="rtsp://wowzaec2demo.streamlock.net/vod/mp4:BigBuckBunny_115k.mp4";
        char* rtsp_address="rtsp://xxx.xxx.xxx.xxx/helofish/video";
        int videoindex=-1,audioindex=-1;
        AVCodecContext* aCodec;
        const AVCodec *aCode;
        SwrContext* actx;
        long long wait_time=0;
        int nb_samples=1024;

};

#endif // RTSPPULL_H
