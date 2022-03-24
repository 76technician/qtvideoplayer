#ifndef AUDIOITEM_H
#define AUDIOITEM_H
#include <QObject>
#include <QPointer>
#include <QAudioDevice>
#include <QQmlEngine>
#include <QTimer>
#include <QAudioFormat>
#include <QAudioFrame>
#include <QAudioSink>
#include <QAudioDevice>
#include <QIODevice>
#include <QThread>
#include <QMediaDevices>
//从qml获取音频设备 槽函数生成format 进而 获取到sink sink.start.write 写入音频数据
class AudioPlay : public QObject
{
    Q_OBJECT
    QML_ELEMENT
public:
    AudioPlay(QObject *parent=nullptr);
    QAudioDevice audioDevice() const;
    void setAudioDevice(QAudioDevice newaudioDevice);
    Q_INVOKABLE void setFormat(int sampleRate, int sampleSize, int Channels);
    Q_INVOKABLE void play_sound(uchar* data,int len);
    void setAudioSink();
signals:
    void audioDeviceChanged();
private:
    QAudioDevice m_audioDevice;
    QTimer m_timer;
    QAudioSink* audio;
    QAudioFormat format;
    QIODevice *sound_device;
};
#endif // AUDIOITEM_H
