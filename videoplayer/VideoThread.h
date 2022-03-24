#ifndef VIDEOTHREAD_H
#define VIDEOTHREAD_H
#include <QPointer>
#include <QVideoSink>
#include <QQmlEngine>
#include <QTimer>
#include <QImage>
#include <QQueue>
#include <QThread>
#include <mutex>
extern "C" {
    #include "libavformat/avformat.h"
    #include "libavcodec/avcodec.h"
    #include "libswscale/swscale.h"
    #include "libavutil/imgutils.h"
}
class VideoThread : public QThread
{
    Q_OBJECT
    QML_ELEMENT
public:
    void run();
    Q_INVOKABLE void video_encode_init(AVCodecParameters* para,long long time_base);
    Q_INVOKABLE void setSync(long long mesc);
    Q_INVOKABLE void receive_pkt(AVPacket* pkt);
    Q_INVOKABLE void set_mod(long long cur);
    Q_INVOKABLE void seek(long long seek_pts);
    Q_INVOKABLE void stop();
    Q_INVOKABLE void open();
    double r2d(AVRational r);
    QImage Repaint(AVFrame *frame);
signals:
    void send_image(QImage frame);
private:
    bool isSeek=false,Stop_Quit=false;
    bool shouldSeek=true;
    long long seek_pts=0;
    QQueue<AVFrame*> cache;
    SwsContext* vctx;
    AVFormatContext *qAfc; //媒体信息描述
    AVCodecContext* vCodec; // 编解码信息
    AVCodec *vCode;
    QQueue<QImage> images;
    long long time_base=1;
    long long sync=0;
    bool first_repaint=true;
    int ret=0;
    long long  flag=1;
    std::mutex video_mutex;
};
#endif // VIDEOTHREAD_H
