#ifndef CAPTUREVIDEO_H
#define CAPTUREVIDEO_H
#include <QObject>
#include <QQmlEngine>
#include <QTimer>
#include <QImage>
#include <QQueue>
#include <QThread>
#include <QDebug>
extern "C" {
    #include "libavformat/avformat.h"
    #include "libavcodec/avcodec.h"
    #include "libswscale/swscale.h"
    #include "libavutil/imgutils.h"
    #include "libavutil/opt.h"
    #include "libavdevice/avdevice.h"
}
class CaptureVideo:public QThread
{
    Q_OBJECT
    QML_ELEMENT
  public:
    Q_INVOKABLE void record_screen_init();
    Q_INVOKABLE void find_video_device();
    Q_INVOKABLE void encode_init_context();
    Q_INVOKABLE void stop();
    Q_INVOKABLE void open();
    Q_INVOKABLE void video_codec(QString s);
    void video_codec();
    void encode_video_frame(AVFrame* frame);
    void run();
  signals:
    void video_play_init(AVCodecParameters *pCodeP);
    void send_pkt(AVPacket* pkt,int flag);
    void add_device(QString s);
    void init_h264_stream(const AVCodec* ecodec,AVCodecContext* ecodectx,int flag);
  private:
    bool Stop_Quit=true;
    AVFormatContext	*pFormatCtx;
    int				videoindex,ret=0;
    AVCodecContext	*pCodecCtx;
    AVCodecContext  *eCodecCtx;
    const AVInputFormat *ifmt;
    const AVCodec			*pCodec;
    const AVCodec           *eCodec;
    AVCodecParameters *pCodeP;
    std::FILE* out;
    int video=1,audio=2;//枚举类型
    int64_t cur_pts;
};
#endif // CAPTUREVIDEO_H
