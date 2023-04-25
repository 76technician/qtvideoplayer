#ifndef VIDEOITEM_H
#define VIDEOITEM_H
#include <QObject>
#include <QPointer>
#include <QAudioDevice>
#include <QVideoSink>
#include <QQmlEngine>
#include <QTimer>
#include <QImage>
#include <QQueue>
#include <QThread>
#include <mutex>
#include <algorithm>
extern "C" {
    #include "libavformat/avformat.h"
    #include "libavcodec/avcodec.h"
    #include "libswscale/swscale.h"
    #include "libavutil/imgutils.h"
}

class Producer : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QVideoSink* videoSink READ videoSink WRITE setVideoSink NOTIFY videoSinkChanged)
public:
    Producer(QObject *parent=nullptr);
    QVideoSink *videoSink() const;
    void setVideoSink(QVideoSink *newVideoSink);
    Q_INVOKABLE void handleTimeout();
    Q_INVOKABLE void paint_frame(QImage frame);
signals:
    void videoSinkChanged();
private:
    QPointer<QVideoSink> m_videoSink;
};


#endif // VIDEOITEM_H
