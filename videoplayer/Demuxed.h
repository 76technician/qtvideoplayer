#ifndef DEMUXED_H
#define DEMUXED_H
extern "C" {
    #include "libavformat/avformat.h"
    #include "libavcodec/avcodec.h"
    #include "libavutil/imgutils.h"
    #include "libswscale/swscale.h"
    #include "libswresample/swresample.h"
}
#include <string>
#include <QString>
#include <QDebug>
#include <QTime>
#include <QAudioFormat>
#include <QAudioOutput>
#include <QAudioDevice>
#include <QMediaDevices>
#include <QAudioSink>
#include <QAudioFormat>
#include <QCoreApplication>
#include <QBuffer>
#include <QAudioBuffer>
#include <QThread>
#include <QQmlEngine>
#include <QUrl>
#include <cstdio>
#include <QDebug>
#include <QImage>
#include  <QByteArray>
#include <mutex>
class Demuxed: public QThread
{
    //此处定义一个信号 将内容qvideoframe发给videoitem
    Q_OBJECT
    QML_ELEMENT
public:
    Q_INVOKABLE void format_init(QString file);
    Q_INVOKABLE void format_init(QUrl file);
    Q_INVOKABLE void demuxed();
    Q_INVOKABLE void video_decode();
    Q_INVOKABLE void change_process(long long recv_pts);
    double r2d(AVRational r);
    void avframe_to_qvideofrmae(AVFrame* frame);
    Q_INVOKABLE void audio_decode();
    Q_INVOKABLE void run();
    Q_INVOKABLE void stop();
    Q_INVOKABLE void open();
    bool OpenReSample(AVCodecParameters *para);
    int Resample(AVFrame *indata, uchar  *d);
signals:
    void send_pkt(AVPacket* pkt);
    void format_solt(int sampleRate,int sampleSize,int channel);
    void send_pcm(uchar* data,int len,long long cur_len);
    void sync(long long mesc);
    void video_init(AVCodecParameters *para,long long time_base,long long sum);
private:
    bool isSet=false,Stop_Quit=false;
    bool shouldSeek=true;
    bool seekisok=true;
    std::string filePath;
    int videoindex = -1;        // 视频流索引
    int audioindex = -1;
    int videoWidth=640;
    int videoHeight=480;
    int width=640;
    int height=480;
    int ret,show_slice;
    int v_framerate=0;
    int sampleRate = 0;//样本率
    int sampleSize = 16;//样本大小
    int channel = 0;//通道数
    long long sum=0;
    long long seek_pts=0;
    double  audio_pts=-1,pts=0,wait_time=0;
//    double base_time=0;
     bool flag=true;
    SwsContext* vctx;
    SwrContext* actx;
    AVFormatContext *qAfc; //媒体信息描述
    AVCodecContext* vCodec; // 编解码信息
    AVCodecContext* aCodec; // 编解码信息
    const AVCodec *aCode;
    const AVCodec *vCode;
    AVFrame *pAVFrameYUV=av_frame_alloc();
    AVFrame *pAVFrame = av_frame_alloc();
    AVFrame *audioFrame = av_frame_alloc();
    std::mutex demued;
};
#endif // DEMUXED_H
