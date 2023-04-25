#ifndef CAPTUREAUDIO_H
#define CAPTUREAUDIO_H
#include <QObject>
#include <QQmlEngine>
#include <QTimer>
#include <QThread>
#include <QDebug>
extern "C" {
    #include "libavformat/avformat.h"
    #include "libavcodec/avcodec.h"
    #include "libswresample/swresample.h"
    #include "libavutil/imgutils.h"
    #include "libavutil/time.h"
    #include "libavutil/audio_fifo.h"
    #include "libavdevice/avdevice.h"
}
#define AAC_FRMATE_SIZE 512
class CaptureAudio:public QThread
{
    Q_OBJECT
    QML_ELEMENT
  public:
    Q_INVOKABLE void find_audio_device();
    Q_INVOKABLE void stop();
    Q_INVOKABLE void open();
    Q_INVOKABLE void audio_codec(QString s);
    Q_INVOKABLE void encode_init_context();
    void encode_audio_frame(AVFrame* frame);
    double r2d(AVRational r);
    void audio_codec();
    void run();
    void OpenResample();
  signals:
    void send_pcm(uchar* data,int len);
    void format_solt(int sampleRate,int sampleSize,int channel);
    void add_device(QString s);
    void send_pkt(AVPacket* pkt,int flag);
    void init_acc_stream(const AVCodec* ecodec,AVCodecContext* ecodectx,int flag);
  private:
    bool Stop_Quit=true;
    AVFormatContext	*pFormatCtx;
    int				audioindex,ret;
    AVCodecContext	*pCodecCtx;
    AVCodecContext	*eCodecCtx;
    SwrContext* actx;
    const AVInputFormat *ifmt;
    const AVCodec			*pCodec;
    const AVCodec			*eCodec;
    AVCodecParameters *pCodeP;
    std::FILE* outfilename;
    long long pts=1;
    uint8_t *outs[2];
    AVAudioFifo *fifo = NULL;
    AVFrame* eframe=NULL;
};



#endif // CAPTUREAUDIO_H
