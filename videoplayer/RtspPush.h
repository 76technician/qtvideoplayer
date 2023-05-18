#ifndef RTSPPUSH_H
#define RTSPPUSH_H
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
#include "libavutil/time.h"
#include "libavdevice/avdevice.h"
#include "libavutil/imgutils.h"
};

/*
 *  2023年1月17日17:36:35
 *  需要配置AVstream 写入文件头进行推流 延迟近1s (已实现h264推流）
 *  2023年1月19日19:20:36
 *  需要使用ffmpeg自带编码器 否则 acc with no global head s16 to fltp
 *  2023年1月20日11:30:14
 *  音视频发出成功 dts pts设置有误
 *  2023年1月21日15:17:54
 *  视频延迟1s 音频延迟多 且vlc 音话不同步 应该是vlc播放不同
 *  2023年2月3日14:13:14
 *  pts设置有问题 全部换为标准pts 两个音频 一个视频
 *  2023年2月7日15:35:31
 *  音频延迟缩短成功
 *  2023年2月8日11:16:11
 *  推流延迟降低到1-2s vlc播放通过 推拉流基本万郴
 * */
class RtspPush:public QThread
{
    Q_OBJECT
    QML_ELEMENT
public:
    Q_INVOKABLE void rtsp_init();
    Q_INVOKABLE void receive_pkt(AVPacket* pkt,int flag);
    Q_INVOKABLE void stop();
    Q_INVOKABLE void open();
    Q_INVOKABLE void init_out_stream(const AVCodec* codec,AVCodecContext* codectx,int flag);
    Q_INVOKABLE void set_rtsp_address(QString s);
    void run();
private:
    AVFormatContext* ofmt_ctx=NULL;
    int ret=0;
    int64_t cur_pts=0;
    int64_t cur_a__pts=0;
    long long num=0;
    bool Stop_Quit=true,video_sync=false;
    QQueue<AVPacket*> pkts;
    QQueue<AVPacket*> pkt_gc;
    int videoindex=-1,audioindex=-1;
    QMutex que;
    char *rtsp_address= "rtsp://101.43.247.56/helofish/video";
};

#endif // RTSPPUSH_H
