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
    #include "libavdevice/avdevice.h"
}
class CaptureVideo:public QThread
{
    Q_OBJECT
    QML_ELEMENT
  public:
    Q_INVOKABLE void record_screen_init();
    Q_INVOKABLE void find_video_device();
    Q_INVOKABLE void stop();
    Q_INVOKABLE void open();
    void video_codec();
    void run();
  signals:
    void video_play_init(AVCodecParameters *pCodeP);
    void send_pkt(AVPacket* pkt);
  private:
    bool Stop_Quit=false;
    AVFormatContext	*pFormatCtx;
    int				videoindex,ret;
    AVCodecContext	*pCodecCtx;
    AVInputFormat *ifmt;
    AVCodec			*pCodec;
    AVCodecParameters *pCodeP;
    AVFrame	*pFrame=av_frame_alloc();
    AVFrame	*pFrameYUV=av_frame_alloc();
};
#endif // CAPTUREVIDEO_H
