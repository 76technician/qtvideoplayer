#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <VideoItem.h>
#include <Demuxed.h>
#include <AudioItem.h>
#include <VideoThread.h>
#include <CaptureVideo.h>
//存在内存泄漏
void Register_qml_type(){
    //此处注册qml类型
    qmlRegisterType<Producer>("Producer", 1, 0, "Producer");
    qmlRegisterType<Demuxed>("Demuxed", 1, 0, "Demuxed");
    qmlRegisterType<AudioPlay>("AudioPlay", 1, 0, "AudioPlay");
    qmlRegisterType<VideoThread>("VideoThread", 1, 0, "VideoThread");
    qmlRegisterType<CaptureVideo>("CaptureVideo", 1, 0, "CaptureVideo");
}
int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;
    Register_qml_type();

    const QUrl url(u"qrc:/videoplayer/main.qml"_qs);
    engine.load(url);

    return app.exec();
}
