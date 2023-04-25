#ifndef AUDIOTHREAD_H
#define AUDIOTHREAD_H
#include <QPointer>
#include <QVideoSink>
#include <QQmlEngine>
#include <QTimer>
#include <QImage>
#include <QQueue>
#include <QThread>
#include <QMutex>
extern "C" {
    #include "libavformat/avformat.h"
    #include "libavcodec/avcodec.h"
    #include "libswresample/swresample.h"
    #include "libavutil/imgutils.h"
}
class AudioThread:public QThread
{
    Q_OBJECT
    QML_ELEMENT
public:
    Q_INVOKABLE void stop();
    Q_INVOKABLE void open();
    Q_INVOKABLE void receive_pkt(AVPacket* pkt);
    Q_INVOKABLE void audio_decode(AVCodecParameters *para);
    double r2d(AVRational r);
    bool OpenReSample(AVCodecParameters *para);
    int Resample(AVFrame *indata, uchar  *d);
    void run();

signals:
    void format_solt(int sampleRate,int sampleSize,int channel);
    void send_pcm(uchar* data,int len,long long cur_len);
    void rtsp_sync(long long mesc);
    void set_nb_samples(int a);

private:
    AVCodecContext* aCodec;
    const AVCodec *aCode;
    SwrContext* actx;
    QQueue<uchar*> audioque;
    QQueue<double> pts_list;
    QQueue<int> audiolen;
    QMutex audiolock;
    bool Stop_Quit=true;
    int ret,sampleRate;
    double audio_pts=-1,pts=0,wait_time=0,timebase;
};

#endif // AUDIOTHREAD_H
