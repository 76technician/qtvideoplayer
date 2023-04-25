#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <VideoItem.h>
#include <Demuxed.h>
#include <AudioItem.h>
#include <VideoThread.h>
#include <CaptureVideo.h>
#include <CaptureAudio.h>
#include <RtspPush.h>
#include <RtspPull.h>
#include <AudioThread.h>

void Register_qml_type(){
    //此处注册qml类型
    qmlRegisterType<Producer>("Producer", 1, 0, "Producer");
    qmlRegisterType<Demuxed>("Demuxed", 1, 0, "Demuxed");
    qmlRegisterType<AudioPlay>("AudioPlay", 1, 0, "AudioPlay");
    qmlRegisterType<VideoThread>("VideoThread", 1, 0, "VideoThread");
    qmlRegisterType<AudioThread>("AudioThread", 1, 0, "AudioThread");
    qmlRegisterType<CaptureVideo>("CaptureVideo", 1, 0, "CaptureVideo");
    qmlRegisterType<CaptureAudio>("CaptureAudio", 1, 0, "CaptureAudio");
    qmlRegisterType<RtspPush>("RtspPush", 1, 0, "RtspPush");
    qmlRegisterType<RtspPull>("RtspPull", 1, 0, "RtspPull");
}
int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;
    Register_qml_type();

    const QUrl url(u"qrc:/HeloFish/main.qml"_qs);
    engine.load(url);

    return app.exec();
}
